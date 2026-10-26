# Item models: how randomized items look in the world

Goal: every placed randomizer item shows a model of what it is. HP3 items use the game's own model. Spells and
door unlocks use unused art from HP3's packages. Items from other games use an Archipelago logo in three tiers:
- progression: logo with a yellow up arrow;
- useful / normal: plain logo;
- filler: grayscale logo.

## HP3's own models

| Item | Asset | Package |
| --- | --- | --- |
| Cauldron cake | `Spells_Objects.CauldronCake_SM` (StaticMesh) | StaticMeshes/HP3_Gameplay_SM.usx |
| Pumpkin pasty | `Spells_Objects.PumpkinPasty_SM` | HP3_Gameplay_SM.usx |
| Wizard card | `WizardCards.WizardCard_SM` | HP3_Gameplay_SM.usx |
| Marauder's Map | `Maps.map_SM` (static), or `HProps.MarauderMap` (skeletal, the cutscene prop) | HP3_Gameplay_SM.usx, Animations/HProps.ukx |
| Portrait password / shortcut password | `Portrait_Portal.PP_Picture_SM` / `PortraitPortal_SM`; frames `PortraitFrames.*` | HP3_Gameplay_SM.usx, HP3_Tester.usx |
| Jellybeans (filler) | `BeanPickup` | HP3_Gameplay_SM.usx |
| Challenge shield | `ChallengeShields.GoldShield_SM` | HP3_Gameplay_SM.usx |

## Spells: the spell-shape glyphs ("chevrons")

`textures/SpellShapes.utx`, group `Shapes`: 64×64 P8 textures (red is the mask colour), one per spell. The set
includes spells HP3 never uses: `SPSrictusempra`, `SPSdraconifor`, `SPSlapifors`, `SPSdepulso`, `SPSCarpeRetractum`,
`SPSavifors`, `SPSglacius`, `SPSexpecto`, `SPSflipendo`, `SPSalohomora`, `SPSdiffindo`, `SPSskurge`, `SPSlumos`,
`SPSspongify`. They can be shown as a sprite (`DrawType=DT_Sprite`, `Texture=`), or as the skin of a flat mesh.

## Door / area unlocks: keys

HP3 has no key mesh. `textures/HP_FX.utx` has four 32×32 key sprites: `Particles.Key1` (gold, blue glow),
`Key2` (gold), `Key3` (silver) and `Les_RedKey` (red, green glow). None of HP3's scripts reference them by that
path (HP2 leftovers?).

## Items from other games: the Archipelago logo

- HP2PC_AP ([Kryen112/HP2PC_AP](https://github.com/Kryen112/HP2PC_AP)) has a logo mesh and textures
  (`mod/HPArchipelago/Models/APLogoMesh.psk`, `Textures/APLogoTex0.png` / `APLogoArrowTex0.png`).
  - The repo has **no licence**, so by default it's all rights reserved: ask the author before copying anything.
  - Their generators (`tools/gen_logo_mesh.py`, `gen_logo_textures.py`) aren't published.
  - Their pipeline is UE1 `ucc` `#exec` imports, which HP3 can't use (no `ucc` ships with HP3).
- What we can do without it:
  1. Draw our own three textures from the Archipelago logo (the Archipelago repo is MIT-licensed): plain,
     arrow, grayscale.
  2. Write a small texture package (`HP3AP.utx`) with our own package writer. ue2.py already reads the format,
     including P8 / DXT mips.
  3. At runtime, show the texture as a sprite, or as the skin of an existing flat mesh such as
     `WizardCard_SM`. That gives a spinning card with the logo, with no new mesh needed.
