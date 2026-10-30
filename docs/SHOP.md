# Shop prices

YAML option `shop_prices`: `normal` (default) keeps the vanilla prices, so shop checks can wait on cakes and pasties arriving; `cheap` makes every check-bearing purchase cost 50% of vanilla, rounded up (25 → 13, 12.5 rounded up).

Until the apworld and slot data exist, the mod reads the switch from `Documents\Harry Potter and the Prisoner of Azkaban\HP3_OpenCastle.ini`:

```ini
[Shop]
Cheap=1   ; 0 or absent = normal
```

It is re-read every 2 s, so it can be flipped while the game runs. Implementation: `mod/OpenCastle/shop.h` (build poc6q; poc6p had 10%).

## What is scaled

- **Wizard cards** (`WizardCardManager.Cards[i].cost`). This is the one price table for every card seller, so the student vendors' cards scale too, not only Fred & George's.
- **Portrait passwords** sold by Fred & George (`PortraitPasswordManager.PortraitPasswords[0..7].cost`; the three password stands always offer the next unowned one).
- **Not scaled:** pumpkin pasties (30 beans), cauldron cakes (50 beans) and chocolate frogs (10 beans) bought with jellybeans. They are currency for the checks, not checks themselves; 15-bean pasties would make the pasty-priced cards much cheaper. The `VI_FinalWizardCard` (Harry Potter) is not a purchase: Fred & George hand it over once every other card is found.

Both managers are rebuilt from class defaults on every map load, so the mod rewrites the table prices on each settled map, then copies each stand's price (`VendorItem.cost`, which the sale check, the deduction and the vendor menu all read) from the table again, the way the stand itself does after a sale. It only writes over a value that is the slot's vanilla or cheap price, and only after checking the tables against the retail data, so a different game build leaves prices alone.

## Fred & George's shop (InsideHub: 6 card stands, 3 password stands, plus 3 pasty, 3 cake and 6 chocolate frog stands)

### Wizard cards (status `WCS_AvailableToFnGShop`)

| Card | Currency | Normal | Cheap |
|---|---|---:|---:|
| GroganStump | pumpkin pasties | 25 | 13 |
| GunhildaOfGorsemoor | jellybeans | 100 | 50 |
| HesperStarkey | jellybeans | 125 | 63 |
| MirabellaPlunkett | pumpkin pasties | 25 | 13 |
| MountainTroll | jellybeans | 150 | 75 |
| Streeler | jellybeans | 200 | 100 |

### Portrait passwords (sold in order, one at a time)

| # | Currency | Normal | Cheap |
|---:|---|---:|---:|
| 1 | cauldron cakes | 10 | 5 |
| 2 | cauldron cakes | 12 | 6 |
| 3 | cauldron cakes | 12 | 6 |
| 4 | cauldron cakes | 15 | 8 |
| 5 | cauldron cakes | 15 | 8 |
| 6 | cauldron cakes | 20 | 10 |
| 7 | cauldron cakes | 25 | 13 |
| 8 | cauldron cakes | 30 | 15 |
| total | cauldron cakes | 139 | 71 |

## Student vendors' cards (same table, so also scaled)

Status `WCS_AvailableToVendors` (1), or `WCS_AvailableInAdventureLevel` (2), which become vendor cards once the game state reaches `GameStateAvailable` (Open Castle runs in GState090, so all of them).

