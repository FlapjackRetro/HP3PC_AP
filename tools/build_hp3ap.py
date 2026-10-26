#!/usr/bin/env python3
"""Build HP3AP.utx, the randomizer's texture package, from its sources.

Usage: python build_hp3ap.py <Archipelago>/data/icon.png <HP3 game folder> OUT/HP3AP.utx

Contents (object path HP3AP.<Name>):
  APLogoProgression / APLogoUseful / APLogoFiller   Archipelago logo tiers (gen_ap_logos.py, MIT logo)
  APKeyDrawn                                        door / area unlock key: our own drawing (gen_key.py drawn), default
  APKeyUpscaled                                     HP3's plain gold key upscaled 4x (gen_key.py upscale)
The third key style is the game's own HP_FX.Particles.Key2, used as is. The key style is a player option:
key_style: drawn (default) | upscaled | original.

APKeyUpscaled is made from the player's own HP_FX.utx, so build this on the player's machine (the installer)
rather than shipping the result.
"""
import os, sys, tempfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_ap_logos, gen_key, utxwriter

KEY_STYLES = {'drawn': 'HP3AP.APKeyDrawn', 'upscaled': 'HP3AP.APKeyUpscaled', 'original': 'HP_FX.Particles.Key2'}


def main():
    if len(sys.argv) < 4: sys.exit(__doc__)
    icon, game, out = sys.argv[1:4]
    tmp = tempfile.mkdtemp(prefix='hp3ap_')
    sys.argv = ['gen_ap_logos.py', icon, tmp, '128']; gen_ap_logos.main()
    gen_key.drawn().save(os.path.join(tmp, 'APKeyDrawn.png'))
    items = [(n, os.path.join(tmp, n + '.png')) for n in ('APLogoProgression', 'APLogoUseful', 'APLogoFiller', 'APKeyDrawn')]
    hp_fx = os.path.join(game, 'textures', 'HP_FX.utx')
    if os.path.exists(hp_fx):
        gen_key.upscale(hp_fx).save(os.path.join(tmp, 'APKeyUpscaled.png'))
        items.append(('APKeyUpscaled', os.path.join(tmp, 'APKeyUpscaled.png')))
    else:
        print('no HP_FX.utx under', game, '- APKeyUpscaled left out')
    for p in utxwriter.build(out, items): print(p)


if __name__ == '__main__':
    main()
