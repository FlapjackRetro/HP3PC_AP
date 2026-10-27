"""
Items for Harry Potter and the Prisoner of Azkaban (PC), Open Castle.

Tiers (decided 2026-10-26, see docs/APWORLD.md):
  progression                 wizard cards, spells, challenge / level unlocks, portrait passwords
  progression_skip_balancing  pumpkin pasty and cauldron cake bundles (shop currency: logic counts totals)
  useful                      shortcut passwords
  filler                      chocolate frogs, the Marauder's Map
  not randomized              jellybeans, challenge stars ("challenge shields" in game), the Harry Potter card

IDs are BASE_ID + a fixed offset per item and never change once released. Card offsets are the card's index in
WizardCardManager.Cards (retail HGame.u), so the client can map an ID straight to the card table.
"""
from __future__ import annotations

from dataclasses import dataclass
from typing import TYPE_CHECKING

from BaseClasses import Item, ItemClassification as IC

if TYPE_CHECKING:
    from .world import HP3PCWorld

GAME_NAME = "Harry Potter 3 PC"   # after Harry Potter 2 PC
BASE_ID = 33_000_000


class HP3PCItem(Item):
    game = GAME_NAME


@dataclass(frozen=True)
class ItemData:
    offset: int
    classification: IC
    group: str
    # what the client does on receipt (documentation for the client and the mod; not used by generation)
    grants: str = ""


# ---- spells -------------------------------------------------------------------------------------------------------
# Always items. In spell_mode: vanilla the first five start in the inventory instead of the pool.
SPELLS: dict[str, ItemData] = {
    "Alohomora":        ItemData(1, IC.progression, "Spells", "spell HGame.AlohomoraSpell"),
    "Depulso":          ItemData(2, IC.progression, "Spells", "spell Depulso"),
    "Lumos":            ItemData(3, IC.progression, "Spells", "wand light (LumosTurnOn)"),
    "Rictusempra":      ItemData(4, IC.progression, "Spells", "spell Rictusempra"),
    "Spongify":         ItemData(5, IC.progression, "Spells", "spell Spongify"),
    "Expecto Patronum": ItemData(6, IC.progression, "Spells", "spell HGame.PatronumSpell"),
    # split_challenge_spells: on -> these four are separate items; off -> granted by the challenge unlocks below
    "Carpe Retractum":  ItemData(7, IC.progression, "Spells", "spell Carpe Retractum (Ron)"),
    "Draconifors":      ItemData(8, IC.progression, "Spells", "spell Draconifors (Hermione)"),
    "Lapifors":         ItemData(9, IC.progression, "Spells", "spell Lapifors (Hermione)"),
    "Glacius":          ItemData(10, IC.progression, "Spells", "spell Glacius"),
}
VANILLA_STARTING_SPELLS = ("Rictusempra", "Alohomora", "Lumos", "Depulso", "Spongify")
CHALLENGE_SPELLS = ("Carpe Retractum", "Draconifors", "Lapifors", "Glacius")

# ---- level unlocks ------------------------------------------------------------------------------------------------
# Mod side: the [Doors] keys in opencastle.c (DOORS table). With split_challenge_spells off, a challenge unlock also
# grants its spell(s) (CHALLENGE_GRANTS).
LEVEL_UNLOCKS: dict[str, ItemData] = {
    "Carpe Retractum Challenge Unlock": ItemData(101, IC.progression, "Level Unlocks", "door CarpeRetractum: Ch1 + Ch1B"),
    "Draconifors Challenge Unlock":     ItemData(102, IC.progression, "Level Unlocks", "door Draconifors: Ch2 + Ch2B"),
    "Glacius Challenge Unlock":         ItemData(103, IC.progression, "Level Unlocks", "door Glacius: Ch3 + Ch3B"),
    "Hogwarts Express Unlock":          ItemData(104, IC.progression, "Level Unlocks", "door Express: Adv1Express"),
    "Patronus Lesson Unlock":           ItemData(105, IC.progression, "Level Unlocks", "door DADA: Adv2ExpPatronum"),
    "Library Unlock":                   ItemData(106, IC.progression, "Level Unlocks", "door Library: Adv3Library"),
    "Shrieking Shack Unlock":           ItemData(107, IC.progression, "Level Unlocks", "door Shack: Adv5Shack"),
    "Paddock Unlock":                   ItemData(108, IC.progression, "Level Unlocks", "door Paddock: Adv6Paddock"),
    "Monster Book Challenge Unlock":    ItemData(109, IC.progression, "Level Unlocks", "door MonsterBook (MBOMDoor)"),
    "Hippogriff Challenge Unlock":      ItemData(110, IC.progression, "Level Unlocks", "door Hippogriff (COMC)"),
    "Doxy Challenge Unlock":            ItemData(111, IC.progression, "Level Unlocks", "door Doxy (well)"),
    "Bonus Room Unlock":                ItemData(112, IC.progression, "Level Unlocks", "door BonusRoom: HP3_BeanBonus (entrance hall south door)"),
}
CHALLENGE_GRANTS = {
    "Carpe Retractum Challenge Unlock": ("Carpe Retractum",),
    "Draconifors Challenge Unlock":     ("Draconifors", "Lapifors"),
    "Glacius Challenge Unlock":         ("Glacius",),
}

