# Client sync: getting items into the game, and checks out of it

Design for how received items reach the player's game and how the game's state is reconciled with the
Archipelago server. Status: **design only** (2026-10-27). Implementation comes with the client and apworld.

Archipelago games are often played asynchronously: items can arrive while the game is closed, mid-cutscene, or on
a save the player later abandons. The design has to stay correct in all of those cases without the mod needing to
watch anything continuously.

## Principles

- **The save is the record of what the game has applied.** Everything the mod gives the player, and how far
  through the received list it has got, is kept in the game's PersistentStore (`OC_*` keys). The store travels
  between levels and is written into save games, so the two can never disagree.
- **The server is the record of what has been received and checked.** On every connection it resends the full
  received-items list, and it ignores duplicate location checks.
- **Reconcile at least once per load.** Compare save and server on every save load / settled level load and fill
  the gap. During play, only a small queue needs draining.
- **One way in for items.** Every item, including the player's own, arrives through the server's received-items
  list. Picking up a location only reports the check.

## 1. The received index lives in the save

Every received item has an index (0, 1, 2, …). The mod keeps "applied up to N" as `OC_RecvIndex` in the
PersistentStore. Because it is saved with the very unlocks and inventory it describes:

| Situation | What happens | Why it's right |
| --- | --- | --- |
| Item applied, player quits without saving | The save still has the old index; the item is applied again on load | The save never had it |
| Player loads an older save | Smaller index; everything received since is re-applied | That save lacks those items |
| Two saves of the same seed | Each has its own index | Neither is shortchanged |
| New Game on an existing seed | Index 0; everything is applied again, starting inventory included | It's a fresh game |

Keeping the index in the client instead would lose or duplicate items in each of these cases.

## 2. Two kinds of items

| Kind | Items | Rule |
| --- | --- | --- |
| **Idempotent** (state) | spells, level / challenge unlocks, portrait passwords, Shortcut Passwords, wizard cards, the Marauder's Map | Safe to give twice. On every load, rebuild the whole set from the full received list and assert it (the `OC_*` flags; cards via `WizardCardManager`, given only if not owned). Also the catch-all for any drift. |
| **Counted** (consumable) | pasty and cake bundles, chocolate frogs | Change the inventory and get spent, so each is applied exactly once: only items with index ≥ `OC_RecvIndex`, then advance the index. |

`OC_RecvIndex` advances over every item, so idempotent items past it are applied once more by the counted path.
That's harmless.

## 3. Delivery window and queue

Received items go into a queue in the mod. They are applied only when it's safe:

- from a tick event (`PlayerTick` / `Tick`), never while the frame is drawn (the poc7d rule);
- while the player is in control: not in a cutscene, minigame (COMC, doxy, Monster Book, bean bonus), menu,
  pause or level transition.

An item arriving during an unskippable cutscene waits in the queue. If the game closes first, nothing is lost: the
saved index never moved past it, so the next load picks it up again.

Feedback on delivery: the pickup sparkle and sound (poc7b), and a short HUD message. Message details TBD.

## 4. Reconciliation on every load

When a save loads or a level settles (the existing settle point, which already reads the `OC_*` flags):

1. **Read the save:** `OC_RecvIndex`, the seed / slot binding (§6), the found flags, the checked locations.
2. **Get the full received list** from the client (the server resends it on every connection).
3. **Idempotent items:** compute the set from the whole list and assert every one (set flags, give missing
   cards, grant the map / shortcut passwords).
4. **Counted items:** queue every item with index ≥ `OC_RecvIndex`.
5. **Send every checked location in the save** to the client. The server ignores ones it already has. This covers
   checks made while the client was disconnected, or a dropped connection.

If the client isn't connected at load time, the mod keeps the save as is and reconciles when the client appears.

## 5. Locations (checks out)

- Each check is recorded in the save when it happens. Many already have vanilla persistence (`<LevelID><Actor>_PU`,
  spawner `_lives`, card ownership in `WizardCardManager`); anything without it gets an `OC_Loc_<id>` key.
- The mod reports the check to the client at once, and the whole set again on every load (§4.5).
- A location holding the player's own item does nothing locally: the server sends the item back through the
  received list.

## 6. Seed binding

- On first connect, the mod writes the seed name and slot into the save (`OC_Seed`, `OC_Slot`).
- When a save is loaded, the client compares them with the connected room. On a mismatch it warns and neither
  sends checks nor delivers items, so two runs' items never mix.
- A save with no binding (a fresh New Game) is bound on first connect.

## 7. Starting inventory and options

- The server delivers starting inventory (precollected items: `spell_mode: vanilla` spells, `shortcut_passwords:
  start_unlocked`, …) as the first received items, with the client's items-handling flags set to receive both
  remote and own items plus starting inventory.
- The ini starting state (`HP3_OpenCastle.ini`, poc7c) was removed in poc8c: until the client exists, everything starts locked and the debug panel turns things on.
- Options that are settings, not items (`shop_prices`, `cards_required`, `goal`), come from slot data on connect
  and are stored in the save alongside the seed binding.

## 8. Transport between mod and client

| Option | Pros | Cons |
| --- | --- | --- |
| **Files** in `Documents\Harry Potter and the Prisoner of Azkaban\` (an inbox the client writes, an outbox the mod writes) | Same pattern the mod already uses for its ini; easy to inspect; either side can restart freely | Polling; needs append-only or atomic-replace writes |
| **Local socket** (mod connects to the client on localhost) | Immediate | The mod handles reconnects; harder to debug |

Latency barely matters given §3–§4, so **files first**. A sketch:

- `HP3AP_inbox.txt` (client → mod): the full received list, one line per item: `index item_id sender location`.
  Rewritten atomically (write a temp file, then rename) whenever it changes, plus the seed / slot and slot data.
- `HP3AP_outbox.txt` (mod → client): checked location IDs, appended; and on load, the full set from the save.
- Both carry the seed / slot, so a stale file from another run is ignored.

## Open questions

- How to tell reliably that the player is in control (cutscene state on `KWGame` / the hero controller,
  minigame actors, `Level.Pause`).
- The HUD message for received items.
- Whether bean jars (filler, as HP2PC does) join the counted kind later.
- DeathLink: not planned.
