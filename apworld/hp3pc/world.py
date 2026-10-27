from collections.abc import Mapping
from typing import Any

from BaseClasses import Region
from worlds.AutoWorld import World

from . import items
from . import options as hp3_options


class HP3PCWorld(World):
    """
    Harry Potter and the Prisoner of Azkaban (PC, 2004) in Open Castle form: Hogwarts in its late-year state with
    every story level behind a door, spells and level unlocks as items, and wizard cards opening the Final Day.
    """

    game = items.GAME_NAME
    options_dataclass = hp3_options.HP3PCOptions
    options: hp3_options.HP3PCOptions

    item_name_to_id = items.ITEM_NAME_TO_ID
    item_name_groups = items.ITEM_NAME_GROUPS
    location_name_to_id: dict[str, int] = {}   # TODO: locations.py (data/hp3_location_catalog.csv)

    origin_region_name = "InsideHub"

    def generate_early(self) -> None:
        for name in items.starting_items(self):
            self.multiworld.push_precollected(self.create_item(name))

    def create_regions(self) -> None:
        # TODO: regions and locations. Until then only the hub exists, so seeds can't be generated yet.
        self.multiworld.regions.append(Region(self.origin_region_name, self.player, self.multiworld))

    def create_items(self) -> None:
        pool = items.core_pool(self)
        free = len(self.multiworld.get_unfilled_locations(self.player))
        if len(pool) > free:
            raise RuntimeError(f"{self.game}: {len(pool)} items but only {free} locations (locations aren't written yet)")
        # TODO: each location's vanilla currency bundle, then frogs for the rest
        pool += [items.FILLER_ITEM_NAME] * (free - len(pool))
        self.multiworld.itempool += [self.create_item(n) for n in pool]

    def create_item(self, name: str) -> items.HP3PCItem:
        return items.create_item(self, name)

    def get_filler_item_name(self) -> str:
        return items.FILLER_ITEM_NAME

    def fill_slot_data(self) -> Mapping[str, Any]:
        return self.options.as_dict("goal", "cards_required", "spell_mode", "split_challenge_spells", "shop_prices",
                                    "shortcut_passwords")
