"""Regions: Hogwarts (InsideHub, where the game starts) and one region per level / challenge / portrait, each entered
from the castle or the grounds through the door the mod gates. A region's entrance needs its unlock item; the spells
needed inside are on the locations (location_table.py), so the regions stay a plain star around the castle."""
from __future__ import annotations

from typing import TYPE_CHECKING

from BaseClasses import Region

if TYPE_CHECKING:
    from .world import HP3PCWorld

ORIGIN = "Hogwarts"

# region -> (parent region, item needed to enter, or None)
REGIONS: dict[str, tuple[str, str | None]] = {
    "Hogwarts": ("", None),
    "Grounds": ("Hogwarts", None),
    "Dungeons": ("Hogwarts", None),
    "Fred and George's Shop": ("Hogwarts", None),
    "Student Vendors": ("Hogwarts", None),
    "Hogwarts Express": ("Hogwarts", "Hogwarts Express Unlock"),
    "Patronus Lesson": ("Hogwarts", "Patronus Lesson Unlock"),
    "Library": ("Hogwarts", "Library Unlock"),
    "Shrieking Shack Tunnel": ("Dungeons", "Shrieking Shack Unlock"),
    "Paddock": ("Hogwarts", "Paddock Unlock"),
    "Wizard Card Bonus Room": ("Hogwarts", "Bonus Room Unlock"),
    "Monster Book Challenge": ("Hogwarts", "Monster Book Challenge Unlock"),
    "Hippogriff Challenge": ("Grounds", "Hippogriff Challenge Unlock"),
    "Doxy Challenge": ("Grounds", "Doxy Challenge Unlock"),
    "Carpe Retractum Challenge": ("Hogwarts", "Carpe Retractum Challenge Unlock"),
    "Draconifors Challenge": ("Hogwarts", "Draconifors Challenge Unlock"),
    "Glacius Challenge": ("Hogwarts", "Glacius Challenge Unlock"),
    "Ron's Final Exam": ("Hogwarts", "Ron's Final Exam Unlock"),
    "Hermione's Final Exam": ("Hogwarts", "Hermione's Final Exam Unlock"),
    "Harry's Final Exam": ("Hogwarts", "Harry's Final Exam Unlock"),
    "Portrait B": ("Hogwarts", "Portrait Password 4"),
    "Portrait C": ("Hogwarts", "Portrait Password 6"),
    "Portrait D": ("Hogwarts", "Portrait Password 7"),
    "Portrait E": ("Hogwarts", "Portrait Password 8"),
    "Portrait F": ("Hogwarts", "Portrait Password 2"),
}


def create_regions(world: HP3PCWorld) -> dict[str, Region]:
    mw, player = world.multiworld, world.player
    regions = {name: Region(name, player, mw) for name in REGIONS}
    mw.regions += regions.values()
    for name, (parent, item) in REGIONS.items():
        if not parent:
            continue
        rule = (lambda state, i=item: state.has(i, player)) if item else None
        regions[parent].connect(regions[name], f"{parent} -> {name}", rule)
    return regions
