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

## Decided (2026-10-26)

| Item | Look |
| --- | --- |
| Pasty, cake, wizard card | their own meshes |
| Marauder's Map | its own mesh |
| Portrait / shortcut passwords | the password scroll (the one Peeves carries and Fred & George sell; `HProps.PeevesScroll`) |
| Carpe Retractum, Alohomora, Depulso, Draconifors, Expecto Patronum, Glacius, Lapifors, Lumos, Rictusempra, Spongify | their spell-shape glyph (`SpellShapes.Shapes.SPS*`) |
| Door / area unlocks | key, player option `key_style`: **`drawn`** (default, `HP3AP.APKeyDrawn`, our own drawing) / `upscaled` (`HP3AP.APKeyUpscaled`, Key2 at 4×) / `original` (`HP_FX.Particles.Key2`, 32 px). The glow keys (`Key1`, `Les_RedKey`) draw a black square when masked, so they're out. |
| Items from other games | Archipelago logo, three tiers |

## Building the Archipelago textures

    python tools/gen_ap_logos.py <Archipelago>/data/icon.png build/aplogo 128
    python tools/utxwriter.py HP3AP.utx APLogoProgression=build/aplogo/APLogoProgression.png \
        APLogoUseful=build/aplogo/APLogoUseful.png APLogoFiller=build/aplogo/APLogoFiller.png

- `utxwriter.py` writes a version-129 texture package. Each image becomes a masked P8 texture with a full mip
  chain: transparent pixels map to palette index 0, as in HP3's own sprite textures.
- `ue2.py` reads the result back, and the mip offsets are checked.
- In game the objects are `HP3AP.APLogoProgression` / `APLogoUseful` / `APLogoFiller`. `install_opencastle.bat`
  copies `HP3AP.utx` into the game's `textures` folder.
- The poc6 test (`[Test] APLogos=1` in HP3_OpenCastle.ini) loads them with `StaticLoadObject` and draws every
  cake / pasty / card as one of the tiers.

## Depth

A texture is flat. Depth needs a mesh to put it on:
1. **An existing HP3 mesh with the texture as its skin**, e.g. `WizardCard_SM` (a card with thickness) or a
   coin- or tile-shaped mesh. No new file format is needed, but the mesh's UV layout decides how the texture
   wraps, so each candidate has to be checked.
2. **Our own StaticMesh in the package**: an extruded token with the logo or glyph on both faces and a coloured
   rim. That needs `utxwriter.py` to also write `StaticMesh` exports (vertex / UV / index buffers, sections,
   collision), a bigger job than textures.
3. **Sprites** (`DrawType=DT_Sprite`) always face the camera, so they never look edge-on. This is what the test
   uses.

## Test result (poc6d, 2026-10-26)

- `HP3AP.utx` (written by `utxwriter.py`) loads with `StaticLoadObject` and renders in game. The progression
  logo was seen on a cake thrown out of a chest in Adv5Shack, at `DrawScale` 0.3 on a 128 px texture: an OK
  size.
- Swapping works both for pickups placed in the map (at settle) and for goodies spawned later (after
  `PostBeginPlay`).
- Lesson: never keep a texture pointer across a level change. Anything unreferenced is garbage collected, so
  reload with `StaticLoadObject` each time (two poc6 crashes).

## Keys (decided after the poc6f grid test)

- **Default:** the drawn key, which looks best in game. All three styles stay available because the community may
  prefer another one, so `key_style` is a player option.
- **Building the package:** `tools/build_hp3ap.py <icon.png> <game folder> HP3AP.utx` builds everything.
  - `APKeyUpscaled` is generated from the player's own `HP_FX.utx`, so the installer should run the build on the
    player's machine. No EA-derived art is shipped or committed.
