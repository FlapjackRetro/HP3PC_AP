"""Item table checks that don't need locations (run: python -m pytest worlds/hp3pc/test)."""
import unittest

from BaseClasses import ItemClassification as IC

from .. import items


class TestItemTable(unittest.TestCase):
    def test_ids_unique(self):
        ids = list(items.ITEM_NAME_TO_ID.values())
        self.assertEqual(len(ids), len(set(ids)))

    def test_cards(self):
        self.assertEqual(len(items.CARDS), 79)
        self.assertNotIn("Harry Potter Card", items.CARDS)
        self.assertTrue(all(d.classification == IC.progression for d in items.CARDS.values()))

    def test_tiers(self):
        for name in items.CURRENCY:
            self.assertEqual(items.ITEM_TABLE[name].classification, IC.progression_skip_balancing, name)
        self.assertEqual(items.ITEM_TABLE["Shortcut Passwords"].classification, IC.useful)
        self.assertEqual(items.ITEM_TABLE["Chocolate Frog"].classification, IC.filler)
        self.assertEqual(items.ITEM_TABLE["Marauder's Map"].classification, IC.filler)
        for n in range(1, 9):
            self.assertEqual(items.ITEM_TABLE[f"Portrait Password {n}"].classification, IC.progression)
        for name in list(items.SPELLS) + list(items.LEVEL_UNLOCKS):
            self.assertEqual(items.ITEM_TABLE[name].classification, IC.progression, name)

    def test_challenge_grants_cover_split_spells(self):
        granted = {s for spells in items.CHALLENGE_GRANTS.values() for s in spells}
        self.assertEqual(granted, set(items.CHALLENGE_SPELLS))


class _Opt:
    def __init__(self, value, **named):
        self.value = value
        for k, v in named.items():
            setattr(self, k, v)

    def __eq__(self, other):
        return self.value == other

    def __bool__(self):
        return bool(self.value)


class _World:
    def __init__(self, spell_mode=0, split=0, shortcuts=0):
        class O:
            pass
        self.options = O()
        self.options.spell_mode = _Opt(spell_mode, option_vanilla=0, option_hard=1)
        self.options.split_challenge_spells = _Opt(split)
        self.options.shortcut_passwords = _Opt(shortcuts, option_item=0, option_start_unlocked=1)


class TestPool(unittest.TestCase):
    def test_vanilla(self):
        w = _World()
        self.assertEqual(set(items.starting_items(w)), set(items.VANILLA_STARTING_SPELLS))
        pool = items.core_pool(w)
        self.assertIn("Expecto Patronum", pool)
        self.assertIn("Glacius Challenge Unlock", pool)
        self.assertNotIn("Glacius", pool)
        self.assertNotIn("Lumos", pool)
        self.assertIn("Shortcut Passwords", pool)
        # 1 spell + 15 unlocks + 9 passwords + 79 cards + map
        self.assertEqual(len(pool), 1 + 15 + 9 + 79 + 1)

    def test_hard_split(self):
        w = _World(spell_mode=1, split=1, shortcuts=1)
        self.assertEqual(items.starting_items(w), ["Shortcut Passwords"])
        pool = items.core_pool(w)
        self.assertEqual(len([n for n in pool if n in items.SPELLS]), 10)
        self.assertNotIn("Shortcut Passwords", pool)
