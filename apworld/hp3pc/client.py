"""Harry Potter 3 PC client: bridges the Archipelago server and the Open Castle mod.

Started from the Archipelago launcher ("Harry Potter 3 PC Client"), or from an Archipelago source checkout:
    python -c "from worlds.hp3pc.client import launch; launch()" --connect localhost:38281 --name Harry

The mod (mod/OpenCastle, HP3OpenCastle.asi) connects to this client on 127.0.0.1:42780, the HP2PC_AP way. One line per
message, ASCII; IDs on the wire are item / location ids minus the world's base id (33000000).

  mod -> client   HELLO <mod version>
                  BIND <seed>|<slot>      the loaded save's room, or "BIND -" for a save not bound yet
                  CHECK <loc>             a location was just checked
                  CHECKS <loc>,<loc>...   every location checked in that save (after each BIND)
                  MISMATCH                the save belongs to another room
                  DOOR                    the Final Day door has opened in game
                  GOAL                    the player walked through the Final Day door (resent on every connection
                                          while the save has reached it)
  client -> mod   ROOM <seed>|<slot>|<split 0/1>|<cheap 0/1>|<cards required>|<secretsanity 0/1>|<candysanity 0/1>
                  ITEMS <first index> <item>,<item>...   the full received list, in chunks
                  SYNCED <count>          end of the full list
                  ITEM <index> <item>     one more received item
                  CHECKED <loc>,<loc>...  locations the server already has
                  MSG <text>              shown in the mod's log
                  LOOKS <loc>:<code>,...  what each location holds, so the pickup can show it (LOOK_CODES below;
                                          the mod's table is mod/OpenCastle/aplook.h)

The save keeps how far it has applied the received list (docs/CLIENT_SYNC.md), so the client always sends the whole
list and the mod picks up from where the save is.

Goal: once `cards_required` wizard cards have been received the client says "The Final Day Door is open" and the mod
opens the door on the grounds; walking in through it (the mod sends GOAL) completes the goal. The gap gives the
player time to tell the others they're about to finish.
"""
from __future__ import annotations

import asyncio
import logging
from typing import Optional

from CommonClient import ClientCommandProcessor, CommonContext, get_base_parser, gui_enabled, logger, server_loop
from NetUtils import ClientStatus

from . import items

GAME_PORT = 42780

# Look codes, shared with mod/OpenCastle/aplook.h (LOOKDEFS). Spells 1-10 in the mod's spell order.
SPELL_LOOK = {"Alohomora": 1, "Carpe Retractum": 2, "Depulso": 3, "Draconifors": 4, "Expecto Patronum": 5,
              "Glacius": 6, "Lapifors": 7, "Lumos": 8, "Rictusempra": 9, "Spongify": 10}
UNLOCK_LOOK = {"Hippogriff Challenge Unlock": 21, "Doxy Challenge Unlock": 22, "Monster Book Challenge Unlock": 23}
LOOK_KEY, LOOK_SCROLL, LOOK_MAP, LOOK_PASTY, LOOK_CAKE, LOOK_FROG, LOOK_CARD = 20, 30, 32, 33, 34, 35, 36
LOOK_CARD0 = 100   # + card index: that wizard card (mod/OpenCastle/wc_textures.h)
LOOK_AP_PROGRESSION, LOOK_AP_USEFUL, LOOK_AP_FILLER = 40, 41, 42
ID_TO_NAME = {code: name for name, code in items.ITEM_NAME_TO_ID.items()}


def hp3_look(item_id: int) -> int:
    """The look of one of this game's items (by id)."""
    name = ID_TO_NAME.get(item_id, "")
    if name in SPELL_LOOK:
        return SPELL_LOOK[name]
    if name in UNLOCK_LOOK:
        return UNLOCK_LOOK[name]
    if name in items.LEVEL_UNLOCKS:
        return LOOK_KEY
    if name in items.PASSWORDS:
        return LOOK_SCROLL
    if name == "Marauder's Map":
        return LOOK_MAP
    if name in items.CURRENCY_VALUE:
        pasties, cakes = items.CURRENCY_VALUE[name]
        return LOOK_PASTY if pasties >= cakes else LOOK_CAKE
    if name in items.CARDS:   # that card's own face: 100 + its index in WizardCardManager.Cards
        return LOOK_CARD0 + items.CARDS[name].offset - items.CARD_OFFSET
    return LOOK_FROG


