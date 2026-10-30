from collections.abc import Mapping
from typing import Any

from BaseClasses import CollectionState, Item, Location
from worlds.AutoWorld import WebWorld, World

from . import items
from . import options as hp3_options
from . import regions as hp3_regions
from . import rules
from .location_table import LOCATIONS, LocationData

BASE_LOCATION_ID = 33_000_000
GOAL_LOCATION = "Final Day Door"
VANILLA_CANDY_SUFFIX = " (vanilla)"   # candysanity off: the pickup's event location


class HP3PCLocation(Location):
    game = items.GAME_NAME


class HP3PCWeb(WebWorld):
    theme = "stone"
    tutorials = []


class HP3PCWorld(World):
    """
    Harry Potter and the Prisoner of Azkaban (PC, 2004) in Open Castle form: Hogwarts in its late-year state with
    every story level behind a door, spells and level unlocks as items, and wizard cards opening the Final Day.
    """

    game = items.GAME_NAME
    web = HP3PCWeb()
    options_dataclass = hp3_options.HP3PCOptions
    options: hp3_options.HP3PCOptions
    topology_present = True

    item_name_to_id = items.ITEM_NAME_TO_ID
    item_name_groups = items.ITEM_NAME_GROUPS
    location_name_to_id = {loc.name: BASE_LOCATION_ID + loc.offset for loc in LOCATIONS}
    location_name_groups: dict[str, set[str]] = {}
    for _loc in LOCATIONS:
        location_name_groups.setdefault(_loc.region, set()).add(_loc.name)
    del _loc

    origin_region_name = hp3_regions.ORIGIN

    # ---- generation ------------------------------------------------------------------------------------------------
    def generate_early(self) -> None:
        self.starting_spells: list[str] = []   # hard mode: spells given to open the start (hard_mode_opening)
        # the locations in play: secretsanity / candysanity off leave out the secret areas / the placed pasties and
        # cakes (those stay vanilla in the game, and with candysanity off logic counts them as events)
        self.vanilla_candy: list[LocationData] = []
        self.active_locations: list[LocationData] = []
        for loc in LOCATIONS:
            if loc.kind == "secret" and not self.options.secretsanity:
                continue
            if loc.kind == "placed" and not self.options.candysanity:
                self.vanilla_candy.append(loc)
                continue
            self.active_locations.append(loc)
        for name in items.starting_items(self):
            self.multiworld.push_precollected(self.create_item(name))

    def create_regions(self) -> None:
        regs = hp3_regions.create_regions(self)
        for loc in self.active_locations:
            region = regs[loc.region]
            region.locations.append(HP3PCLocation(self.player, loc.name, self.location_name_to_id[loc.name], region))
        for loc in self.vanilla_candy:   # the vanilla pasty / cake, as an event logic can count
            region = regs[loc.region]
            event = HP3PCLocation(self.player, loc.name + VANILLA_CANDY_SUFFIX, None, region)
            event.place_locked_item(items.HP3PCItem(items.bundle_name(loc.pasties, loc.cakes),
                                                    items.IC.progression_skip_balancing, None, self.player))
            region.locations.append(event)
        goal = HP3PCLocation(self.player, GOAL_LOCATION, None, regs["Grounds"])
        goal.place_locked_item(self.create_item(items.VICTORY_EVENT))
        regs["Grounds"].locations.append(goal)

    def vanilla_shop_items(self) -> dict[str, str]:
        """shopsanity off: location name -> the vanilla item locked there (its card, or Portrait Password n)."""
        if self.options.shopsanity:
            return {}
        return {loc.name: loc.card or f"Portrait Password {int(loc.actor.split('_')[1]) + 1}"
                for loc in self.active_locations if loc.kind in ("shop", "vendor")}

    def create_items(self) -> None:
        pool = items.core_pool(self)
        locked = self.vanilla_shop_items()
        for loc_name, item_name in locked.items():
            self.get_location(loc_name).place_locked_item(self.create_item(item_name))
            pool.remove(item_name)
        # every location's vanilla currency (cards are in the core pool already); frogs for the rest
        currency = [items.bundle_name(loc.pasties, loc.cakes) for loc in self.active_locations
                    if not loc.card and (loc.pasties or loc.cakes)]
        free = len(self.active_locations) - len(locked)
        if len(pool) > free:
            raise RuntimeError(f"{self.game}: {len(pool)} core items for {free} locations")
        # if the core items push some currency out, drop the smallest bundles first (the shops need the big ones)
        room = free - len(pool)
        if len(currency) > room:
            currency.sort(key=lambda n: sum(items.CURRENCY_VALUE[n]), reverse=True)
            currency = currency[:room]
        # Only as much currency as the shops can need is progression (biggest bundles first); the rest is the same
        # item as filler. Logic counts progression bundles only, and fill has far fewer keys to place (with every
        # bundle as progression, hard mode failed to fill now and then).
        need = rules.currency_thresholds(self, self.active_locations)
        top = [max(v[k] for v in need.values()) for k in (0, 1)]
        # the vanilla pasties and cakes (candysanity off) count first
        have = [sum(loc.pasties for loc in self.vanilla_candy), sum(loc.cakes for loc in self.vanilla_candy)]
        progression, filler = [], []
        for name in sorted(currency, key=lambda n: (-sum(items.CURRENCY_VALUE[n]), n)):
            value = items.CURRENCY_VALUE[name]
            if any(value[k] and have[k] < top[k] for k in (0, 1)):
                progression.append(name)
                have = [have[k] + value[k] for k in (0, 1)]
            else:
                filler.append(name)
        for k, what in enumerate(("pumpkin pasties", "cauldron cakes")):
            if have[k] < top[k]:
                raise RuntimeError(f"{self.game}: the pool holds {have[k]} {what}, the shops need {top[k]}")
        frogs = free - len(pool) - len(currency)
        self.multiworld.itempool += [self.create_item(n) for n in pool + progression]
        self.multiworld.itempool += [self.create_item(n, items.IC.filler) for n in filler]
        self.multiworld.itempool += [self.create_item(items.FILLER_ITEM_NAME) for _ in range(frogs)]

    def set_rules(self) -> None:
        need = rules.currency_thresholds(self, self.active_locations)
        for loc in self.active_locations:
            self.get_location(loc.name).access_rule = rules.location_rule(self, loc, need.get(loc.name, (0, 0)))
        for loc in self.vanilla_candy:
            self.get_location(loc.name + VANILLA_CANDY_SUFFIX).access_rule = rules.location_rule(self, loc)
        self.get_location(GOAL_LOCATION).access_rule = rules.goal_rule(self)
        self.multiworld.completion_condition[self.player] = lambda state: state.has(items.VICTORY_EVENT, self.player)
        if self.options.spell_mode == self.options.spell_mode.option_hard:
            self.hard_mode_opening()

    def hard_mode_opening(self) -> None:
        """Hard mode: the vanilla starting spells go to the checks open at the start, in the player's own world (with
        fewer, fill failed now and then: a solo seed's few early spots ran out before the spells were placed). With
        the shops (bean-priced ones) or the secret areas off, the opening can be a single check, so the player starts
        with as few of those spells as it takes to open enough checks (the one that opens the most first)."""
        def open_spots() -> int:
            state = CollectionState(self.multiworld)
            return sum(1 for loc in self.get_locations() if loc.address is not None and not loc.item and loc.can_reach(state))

        spells = list(items.VANILLA_STARTING_SPELLS)
        free = open_spots()
        while spells and free < len(spells) + 3:
            best, best_free = None, -1
            for name in spells:
                item = self.create_item(name)
                self.multiworld.precollected_items[self.player].append(item)   # CollectionState() collects these
                n = open_spots()
                self.multiworld.precollected_items[self.player].remove(item)
                if n > best_free:
                    best, best_free = name, n
            spells.remove(best)
            self.multiworld.push_precollected(self.create_item(best))
            pooled = next(i for i in self.multiworld.itempool if i.player == self.player and i.name == best)
            self.multiworld.itempool.remove(pooled)
            self.multiworld.itempool.append(self.create_item(items.FILLER_ITEM_NAME))
            self.starting_spells.append(best)
            free = best_free
        for name in spells[:free]:
            self.multiworld.local_early_items[self.player][name] = 1

    # ---- items and currency on the state -----------------------------------------------------------------------------
    def create_item(self, name: str, classification: "items.IC | None" = None) -> items.HP3PCItem:
        item = items.create_item(self, name)
        if classification is not None:
            item.classification = classification
        return item

    def get_filler_item_name(self) -> str:
        return items.FILLER_ITEM_NAME

    def collect(self, state: CollectionState, item: Item) -> bool:
        changed = super().collect(state, item)
        if changed and item.name in items.CURRENCY_VALUE:
            p, c = items.CURRENCY_VALUE[item.name]
            state.prog_items[self.player][rules.PASTY_KEY] += p
            state.prog_items[self.player][rules.CAKE_KEY] += c
        return changed

    def remove(self, state: CollectionState, item: Item) -> bool:
        changed = super().remove(state, item)
        if changed and item.name in items.CURRENCY_VALUE:
            p, c = items.CURRENCY_VALUE[item.name]
            state.prog_items[self.player][rules.PASTY_KEY] -= p
            state.prog_items[self.player][rules.CAKE_KEY] -= c
        return changed

    def fill_slot_data(self) -> Mapping[str, Any]:
        data = dict(self.options.as_dict("goal", "cards_required", "spell_mode", "split_challenge_spells",
                                         "shop_prices", "shortcut_passwords", "secretsanity", "candysanity",
                                         "shopsanity"))
        data["world_version"] = "0.3.4"
        return data
