"""Access rules: spells, unlocks, passwords and shop currency.

Requirements in location_table.py are plain item names. Spells go through has_spell: with split_challenge_spells
off, the four challenge spells aren't items, and their challenge unlock grants them instead (items.CHALLENGE_GRANTS).

Currency is counted on the state (World.collect / remove add each bundle's pasties and cakes to PASTY_KEY /
CAKE_KEY). Shop and vendor purchases are logic-gated on the running total of the purchases before them in their
sequence, since what's spent is gone:
  - Fred & George's portrait passwords sell in order: password k needs the cakes for passwords 1..k.
  - The student vendors sell in a fixed order: purchase k needs the pasties of the pasty-priced purchases 1..k.
  - Fred & George's two pasty-priced cards: the cheaper first (25, then 50 in total).
Jellybean prices are assumed affordable (jellybeans aren't randomized and are everywhere). Buying pasties and cakes
with jellybeans, the repeatable event rewards and the random-amount containers are a safety net, never logic.
"""
from __future__ import annotations

import math
from typing import TYPE_CHECKING, Callable

from BaseClasses import CollectionState

from . import items
from .location_table import LocationData

if TYPE_CHECKING:
    from .world import HP3PCWorld

PASTY_KEY = "Pumpkin Pasties (count)"
CAKE_KEY = "Cauldron Cakes (count)"

_SPELL_FROM_UNLOCK = {spell: unlock for unlock, spells in items.CHALLENGE_GRANTS.items() for spell in spells}


def price(world: HP3PCWorld, amount: int) -> int:
    if world.options.shop_prices == world.options.shop_prices.option_cheap:
        return math.ceil(amount / 2)
    return amount


def requirement_names(world: HP3PCWorld, names: tuple) -> tuple:
    """Item names to check for a location's requirements, with challenge spells mapped to their unlock when the
    spells aren't separate items."""
    out = []
    for n in names:
        if n in _SPELL_FROM_UNLOCK and not world.options.split_challenge_spells:
            n = _SPELL_FROM_UNLOCK[n]
        if n not in out:
            out.append(n)
    return tuple(out)


def currency_thresholds(world: HP3PCWorld, locations: tuple[LocationData, ...]) -> dict[str, tuple[int, int]]:
    """(pasties, cakes) needed in total for each shop / vendor location."""
    need: dict[str, tuple[int, int]] = {}
    cakes = 0
    for loc in sorted((l for l in locations if l.kind == "shop" and l.actor.startswith("pp_")), key=lambda l: l.actor):
        cakes += price(world, loc.price[1])
        need[loc.name] = (0, cakes)
    pasties = 0
    for loc in sorted((l for l in locations if l.kind == "vendor"), key=lambda l: int(l.actor.split("_")[1])):
        if loc.price[0] == "pasty":
            pasties += price(world, loc.price[1])
        need[loc.name] = (pasties, 0)
    pasties = 0
    for loc in sorted((l for l in locations if l.kind == "shop" and not l.actor.startswith("pp_")),
                      key=lambda l: (l.price[0] != "pasty", l.price[1], l.name)):
        if loc.price[0] == "pasty":
            pasties += price(world, loc.price[1])
            need[loc.name] = (pasties, 0)
        else:
            need[loc.name] = (0, 0)
    return need


def location_rule(world: HP3PCWorld, loc: LocationData, need: tuple[int, int] = (0, 0)) -> Callable[[CollectionState], bool]:
    player = world.player
    names = requirement_names(world, loc.requires)
    pasties, cakes = need
    if not names and not pasties and not cakes:
        return lambda state: True

    def rule(state: CollectionState) -> bool:
        if names and not state.has_all(names, player):
            return False
        counts = state.prog_items[player]
        return counts[PASTY_KEY] >= pasties and counts[CAKE_KEY] >= cakes
    return rule


def goal_rule(world: HP3PCWorld) -> Callable[[CollectionState], bool]:
    player, n = world.player, world.options.cards_required.value
    return lambda state: state.has_group("Wizard Cards", player, n)
