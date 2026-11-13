# Retail copy reference

The copy this project is developed and tested against. If your files match these hashes, the mod, the setup script
and the framerate patch will work as documented. A file that doesn't match may still work, but it is untested: please
say which files differ when you report a problem.

Check your install automatically, before or after `setup.bat`:

```powershell
.\tools\verify_retail.ps1
# or, for another install folder:
.\tools\verify_retail.ps1 -GameDir 'D:\Games\Harry Potter and the Prisoner of Azkaban(TM)'
```

It checks every installed file against [`data/retail_manifest.csv`](../data/retail_manifest.csv) and lists the files
setup added (HP3-UHDE's DLLs, the plugins). It reports a framerate-patched `Engine.dll` as patched rather than as a
mismatch.

## The disc

| | |
| --- | --- |
| Edition | Retail PC CD-ROM, US, v1.0 (EA, 2004) |
| Volume label | `HPPoA` (CDFS, 521 MB) |
| Disc root | `00000000.016`, `00000000.256`, `00000001.TMP`, `00000002.TMP`, `AutoRun\`, `AutoRun.exe`, `autorun.inf`, `AutoRunGUI.dll`, `common_filelist.txt`, `compressed.zip` (478,592,828 bytes), `DirectX\`, `eauninstall.exe`, `hppoa.ico`, `setup.exe`, `Support\`, `System\` |
| Copy protection | SafeDisc 3.20 or later: `hppoa.exe` has the `stxt774` / `stxt371` sections and the `BoG_` signature, and it extracts the `~e5.0001` start-up helper. Whether it needs SafeDiscShim: [SETUP.md](SETUP.md) step 1. |
| Installed to | `C:\Program Files (x86)\EA GAMES\Harry Potter and the Prisoner of Azkaban(TM)\` (803 files) |
| Tested on | Windows 11 Pro 25H2 (build 26200) |

The hashes below are of a fresh install, before the game was first started and before `setup.bat`.

## Executable and engine code

| File | Bytes | SHA-256 | What the mod does with it |
| --- | ---: | --- | --- |
| `system\hppoa.exe` | 1,455,177 | `1ac9c1e37b1cd8d9c581558be8c8ebb607ef0853b66c698fa3b881dcb9b54b0d` | The game executable. Never modified, never shipped. SafeDisc-protected (see [SETUP.md](SETUP.md) step 1). |
| `system\Core.dll` | 720,896 | `b22cc27f6454eff97909ec0aa1b54a0c7aecdef2c5202e762a245ef431f92bbc` | The plugins hook its exports in memory (ProcessEvent, FindFunction, LoadObject, FString, AddToRoot). |
| `system\Engine.dll` | 4,980,736 | `73bbe178769b51429062c94ca0a209da3bd6e40d020588e8a4ca401427ecc34a` | The plugins hook its exports in memory (travel URLs, collision, animation, actor render data). **Patched on disk** by the framerate fix (below). |
| `system\D3DDrv.dll` | 651,264 | `fd0ef7c6950493ac8e2a8362e0968ae14fcf6463a4fa844e4365ae49d24e6c4e` | Renderer. It loads HP3-UHDE's `d3d8.dll` → `d3d9.dll` (Ultimate ASI Loader), which loads the plugins. |

## Script packages (the plugins find classes, properties and functions in these by name)

| File | Bytes | SHA-256 | What the mod does with it |
| --- | ---: | --- | --- |
| `system\Core.u` | 78,510 | `cb6a84a03279b9f6c597c296b43a89b5d865e406e4654866c55216b3b0982b39` |  |
| `system\Engine.u` | 2,518,546 | `3b5c2b2002dbc8522e8a932e2733228f7f60711d55a9ccb1a033621b3a12f1ef` |  |
| `system\kwGame.u` | 1,589,327 | `9ee25ea5d804d9892a23276ef9e1bf4e5c4dfc452ad66aa09895fd0d6abb61f3` | KWGame: game state. |
| `system\GUI.u` | 146,510 | `11bb138c85a01872444b2ba9ddfb33872c331fa62e724a3204165ad91d5c133e` | Menu and dialog widgets (the shop dialog). |
| `system\HGame.u` | 11,253,595 | `d0981cc5fc704391e3f152f1e94295b2e0440b2660b2b5027dba7eb2cd889ef1` | Nearly all game logic the mod changes. `tools/build_card_textures.py` also reads its card skins. |

## Assets the mod borrows (item models, icons)

| File | Bytes | SHA-256 | What the mod does with it |
| --- | ---: | --- | --- |
| `textures\HP3_Menu.utx` | 26,257,254 | `9ff4826fc0d1bc1f02347ec16729d47bb69d9869695326a54e21d8cd663cd2cd` | Icons, portraits, collector cards. |
| `textures\SpellShapes.utx` | 106,425 | `32ea1eab8b660a63ce4f71c84d84feb135370927f3d67447b33ae0814e7324cf` | Spell icons. |
| `textures\HP_FX.utx` | 1,879,565 | `b39fb883600a43ed642beaee93d9f77d2172290c5515882abb8276e1b52aa651` | Read at setup by `tools/build_hp3ap.py` (APKeyUpscaled). |
| `Animations\HPCharacters.ukx` | 15,783,072 | `fdd618f39f7b2be7bf87073766dcc29e2d6ae00490c7e2940e355a192a95aae6` | Character models (shop looks). |
| `Animations\HProps.ukx` | 8,008,715 | `a89eaee35112e0d263fe284a8ea5e4c32d4caaf2217952d2f14249c9a567451f` | Prop models. |
| `StaticMeshes\HP3_Gameplay_SM.usx` | 2,102,124 | `33bd4e355b0621c96f89d2d43d4332bda71964a353b49c2bb41b92a12726b06d` | Cards, cauldron cakes, pumpkin pasties. |
| `StaticMeshes\HP3_InteriorAccents_SM.usx` | 4,542,800 | `68f16255ce0244292b402ecd37949538c02422f658b6def17d65c33aa282d270` | Password scroll. |
| `StaticMeshes\HP3_InterArch_SM.usx` | 4,109,474 | `9476d2dda1dd5322fc3a5badd553ded11910534b5d66331e77b363158114d307` | Portcullis (chamber gates). |
| `sounds\soundeffects.uax` | 49,282,767 | `7cb9574eaae61275b2074b65590f07c162fc9e6568e40417f315d57f990ba9fe` | Pickup sound. |

## Config files patched by setup (HP3-UHDE's ini patches; the originals are backed up as `.bak`)

| File | Bytes | SHA-256 | What the mod does with it |
| --- | ---: | --- | --- |
| `system\Default.ini` | 9,998 | `12d9a8af7214c5a6322a1a5c3c952e8a1c10e360d8982376abccfeea7a4e295a` |  |
| `system\defuser.ini` | 4,396 | `70c7ea3c53f7a3593878774f4166fc30ebb0c4aeddc1935d306d508db81960e0` |  |

## Maps

The mod spawns, retargets and hides actors by name in these maps ([OPEN_CASTLE_CHANGES.md](OPEN_CASTLE_CHANGES.md)),
and its location tables use the actors' names and positions. All 34 are in the manifest.

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `maps\Entry.unr` | 179,027 | `ea5fa42471c62cd46758c8953a3e82623d2666fbdd012280aef92d23e8c757f9` |
| `maps\HP_Preamble.unr` | 174,512 | `b03ce13420bdfde549f38fdf1d23a6eff0cb30803ea7f223d21022abff280545` |
| `maps\HP3_Adv1Express.unr` | 4,649,775 | `f311a8d5e761422811050c4d31a3382dd1402ca25e5871a263e5ad2cb6e7f8e8` |
| `maps\HP3_Adv2ExpPatronum.unr` | 2,308,280 | `c4359878c5bfdc27c57648346f667d12713484fc1d07be6b5773bdc080487b5f` |
| `maps\HP3_Adv3Library.unr` | 2,563,797 | `99632fc413c938bea9a21f409edff3c73114c24447ac04812ccd3eece943fec4` |
| `maps\HP3_Adv5Shack.unr` | 6,863,292 | `34a1cbeaffc07ca7402a8ae9837c3dee82efc8c5b702906a4dbdaccb46b44639` |
| `maps\HP3_Adv6Paddock.unr` | 3,371,173 | `253533be6c76b87f6ba57ca1c13c8026e3312dc34ca63d76afe382e1bc5c0ea1` |
| `maps\HP3_BeanBonus.unr` | 3,257,472 | `0a8be15101084f3e4dc2a988040e89a88ae9d3bfe649174afd9f748878e40e0b` |
| `maps\HP3_BuckyExecuted.unr` | 2,829,278 | `1363ea8ac1ffe9d309b507893c31b9e73fc8ef32cf71e3f4ab7923a39202f28c` |
| `maps\HP3_Ch1CarpeRetractum.unr` | 5,075,050 | `6a09530dee3ea8d62aefafd39571c74a93386705fb768f3a8fa1e31c5cc8b5d5` |
| `maps\HP3_Ch1CarpeRetractumB.unr` | 6,552,176 | `983ab514f4f3a1690b53f6d54ecee0b16d8d51dc3e6e6bf067411b3c4dff5f08` |
| `maps\HP3_Ch2Draconifors.unr` | 6,358,291 | `ce45b5a4ec07206995a850b397e7071e42d937e80e9d7bad1923efb51c0cacd1` |
| `maps\HP3_Ch2DraconiforsB.unr` | 6,877,601 | `f4e185d9f5a7726c64a3ebe2d7cc05427d6e8f7a8b1d820d8e6bbb2b6e5e354e` |
| `maps\HP3_Ch3Glacius.unr` | 4,603,861 | `1d292a4c6103e116da86ba7953b6224e5d21cc49f869b331d19e5d0ff257f5a0` |
| `maps\HP3_Ch3GlaciusB.unr` | 6,986,630 | `3099c315bf40c161b5e817167e08100d8e0b4f2d3a036a49437e40b17e674597` |
| `maps\HP3_COMC.unr` | 2,465,601 | `f09db4c3987ee3ae5e54e5da90d4f4b444e7297d09d0682061bf5a53f76cff07` |
| `maps\HP3_DarkTower.unr` | 4,097,870 | `761cf87b76b060df965878acdb44a95bd89fad709aea393352f750929e67883b` |
| `maps\HP3_DementorBattle.unr` | 1,155,596 | `303ab7c1bc350e3e9884dd57cb33d2e168316024d2f9522318bbbf8424182983` |
| `maps\HP3_DungeonHUB.unr` | 2,540,623 | `3df7329c990cbac4693224eec14c4e264c5920872b232709564363e6dedcdd4d` |
| `maps\HP3_FA1Ron.unr` | 6,958,509 | `04adf65011f11d9e2f1b61a2535c42c55df383252929e08be380ef5e48d8215c` |
| `maps\HP3_FA2Hermione.unr` | 6,763,993 | `d51ccd4a3fca33c3836fd6156d48385fa26ad677c87ffd18b9d5360292cb8c41` |
| `maps\HP3_FA3Harry.unr` | 4,164,325 | `0f536830352fa2119f87fe193f074de76267b80f7f94f9c340535e75c757f5ac` |
| `maps\HP3_Final.unr` | 4,225,789 | `c3b678cea8c21b39d857093dab2520c807d137254801f5caacf31815b5c2e67d` |
| `maps\HP3_GroundsDada.unr` | 1,842,444 | `860bb164d7b84296e243df9b6d34b472fdd6da860f6d6c29c2d343a5311d4813` |
| `maps\HP3_GroundsHub.unr` | 3,622,501 | `46127d2bdd686a442875f81368bef7d2187d724669212b93c6d36fb438c48459` |
| `maps\HP3_Infirmary.unr` | 1,669,290 | `79c8736fac8d927ba5a901de3e57321486139929a166c9d26326e4f2e7ce99d9` |
| `maps\HP3_InsideHub.unr` | 9,766,176 | `baef1154fb4c74ee36f0cc94236ac2e395564ed0f1e5097a9cd5b36e93955cc8` |
| `maps\HP3_PortraitPassword_B.unr` | 549,566 | `7cee842fdcc63cf4dbeda345baea365492895b43396c7449a96a354f3f0c4a41` |
| `maps\HP3_PortraitPassword_C.unr` | 673,747 | `c1e10f7a9ed760d6a277139fe1c9d520f9203c600983210245836e4eb6002da8` |
| `maps\HP3_PortraitPassword_D.unr` | 647,120 | `8cafc6129aef6503f7931ea5e2299ff1f49c1f69607158e2a61d943c679fcd28` |
| `maps\HP3_PortraitPassword_E.unr` | 1,271,602 | `20aa465754d81f8c78414c167691033daa03b8a4fc111804c8f3ee43cc6fb6e2` |
| `maps\HP3_PortraitPassword_F.unr` | 317,688 | `073a0c042717528541c1ac5fd20e79200cf87e70551e33af5b062555cf396e3f` |
| `maps\HP3_Quidditch.unr` | 533,702 | `68711e422478a6f44f6ef042b1b3f4c4fc2924eb6e888002601e2cbe328be68b` |
| `maps\HP3_WhompingWillow.unr` | 2,797,110 | `4eb351b6e6232a65ca5be5609b03e33d9dcc06782470a8a8673607c31b894b57` |

## Engine.dll after the framerate patch

`tools/framerate_patch.py` (setup step 3) only accepts the retail `Engine.dll` above, keeps it as `Engine.dll.retail.bak`, and writes one of:

| Patch | SHA-256 |
| --- | --- |
| 60 fps cap (what `setup.bat` applies) | `e4b17d38f0860951385a0e8023477a0daa827c46648fe1d26c464ab383a1c826` |
| 60 fps cap with `--with-fps-counter` | `c1dfe8153d8ffc02e700f9d38be860b7ad62aebf94b01fa993f3f9658228e735` |

## Files setup adds

These aren't part of the retail install. `verify_retail.ps1` lists them separately:

- `system\d3d8.dll`, `system\d3d9.dll`, `system\plugins\dxgi.asi`: HP3-UHDE core files ([SETUP.md](SETUP.md) step 2).
- `system\plugins\HP3OpenCastle.asi`, and optionally `HP3MapTracker.asi` and `HP3WindowFix.asi`: this project's plugins.
- `textures\HP3AP.utx`: this project's texture package.
- `system\Engine.dll.retail.bak` and the `.bak` copies of the patched ini files.
- Logs and settings the plugins write, such as `HP3_OpenCastle.log`.
