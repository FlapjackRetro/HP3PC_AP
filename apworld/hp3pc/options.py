from dataclasses import dataclass

from Options import Choice, DefaultOnToggle, PerGameCommonOptions, Range, Toggle


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
    hard: start with no spells. Only a few checks are open at the start (on the grounds, and the cards bought with
    jellybeans); in your own world they hold the five vanilla starting spells. If too few are open (shopsanity or
    secretsanity off), you start with one or two of those spells instead."""
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


class Secretsanity(DefaultOnToggle):
    """On: every catalogued secret area is a location (found when you step into it).
    Off: the secret areas aren't locations, and finding one does nothing for the multiworld."""
    display_name = "Secretsanity"


class Candysanity(DefaultOnToggle):
    """On: every catalogued pumpkin pasty and cauldron cake placed in a level is a location, and its pasty or cake is
    an item in the pool.
    Off: they stay vanilla: picking one up gives you the pasty or cake, and logic counts them for the shops.
    Pasties and cakes in chests and knights are checks either way."""
    display_name = "Candysanity"


class Shopsanity(DefaultOnToggle):
    """On: Fred & George's 6 wizard cards and 8 portrait passwords, and the 20 student-vendor card purchases, sell
    items from the multiworld (the stand, the thrown card and the sale dialog show the item).
    Off: each of them sells its vanilla card or password (still a location, with that item locked in)."""
    display_name = "Shopsanity"


@dataclass
class HP3PCOptions(PerGameCommonOptions):
    goal: Goal
    cards_required: CardsRequired
    spell_mode: SpellMode
    split_challenge_spells: SplitChallengeSpells
    shop_prices: ShopPrices
    shortcut_passwords: ShortcutPasswords
    secretsanity: Secretsanity
    candysanity: Candysanity
    shopsanity: Shopsanity
