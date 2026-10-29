# Logic to-do: item spawns without a definitive logic requirement

Generated 2026-10-28 from `data/hp3_location_catalog.csv` against the logic notes in [APWORLD.md](APWORLD.md).
Candidate locations: placed pasties / cakes / cards, event-spawned pasties / cakes, containers with a fixed pasty /
cake / card drop, and secret areas. Left out: random-amount containers (not locations), bean-only containers,
challenge stars (not randomized), bundimuns (exempt: their behaviour isn't changed), shops and vendors (logic = shop access + currency; this includes the five
`WC_BeatrixBloxam` cards, which are display props in Fred & George's shop), and spawns whose trigger never fires in
Open Castle (Hogwarts: the Peeves 2 / 3 rewards, `CauldronCake4`, `PumpkinPasty5`), and the five cards in the
Grounds' developer room (Bran the Bloodthirsty, Cyclops, Goliath, Hengist of Upper Barnton, Morholt), and
the Library's out-of-bounds copy of the grounds (a cutscene set: secrets 2, 4, 5, 6 and a Double-Ended Newt card), and the half of each Glacius map
that the player is never in (Ch3 and Ch3B share the middle of the level; see APWORLD.md), Glacius
`PumpkinPasty36` (exempt), Glacius B's and Draconifors B's bean bonus rooms (exempt; 21 and 30 spawns), Carpe event cake `HPSpawnThingy3` (exempt: only the first-visit doxy cutscene spawns it), four Glacius B slide pasties (`PumpkinPasty22`, `23`, `24`, `6`: exempt for now, believed reachable but
unconfirmed; see APWORLD.md), and the dropped maps
(Quidditch, WhompingWillow, BuckyExecuted, DementorBattle, Infirmary, DarkTower, GroundsDada, Final).

- **Partly known:** 5 (a region rule or an event chain covers them, but they weren't reported spot by spot).
- **Nothing recorded:** 0.

## Partly known

| Area | Actor | What | Where | Status |
| --- | --- | --- | --- | --- |
| Hermione FE | `CauldronCake20` | cake (placed) | (6186, 1091, 1642) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `CauldronCake21` | cake (placed) | (7070, 1091, 3348) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `CauldronCake22` | cake (placed) | (7075, -21, 3631) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `CauldronCake23` | cake (placed) | (5704, -771, 1389) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty21` | pasty (placed) | (6103, -17, 3215) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |

## Nothing recorded

| Area | Count |
| --- | ---: |