# ---- passwords ----------------------------------------------------------------------------------------------------
# Portrait passwords: PortraitPasswordManager entries 0-7 (pp_0..pp_7), numbered in the order the twins sell them.
# They open the PortraitPassword_B-F portraits; which number opens which portrait is still to be mapped.
PASSWORDS: dict[str, ItemData] = {
    **{f"Portrait Password {n + 1}": ItemData(200 + n, IC.progression, "Portrait Passwords", f"pp_{n}") for n in range(8)},
    # entries 8-21 at once, as vanilla's HPAction_GivePortShortcuts. Useful only: logic never needs a shortcut.
    "Shortcut Passwords": ItemData(220, IC.useful, "Shortcut Passwords", "pp_8..pp_21 (EnablePortraitShortcuts)"),
}

# ---- currency and filler ------------------------------------------------------------------------------------------
# Bundles match the fixed amounts vanilla containers hand out (data/hp3_location_catalog.csv): pasties 1-5 and 10,
# cakes 1-4. A location's item is the bundle of what it held in vanilla.
PASTY_BUNDLES = (1, 2, 3, 4, 5, 10)
CAKE_BUNDLES = (1, 2, 3, 4)


def pasty_name(n: int) -> str:
    return "Pumpkin Pasty" if n == 1 else f"{n} Pumpkin Pasties"


def cake_name(n: int) -> str:
    return "Cauldron Cake" if n == 1 else f"{n} Cauldron Cakes"


CURRENCY: dict[str, ItemData] = {
    **{pasty_name(n): ItemData(300 + n, IC.progression_skip_balancing, "Pumpkin Pasties", f"{n} x PumpkinPasty")
       for n in PASTY_BUNDLES},
    **{cake_name(n): ItemData(320 + n, IC.progression_skip_balancing, "Cauldron Cakes", f"{n} x CauldronCake")
       for n in CAKE_BUNDLES},
}
# currency value of each bundle, for logic: (kind, amount)
CURRENCY_VALUE: dict[str, tuple[str, int]] = {
    **{pasty_name(n): ("pasty", n) for n in PASTY_BUNDLES},
    **{cake_name(n): ("cake", n) for n in CAKE_BUNDLES},
}

FILLER: dict[str, ItemData] = {
    # heals on receipt (ChocolateFrog.HealthToAdd); not kept in the inventory
    "Chocolate Frog": ItemData(340, IC.filler, "Filler", "AddHealth(ChocolateFrog.HealthToAdd)"),
    # the map is cosmetic in Open Castle
    "Marauder's Map": ItemData(341, IC.filler, "Filler", "MaraudersMap inventory item"),
}
FILLER_ITEM_NAME = "Chocolate Frog"
# offsets 350-399: reserved for traps (none designed yet)

