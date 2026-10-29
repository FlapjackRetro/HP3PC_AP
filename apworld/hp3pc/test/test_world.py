"""Generation tests on Archipelago's WorldTestBase (run from an Archipelago checkout with the world in worlds/hp3pc:
python -m unittest worlds.hp3pc.test)."""
import collections

from test.bases import WorldTestBase

from .. import items
from ..location_table import LOCATIONS
from ..rules import requirement_names


class TestLocationTable(WorldTestBase):
    game = items.GAME_NAME

    def test_names_unique(self):
        names = [loc.name for loc in LOCATIONS]
        self.assertEqual(len(names), len(set(names)))

    def test_every_card_has_one_vanilla_location(self):
        cards = collections.Counter(loc.card for loc in LOCATIONS if loc.card)
        self.assertEqual(set(cards), set(items.CARDS))
        self.assertTrue(all(v == 1 for v in cards.values()), [k for k, v in cards.items() if v > 1])

    def test_vanilla_bundles_are_items(self):
        for loc in LOCATIONS:
            if not loc.card and (loc.pasties or loc.cakes):
                self.assertIn(items.bundle_name(loc.pasties, loc.cakes), items.CURRENCY, loc.name)

    def test_requirements_are_items(self):
        for loc in LOCATIONS:
            for name in loc.requires:
                self.assertIn(name, items.ITEM_TABLE, loc.name)
            for name in requirement_names(self.world, loc.requires):
                self.assertIn(name, items.ITEM_TABLE, loc.name)


class TestDefault(WorldTestBase):
    game = items.GAME_NAME


class TestHardSplitCheap(WorldTestBase):
    game = items.GAME_NAME
    options = {"spell_mode": "hard", "split_challenge_spells": True, "shop_prices": "cheap",
               "shortcut_passwords": "start_unlocked"}


class TestAllCards(WorldTestBase):
    game = items.GAME_NAME
    options = {"cards_required": 79, "split_challenge_spells": True}

    def test_goal_needs_every_card(self):
        self.collect_all_but(["Zou Yen Card", items.VICTORY_EVENT])
        self.assertFalse(self.can_reach_location("Final Day Door"))
        self.collect_by_name("Zou Yen Card")
        self.assertTrue(self.can_reach_location("Final Day Door"))


class TestRules(WorldTestBase):
    game = items.GAME_NAME
    options = {"spell_mode": "hard"}

    def test_challenge_unlock_grants_spells(self):
        loc = "Glacius: Pumpkin Pasty 37"
        self.assertFalse(self.can_reach_location(loc))
        self.collect_by_name(["Glacius Challenge Unlock", "Rictusempra", "Spongify"])
        self.assertTrue(self.can_reach_location(loc))   # Glacius comes with the unlock (split off)

    def test_password_cakes(self):
        self.collect_by_name(["Alohomora"])
        self.assertFalse(self.can_reach_location("F&G Shop: Portrait Password 1"))
        # progression cake-only bundles, smallest first, until 10 to 21 cakes (password 1 costs 10, 1 + 2 cost 22)
        bundles = sorted((i for i in self.multiworld.itempool
                          if i.advancement and i.name in items.CURRENCY_VALUE and items.CURRENCY_VALUE[i.name][0] == 0),
                         key=lambda i: items.CURRENCY_VALUE[i.name][1])
        total = 0
        for item in bundles:
            if total >= 10:
                break
            self.collect(item)
            total += items.CURRENCY_VALUE[item.name][1]
        self.assertTrue(10 <= total < 22, total)
        self.assertTrue(self.can_reach_location("F&G Shop: Portrait Password 1"))    # 10
        self.assertFalse(self.can_reach_location("F&G Shop: Portrait Password 2"))   # 22
