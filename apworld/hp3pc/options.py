from dataclasses import dataclass

from Options import Choice, PerGameCommonOptions, Range, Toggle


class Goal(Choice):
    """What opens the Final Day door (the goal).
    final_day_cards: collect the number of wizard cards set by cards_required."""
    display_name = "Goal"
    option_final_day_cards = 0
    default = option_final_day_cards


class CardsRequired(Range):
    """Wizard cards needed to open the Final Day door (79 are in the pool; the Harry Potter card isn't one)."""
    display_name = "Cards Required"
    range_start = 1
    range_end = 79
    default = 40


class SpellMode(Choice):
    """vanilla: start with Rictusempra, Alohomora, Lumos, Depulso and Spongify; Expecto Patronum and the challenge
    spells are in the pool.
    hard: start with no spells. Early progress comes from the few checks reachable without any."""
    display_name = "Spell Mode"
    option_vanilla = 0
    option_hard = 1
    default = option_vanilla


class SplitChallengeSpells(Toggle):
    """Off: each challenge unlock also gives its spell(s) (Carpe Retractum; Draconifors and Lapifors; Glacius).
    On: the four spells are separate items."""
    display_name = "Split Challenge Spells"


class ShopPrices(Choice):
    """Prices of the wizard cards and portrait passwords sold in the castle.
    normal: vanilla prices; shop checks may wait on pumpkin pasties and cauldron cakes.
    cheap: 50% of vanilla, rounded up."""
    display_name = "Shop Prices"
    option_normal = 0
    option_cheap = 1
    default = option_normal


class ShortcutPasswords(Choice):
    """item: the shortcut passwords are one item in the pool (useful, never required by logic).
    start_unlocked: every shortcut is open from the start."""
    display_name = "Shortcut Passwords"
    option_item = 0
    option_start_unlocked = 1
    default = option_item


@dataclass
class HP3PCOptions(PerGameCommonOptions):
    goal: Goal
    cards_required: CardsRequired
    spell_mode: SpellMode
    split_challenge_spells: SplitChallengeSpells
    shop_prices: ShopPrices
    shortcut_passwords: ShortcutPasswords