# ---- wizard cards -------------------------------------------------------------------------------------------------
# (index in WizardCardManager.Cards, class, display name). Item name: "<display name> Card".
# Index 39, Harry Potter, is not an item: Fred & George give it once 74 cards are found (all but it and the five in
# the Wizard Card bonus room, HP3_BeanBonus, which it opens), as in vanilla.
_CARDS = [
    ( 0, "WC_AlgufftheAwful", "Alguff the Awful"),
    ( 1, "WC_AmarilloLestoat", "Amarillo Lestoat"),
    ( 2, "WC_Babayaga", "Babayaga"),
    ( 3, "WC_BeatrixBloxam", "Beatrix Bloxam"),
    ( 4, "WC_Billywig", "Billywig"),
    ( 5, "WC_BlodwynBludd", "Blodwyn Bludd"),
    ( 6, "WC_BowmanWright", "Bowman Wright"),
    ( 7, "WC_Bowtruckle", "Bowtruckle"),
    ( 8, "WC_BrantheBloodthirsty", "Bran the Bloodthirsty"),
    ( 9, "WC_CarlottaPinkstone", "Carlotta Pinkstone"),
    (10, "WC_Cliodne", "Cliodne"),
    (11, "WC_CommonWelshGreenDragon", "Common Welsh Green Dragon"),
    (12, "WC_CordeliaMisericordia", "Cordelia Misericordia"),
    (13, "WC_CountVladDrakul", "Count Vlad Drakul"),
    (14, "WC_CrispinCronk", "Crispin Cronk"),
    (15, "WC_Cyclops", "Cyclops"),
    (16, "WC_CyprianYoudle", "Cyprian Youdle"),
    (17, "WC_DorcasWellbeloved", "Dorcas Wellbeloved"),
    (18, "WC_DoubleEndedNewt", "Double-Ended Newt"),
    (19, "WC_Doxy", "Doxy"),
    (20, "WC_Dumbledore", "Dumbledore"),
    (21, "WC_DymphnaFurmage", "Dymphna Furmage"),
    (22, "WC_EargittheUgly", "Eargit the Ugly"),
    (23, "WC_EdgarStroulger", "Edgar Stroulger"),
    (24, "WC_ElfridaClagg", "Elfrida Clagg"),
    (25, "WC_EthelredTheEverReady", "Ethelred the Ever-Ready"),
    (26, "WC_FalcoAesalon", "Falco Aesalon"),
    (27, "WC_FulberttheFearful", "Fulbert the Fearful"),
    (28, "WC_GiantPurpleToad", "Giant Purple Toad"),
    (29, "WC_GiantSquid", "Giant Squid"),
    (30, "WC_GlanmorePeakes", "Glanmore Peakes"),
    (31, "WC_Gnome", "Gnome"),
    (32, "WC_GodricGryffindor", "Godric Gryffindor"),
    (33, "WC_Goliath", "Goliath"),
    (34, "WC_Gringott", "Gringott"),
    (35, "WC_GroganStump", "Grogan Stump"),
    (36, "WC_GunhildaOfGorsemoor", "Gunhilda of Gorsemoor"),
    (37, "WC_GwenogJones", "Gwenog Jones"),
    (38, "WC_Gytrash", "Gytrash"),
    (39, "WC_HarryPotter", "Harry Potter"),
    (40, "WC_HebrideanBlackDragon", "Hebridean Black Dragon"),
    (41, "WC_HelgaHufflepuff", "Helga Hufflepuff"),
    (42, "WC_HengistofUpperBarnton", "Hengist of Upper Barnton"),
    (43, "WC_HesperStarkey", "Hesper Starkey"),
    (44, "WC_HonoriaNutcombe", "Honoria Nutcombe"),
    (45, "WC_HungarianHorntailDragon", "Hungarian Horntail Dragon"),
    (46, "WC_IgnatiaWildsmith", "Ignatia Wildsmith"),
    (47, "WC_Imp", "Imp"),
    (48, "WC_JocundaSykes", "Jocunda Sykes"),
    (49, "WC_JoscelindWadcock", "Joscelind Wadcock"),
    (50, "WC_Kelpie", "Kelpie"),
    (51, "WC_LadyCarmillaSanguina", "Lady Carmilla Sanguina"),
    (52, "WC_LeticiaSomnolens", "Leticia Somnolens"),
    (53, "WC_MalodoraGrymm", "Malodora Grymm"),
    (54, "WC_Manticore", "Manticore"),
    (55, "WC_MirabellaPlunkett", "Mirabella Plunkett"),
    (56, "WC_MorganLeFay", "Morgan le Fay"),
    (57, "WC_Morholt", "Morholt"),
    (58, "WC_MountainTroll", "Mountain Troll"),
    (59, "WC_MungoBonham", "Mungo Bonham"),
    (60, "WC_NorvelTwonk", "Norvel Twonk"),
    (61, "WC_NorwegianRidgebackDragon", "Norwegian Ridgeback Dragon"),
    (62, "WC_OldMotherHubbard", "Old Mother Hubbard"),
    (63, "WC_OswaldBeamish", "Oswald Beamish"),
    (64, "WC_Phoenix", "Phoenix"),
    (65, "WC_ProfessorAmandoDippet", "Professor Armando Dippet"),
    (66, "WC_RoderickPlumpton", "Roderick Plumpton"),
    (67, "WC_RomanianLonghornDragon", "Romanian Longhorn Dragon"),
    (68, "WC_RowenaRavenclaw", "Rowena Ravenclaw"),
    (69, "WC_SacharissaTugwood", "Sacharissa Tugwood"),
    (70, "WC_SalazarSlytherin", "Salazar Slytherin"),
    (71, "WC_SirHerbertVarney", "Sir Herbert Varney"),
    (72, "WC_Streeler", "Streeler"),
    (73, "wc_ThaddeusThurkell", "Thaddeus Thurkell"),
    (74, "WC_UgtheUnreliable", "Ug the Unreliable"),
    (75, "WC_Unicorn", "Unicorn"),
    (76, "WC_UrgtheUnclean", "Urg the Unclean"),
    (77, "WC_WendelintheWeird", "Wendelin the Weird"),
    (78, "WC_WilfredElphick", "Wilfred Elphick"),
    (79, "WC_ZouYen", "Zou Yen"),]