| Card | Status | Available from | Currency | Normal | Cheap |
|---|---:|---:|---|---:|---:|
| BeatrixBloxam | 1 | GState010 | jellybeans | 25 | 13 |
| CarlottaPinkstone | 1 | GState010 | jellybeans | 100 | 50 |
| CrispinCronk | 1 | GState010 | jellybeans | 100 | 50 |
| DymphnaFurmage | 1 | GState010 | jellybeans | 50 | 25 |
| MungoBonham | 1 | GState010 | jellybeans | 50 | 25 |
| WendelintheWeird | 1 | GState010 | jellybeans | 75 | 38 |
| Cliodne | 1 | GState060 | pumpkin pasties | 30 | 15 |
| EdgarStroulger | 1 | GState060 | jellybeans | 150 | 75 |
| ElfridaClagg | 1 | GState060 | jellybeans | 125 | 63 |
| IgnatiaWildsmith | 1 | GState060 | pumpkin pasties | 10 | 5 |
| NorvelTwonk | 1 | GState060 | jellybeans | 125 | 63 |
| WilfredElphick | 1 | GState060 | pumpkin pasties | 40 | 20 |
| ZouYen | 1 | GState060 | pumpkin pasties | 20 | 10 |
| DorcasWellbeloved | 1 | GState070 | jellybeans | 175 | 88 |
| EthelredTheEverReady | 1 | GState070 | pumpkin pasties | 80 | 40 |
| GlanmorePeakes | 1 | GState070 | pumpkin pasties | 60 | 30 |
| JocundaSykes | 1 | GState070 | jellybeans | 200 | 100 |
| MorganLeFay | 1 | GState070 | pumpkin pasties | 50 | 25 |
| OswaldBeamish | 1 | GState070 | jellybeans | 225 | 113 |
| ThaddeusThurkell | 1 | GState070 | pumpkin pasties | 70 | 35 |
| FalcoAesalon | 2 | GState070 | jellybeans | 175 | 88 |
| HonoriaNutcombe | 2 | GState070 | jellybeans | 175 | 88 |
| ProfessorAmandoDippet | 2 | GState090 | jellybeans | 250 | 125 |
| SacharissaTugwood | 2 | GState090 | jellybeans | 200 | 100 |

## Apworld option (to add when the apworld exists)

```python
class ShopPrices(Choice):
    """Prices of the wizard cards and portrait passwords sold in the castle.
    normal: vanilla prices; shop checks may wait on pumpkin pasties and cauldron cakes.
    cheap: 50% of vanilla, rounded up."""
    display_name = "Shop Prices"
    option_normal = 0
    option_cheap = 1
    default = 0
```

Slot data `shop_prices` (0/1) replaces the ini switch.

## Shopsanity (mod poc9j)

The twins' 6 cards and 8 passwords and the 20 student-vendor purchases are checks (`mod/OpenCastle/apshop.h`). One
VendorManager and one dialog (`HGame.HPPageVendor`) serve both the twins and the students.

- **Twins' card stands** (`VI_WizardCard`, 6): the mod gives each stand one unchecked `F&G Shop: <card>` location
  (WizardCardClass, price and currency from that card's row), shows that location's item on it, and hides stands once
  fewer locations than stands are left. The stands' Timer (Reload, which would pick the next vanilla shop card) is
  skipped. A purchase (state `PostTransaction` with the stand hidden by `Purchased`) checks the location, and the card
  `Purchased` gave is put back like any vanilla card reward.
- **Password stands** (`VI_PortraitPassword`, 3): they sell "the next unowned password", and the table's `Owned` flags
  are the received passwords (portraits open with those). From a stand's `DoingVendorTransaction` to its
  `PostTransaction` the flags are swapped for the checked password locations, so the next password is the first
  unchecked location (its price, its number for `AddPassword`), then the received flags are put back. All three
  stands show that location's item.
- **Student vendors**: they sell the first card with Status 1 (`AvailableToVendors`). The mod keeps exactly one card at
  Status 1 while a purchase is unchecked (purchase k's own card, or another of the 20 if that one was received) and
  sets every other Status-1 card to 5 (received) or 6 (`OwnedByVendor`). While a student's `DoingVendorTransaction`
  starts, that card's table price is purchase k's vanilla price (halved when cheap), so logic's order and prices hold
  whatever was received. The card the student throws shows purchase k's item; picking it up checks k.
- **Dialog**: on the page's first `Vendor_OnDraw` the picture (`imgItem.Image`) becomes the item's art (card face,
  portrait for passwords 1-8, spell shape, pasty / cake / frog, key, creature, map, or the Archipelago logo by tier for
  other games) and the two lines (`lblInfo0`, `lblInfo1`) the name and description from the client's `SHOPTEXT`.
- **shopsanity off**: the apworld locks each location's vanilla card / password there; everything above still runs and
  shows those vanilla items.

Portrait passwords and their dialog portraits: 1 Bertie Bott, 2 Derwent Shimpling, 3 Mopsus, 4 Tilly Toke, 5 Glover
Hipworth, 6 Hesper Starkey, 7 Leopoldina Smethwyck, 8 Bridget Wenlock (`PortraitPasswords[i].DescDlgTag`).