def other_look(flags: int) -> int:
    """Another game's item: the Archipelago logo by tier (traps look like filler)."""
    if flags & 0b001:
        return LOOK_AP_PROGRESSION
    if flags & 0b010:
        return LOOK_AP_USEFUL
    return LOOK_AP_FILLER
BASE = items.BASE_ID
CARD_IDS = {BASE + data.offset for data in items.CARDS.values()}
CHUNK = 100


class HP3CommandProcessor(ClientCommandProcessor):
    def _cmd_game(self) -> bool:
        """Show whether the game (the Open Castle mod) is connected."""
        ctx: HP3Context = self.ctx   # type: ignore[assignment]
        if ctx.game_writer:
            self.output(f"Game connected (mod {ctx.mod_version or '?'}); save bound to {ctx.save_bind or 'nothing yet'}.")
        else:
            self.output(f"Game not connected: start Harry Potter 3 with the Open Castle mod (it connects to port {GAME_PORT}).")
        return True


class HP3Context(CommonContext):
    command_processor = HP3CommandProcessor
    game = items.GAME_NAME
    items_handling = 0b111   # own items and starting inventory come through the server too

    def __init__(self, server_address: Optional[str], password: Optional[str]) -> None:
        super().__init__(server_address, password)
        self.game_writer: Optional[asyncio.StreamWriter] = None
        self.mod_version = ""
        self.save_bind = ""
        self.slot_data: dict = {}
        self.goal_sent = False
        self.door_announced = False
        self.looks: dict[int, int] = {}   # location offset -> look code

    # ---- server side ---------------------------------------------------------------------------------------------
    async def server_auth(self, password_requested: bool = False) -> None:
        if password_requested and not self.password:
            await super().server_auth(password_requested)
        await self.get_username()
        await self.send_connect()

    def on_package(self, cmd: str, args: dict) -> None:
        if cmd == "Connected":
            self.slot_data = args.get("slot_data", {}) or {}
            self.goal_sent = False
            self.door_announced = False
            self.looks = {}
            asyncio.create_task(self.sync_game())
            locs = sorted(set(args.get("missing_locations", [])) | set(args.get("checked_locations", [])))
            if locs:   # what's where, for the looks (no hints are created)
                asyncio.create_task(self.send_msgs([{"cmd": "LocationScouts", "locations": locs, "create_as_hint": 0}]))
        elif cmd == "LocationInfo":
            for item in args["locations"]:
                game = self.slot_info[item.player].game if item.player in self.slot_info else ""
                self.looks[item.location - BASE] = hp3_look(item.item) if game == self.game else other_look(item.flags)
            self.send_looks()
        elif cmd == "ReceivedItems":
            start = args["index"]
            if start == 0:
                asyncio.create_task(self.sync_game())
            else:
                for i, item in enumerate(args["items"], start):
                    self.to_game(f"ITEM {i} {item.item - BASE}")
            self.check_goal()
        elif cmd == "RoomUpdate" and "checked_locations" in args:
            self.send_checked(args["checked_locations"])

    async def disconnect(self, allow_autoreconnect: bool = False) -> None:
        self.slot_data = {}
        await super().disconnect(allow_autoreconnect)

    def room_key(self) -> str:
        return f"{self.server_seed_name or self.seed_name}|{self.slot}"

    def check_goal(self) -> None:
        """Enough cards: announce the open door (the goal itself waits for the player to walk through it)."""
        if self.door_announced or not self.slot_data:
            return
        need = int(self.slot_data.get("cards_required", 0) or 0)
        have = sum(1 for item in self.items_received if item.item in CARD_IDS)
        if need and have >= need:
            self.door_announced = True
            logger.info(f"The Final Day Door is open. ({have} wizard cards received, {need} needed: walk through "
                        f"the door on the grounds to finish.)")

    # ---- game side -----------------------------------------------------------------------------------------------
    def to_game(self, line: str) -> None:
        if self.game_writer:
            self.game_writer.write((line + "\n").encode("ascii", "replace"))

    def send_looks(self) -> None:
        pairs = sorted(self.looks.items())
        for k in range(0, len(pairs), CHUNK):
            self.to_game("LOOKS " + ",".join(f"{loc}:{code}" for loc, code in pairs[k:k + CHUNK]))

    def send_checked(self, locations) -> None:
        offs = sorted(loc - BASE for loc in locations)
        for k in range(0, len(offs), CHUNK):
            self.to_game("CHECKED " + ",".join(map(str, offs[k:k + CHUNK])))

    async def sync_game(self) -> None:
        """Everything the mod needs about the room: identity and options, the whole received list, what's checked."""
        if not self.game_writer or not self.slot_data or self.slot is None:
            return
        sd = self.slot_data
        self.to_game(f"ROOM {self.room_key()}|{int(bool(sd.get('split_challenge_spells')))}|"
                     f"{int(sd.get('shop_prices', 0) == 1)}|{int(sd.get('cards_required', 0) or 0)}|"
                     f"{int(bool(sd.get('secretsanity', 1)))}|{int(bool(sd.get('candysanity', 1)))}")
        ids = [item.item - BASE for item in self.items_received]
        for k in range(0, len(ids), CHUNK):
            self.to_game(f"ITEMS {k} " + ",".join(map(str, ids[k:k + CHUNK])))
        self.to_game(f"SYNCED {len(ids)}")
        self.send_checked(self.checked_locations)
        self.send_looks()
        if self.game_writer:
            await self.game_writer.drain()

    async def game_line(self, line: str) -> None:
        word, _, rest = line.partition(" ")
        if word == "HELLO":
            self.mod_version = rest
            logger.info(f"Game connected (Open Castle {rest}).")
            await self.sync_game()
        elif word == "BIND":
            self.save_bind = "" if rest == "-" else rest
            if self.save_bind and self.slot is not None and self.save_bind != self.room_key():
                logger.warning(f"The loaded save belongs to another room ({self.save_bind}); this room is "
                               f"{self.room_key()}. Nothing is sent or received for it.")
        elif word in ("CHECK", "CHECKS"):
            locs = [BASE + int(x) for x in rest.split(",") if x.strip().isdigit()]
            new = [loc for loc in locs if loc in self.missing_locations]
            if new:
                await self.check_locations(new)
        elif word == "DOOR":
            logger.info("The Final Day Door is open in game.")
        elif word == "GOAL":
            if not self.goal_sent and self.slot is not None:
                self.goal_sent = True
                logger.info("Through the Final Day Door: goal complete!")
                await self.send_msgs([{"cmd": "StatusUpdate", "status": ClientStatus.CLIENT_GOAL}])
        elif word == "MISMATCH":
            logger.warning("The game's save is bound to a different room: load the right save, or start a New Game.")

    async def handle_game(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter) -> None:
        if self.game_writer:
            logger.warning("A second game connection; replacing the first.")
            self.game_writer.close()
        self.game_writer = writer
        try:
            while not reader.at_eof():
                raw = await reader.readline()
                if not raw:
                    break
                line = raw.decode("ascii", "replace").strip()
                if line:
                    await self.game_line(line)
        except (ConnectionError, asyncio.IncompleteReadError):
            pass
        finally:
            if self.game_writer is writer:
                self.game_writer = None
                logger.info("Game disconnected.")
            writer.close()

    def make_gui(self):
        ui = super().make_gui()
        ui.base_title = "Harry Potter 3 PC Client"
        return ui


def launch(*launch_args: str) -> None:
    import colorama
    import Utils

    async def main() -> None:
        parser = get_base_parser(description="Harry Potter 3 PC client (Open Castle mod bridge).")
        parser.add_argument("--name", default=None, help="Slot name to connect as.")
        args = parser.parse_args(launch_args)
        ctx = HP3Context(args.connect, args.password)
        if args.name:
            ctx.auth = args.name
        ctx.server_task = asyncio.create_task(server_loop(ctx), name="ServerLoop")
        game_server = await asyncio.start_server(ctx.handle_game, "127.0.0.1", GAME_PORT)
        logger.info(f"Waiting for the game on 127.0.0.1:{GAME_PORT}.")
        if gui_enabled and not getattr(args, "nogui", False):
            ctx.run_gui()
        ctx.run_cli()
        await ctx.exit_event.wait()
        game_server.close()
        await ctx.shutdown()

    Utils.init_logging("HP3PCClient", exception_logger="Client")
    colorama.just_fix_windows_console()
    asyncio.run(main())
    colorama.deinit()


if __name__ == "__main__":
    import sys
    launch(*sys.argv[1:])