FINAL_CARD_INDEX = 39
CARD_OFFSET = 1000


def card_item_name(display: str) -> str:
    return f"{display} Card"


CARDS: dict[str, ItemData] = {
    card_item_name(name): ItemData(CARD_OFFSET + idx, IC.progression, "Wizard Cards", cls)
    for idx, cls, name in _CARDS if idx != FINAL_CARD_INDEX
}
CARD_CLASS: dict[str, str] = {card_item_name(name): cls for idx, cls, name in _CARDS if idx != FINAL_CARD_INDEX}

# ---- tables -------------------------------------------------------------------------------------------------------
ITEM_TABLE: dict[str, ItemData] = {**SPELLS, **LEVEL_UNLOCKS, **PASSWORDS, **CURRENCY, **FILLER, **CARDS}
ITEM_NAME_TO_ID: dict[str, int] = {name: BASE_ID + data.offset for name, data in ITEM_TABLE.items()}
ITEM_NAME_GROUPS: dict[str, set[str]] = {}
for _name, _data in ITEM_TABLE.items():
    ITEM_NAME_GROUPS.setdefault(_data.group, set()).add(_name)
ITEM_NAME_GROUPS["Currency"] = set(CURRENCY)

VICTORY_EVENT = "Final Day Door Opened"   # event item, no ID


def create_item(world: HP3PCWorld, name: str) -> HP3PCItem:
    if name == VICTORY_EVENT:
        return HP3PCItem(name, IC.progression, None, world.player)
    return HP3PCItem(name, ITEM_TABLE[name].classification, ITEM_NAME_TO_ID[name], world.player)


def starting_items(world: HP3PCWorld) -> list[str]:
    """Items the player starts with (precollected, not in the pool)."""
    start = list(VANILLA_STARTING_SPELLS) if world.options.spell_mode == world.options.spell_mode.option_vanilla else []
    if world.options.shortcut_passwords == world.options.shortcut_passwords.option_start_unlocked:
        start.append("Shortcut Passwords")
    return start


def core_pool(world: HP3PCWorld) -> list[str]:
    """Every non-currency item that goes into the pool, by name. Currency bundles come from the locations (each
    location's vanilla bundle), and frogs fill whatever is left; that part needs the location table."""
    start = set(starting_items(world))
    names = [n for n in SPELLS if n not in start
             and (n not in CHALLENGE_SPELLS or world.options.split_challenge_spells)]
    names += list(LEVEL_UNLOCKS)
    names += [n for n in PASSWORDS if n not in start]
    names += list(CARDS)
    names.append("Marauder's Map")
    return names
