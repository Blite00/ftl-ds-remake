#!/usr/bin/env python3
"""Import a bounded original FTL audio roster for local DS builds.

Source archive is read-only. Full source tails are retained. PCM is mono signed
16-bit little-endian: music 22050 Hz, effects 16000 Hz. No lossy recompression,
dynamic-range compression or gain boost. Keep outputs local.
V68 alarm/damage cues (the last EFFECTS entries) carry explicit options so the
resident effect budget holds: a lower rate, a trim with a short linear fade, or one
seamless loop cycle (the cycle's tail crossfades into its head). The rate is
emitted per asset in audio_assets.h and the manifest.
"""
import argparse
from array import array
import hashlib
import json
import math
from pathlib import Path
import shutil
import subprocess
import sys
from import_ftl import PkgArchive

TRACKS = [
 ('MENU', 'audio/music/bp_MUS_TitleScreen.ogg'),
 ('MAP', 'audio/music/bp_MUS_CivilEXPLORE.ogg'),
 ('COMBAT', 'audio/music/bp_MUS_CivilBATTLE.ogg'),
 ('FINALE', 'audio/music/bp_MUS_LastStand.ogg'),
 ('VICTORY', 'audio/waves/ui/bp_MUS_Victory.ogg'),
]
EFFECTS = [
 ('LASER', 'audio/waves/weapons/bp_laser_3.ogg'),
 ('MISSILE', 'audio/waves/weapons/bp_missile_small.ogg'),
 ('ION', 'audio/waves/weapons/ions/bp_SFX_IonWeapon1.ogg'),
 ('BEAM', 'audio/waves/weapons/bp_beam_1.ogg'),
 ('BOMB', 'audio/waves/weapons/bp_SFX_BombTeleport.ogg'),
 ('HULL', 'audio/waves/explosions/bp_hit_hull_1.ogg'),
 ('SHIELD', 'audio/waves/explosions/bp_hit_shield_1.ogg'),
 ('MISS', 'audio/waves/weapons/miss3.ogg'),
 ('EXPLOSION', 'audio/waves/explosions/bp_explosion_large_1.ogg'),
 ('JUMP', 'audio/waves/ship/bp_jump_1.ogg'),
 ('SELECT', 'audio/waves/ui/select_light1.wav'),
 ('FAIL', 'audio/waves/ui/select_b_fail1.wav'),
 # V68 (data/sounds.xml names): sysExplosion, death1, fire loop, airLeak loop, warning.
 ('SYSTEM', 'audio/waves/explosions/bp_explosion_small_1.ogg', dict(rate=11025, length=1.0, fade=0.15)),
 ('DEATH', 'audio/waves/ship/bp_crew_death_1.ogg'),
 ('FIRE', 'audio/waves/ship/bp_fire_1.ogg', dict(rate=11025, start=0.5, loop=1.0, crossfade=0.25)),
 ('BREACH', 'audio/waves/ship/bp_air_escaping_1.ogg', dict(rate=11025, start=0.5, loop=1.0, crossfade=0.25)),
 ('WARNING', 'audio/waves/ui/warning3.ogg', dict(rate=11025, length=1.3, fade=0.1)),
]

def sha(data):
    return hashlib.sha256(data).hexdigest()


def shape(samples, rate, start=0.0, length=None, fade=0.0, loop=None, crossfade=0.0):
    """Trim/fade or cut one loop cycle; returns a new float array."""
    first = round(start * rate)
    if loop is not None:
        n, x = round(loop * rate) // 2 * 2, round(crossfade * rate)  # even: no pad sample at the seam
        if first + n + x > len(samples) or x <= 0 or x >= n:
            raise ValueError('Loop cycle exceeds source')
        head = samples[first:first + x]
        out = array('f', samples[first + x:first + n + x])
        for j in range(x):
            w = j / x
            out[n - x + j] = out[n - x + j] * (1 - w) + head[j] * w
        return out
    out = array('f', samples[first:first + round(length * rate)] if length else samples[first:])
    f = min(len(out), round(fade * rate))
    for j in range(f):
        out[len(out) - f + j] *= (f - j - 1) / f
    return out


def convert(ffmpeg, source, rate, opts=None):
    # Float output permits peak inspection after downmix/resampling, before any
    # integer saturation. Single-threaded decode and pinned tool version are
    # recorded for reproducibility; the exact binary may differ across versions.
    result = subprocess.run([ffmpeg, '-hide_banner', '-loglevel', 'error',
        '-threads', '1', '-i', str(source), '-vn', '-ac', '1', '-ar', str(rate),
        '-f', 'f32le', '-c:a', 'pcm_f32le', 'pipe:1'], check=True, capture_output=True)
    samples = array('f')
    samples.frombytes(result.stdout)
    if sys.byteorder != 'little':
        samples.byteswap()
    if not samples or any(not math.isfinite(x) for x in samples):
        raise ValueError('Empty or nonfinite decoded audio: ' + str(source))
    if opts:
        samples = shape(samples, rate, **{k: v for k, v in opts.items() if k != 'rate'})
    peak = max(abs(x) for x in samples)
    ceiling = 10 ** (-3 / 20)
    gain = min(1.0, ceiling / peak) if peak else 1.0
    pcm = array('h', (round(x * gain * 32767) for x in samples))
    # DS hardware sample lengths are words; retain the full tail then zero-pad.
    padding = len(pcm) % 2
    if padding:
        pcm.append(0)
    pcm_peak = max(abs(x) for x in pcm)
    if pcm_peak >= 32767:
        raise ValueError('Unexpected clipped sample')
    if sys.byteorder != 'little':
        pcm.byteswap()
    return pcm.tobytes(), dict(decoded_samples=len(samples), padding_samples=padding,
        duration_seconds=len(samples)/rate, decoded_peak=peak, gain=gain,
        gain_db=20*math.log10(gain), pcm_peak=pcm_peak, clipping_samples=0)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('archive', type=Path)
    ap.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    ap.add_argument('--ffmpeg', default=shutil.which('ffmpeg'))
    args = ap.parse_args()
    if not args.ffmpeg:
        ap.error('ffmpeg required; pass --ffmpeg')
    root, archive_path = args.root.resolve(), args.archive.resolve()
    if archive_path.is_relative_to(root / 'generated') or archive_path.is_relative_to(root / 'nitrofiles'):
        raise ValueError('Archive must be outside generated output trees')
    archive = PkgArchive(archive_path)
    version = subprocess.run([args.ffmpeg, '-version'], check=True, capture_output=True,
                             text=True).stdout.splitlines()[0]
    outputs = []
    for kind, roster, base_rate in [('track', TRACKS, 22050), ('sfx', EFFECTS, 16000)]:
        for entry in roster:
            cue, path, opts = entry[0], entry[1], dict(entry[2]) if len(entry) > 2 else {}
            rate = opts.get('rate', base_rate)
            data = archive.read(path)
            original = root / 'generated' / 'original' / path
            original.parent.mkdir(parents=True, exist_ok=True)
            original.write_bytes(data)
            if original.read_bytes() != data:
                raise IOError('Extracted asset integrity mismatch')
            converted, info = convert(args.ffmpeg, original, rate, opts)
            output = root / 'nitrofiles' / 'audio' / (kind + '_' + cue.lower() + '.pcm')
            output.parent.mkdir(parents=True, exist_ok=True)
            output.write_bytes(converted)
            outputs.append(dict(kind=kind, cue=cue, source=path,
                source_bytes=len(data), source_sha256=sha(data),
                output=output.relative_to(root).as_posix(),
                nitro_path='nitro:/audio/' + output.name,
                bytes=len(converted), sha256=sha(converted), rate=rate, shaping=opts, **info))
    header = ['/* Generated by tools/import_audio.py. Original assets remain local. */',
              '#ifndef FTL_DS_AUDIO_ASSETS_H', '#define FTL_DS_AUDIO_ASSETS_H',
              'typedef struct { const char *path; unsigned bytes, rate; } AudioAsset;']
    for kind, roster, prefix, name in [('track', TRACKS, 'AUDIO_TRACK_', 'audio_tracks'),
                                      ('sfx', EFFECTS, 'AUDIO_SFX_', 'audio_sfx')]:
        header.append('enum { ' + ', '.join(prefix + entry[0] for entry in roster) + ', ' + prefix + 'COUNT };')
        header.append('static const AudioAsset ' + name + '[' + prefix + 'COUNT] = {')
        for item in outputs:
            if item['kind'] == kind:
                header.append('    {"%s", %du, %du},' % (item['nitro_path'], item['bytes'], item['rate']))
        header.append('};')
    header.append('#endif')
    (root/'generated'/'audio_assets.h').write_text('\n'.join(header)+'\n', encoding='utf-8')
    totals = {kind+'_bytes': sum(x['bytes'] for x in outputs if x['kind']==kind)
              for kind in ('track','sfx')}
    totals['rom_asset_bytes'] = sum(x['bytes'] for x in outputs)
    if totals['sfx_bytes'] > 768 * 1024:
        raise ValueError('Resident effects exceed AUDIO_SFX_BUDGET')
    manifest = dict(schema=1, source_archive=str(archive_path),
        source_archive_bytes=archive.size, source_archive_sha256=sha(archive_path.read_bytes()),
        ffmpeg=version, format='signed PCM16 little-endian mono',
        policy='Full tracks/tails except listed V68 shaping (trim+fade or one crossfaded loop cycle, per-asset rate); downmix, resample, attenuation only to at most -3 dBFS peak; no gain boost; zero pad to 4 bytes. Music streamed; effects resident within 768 KiB. Local original assets only.',
        totals=totals, assets=outputs)
    (root/'generated'/'audio_manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(totals))
    for item in outputs:
        print('%s: %.3fs, %d bytes, %.2fdB gain' % (item['cue'],item['duration_seconds'],item['bytes'],item['gain_db']))

if __name__ == '__main__':
    main()
