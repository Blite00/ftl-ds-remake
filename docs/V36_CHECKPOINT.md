# V36 original audio checkpoint

## Completed

Imported five full original music tracks and twelve effects from the installed FTL archive, read-only. Original extracted sources and converted assets remain local. See [V36_AUDIO.md](V36_AUDIO.md) and generated/audio_manifest.json for the roster, provenance, conversions and authored cue mapping.

Music streams from embedded NitroFS; bounded resident effects share a four-voice mixer. Audio status appears on Sound. The existing synth remains a startup fallback. Save format and game mechanics are unchanged. The revised 36-second trailer is releases/trailer/ftl-ds-v35-trailer-recut.mp4; it uses V35 footage and the earlier synthesized score, not this new audio.

## Evidence

- ROM built successfully: 40,924,160 bytes. SHA-256: `b45489d82ded905f12009e41c697ab001d23592bd74e29a16238aed96c9900b9`.
- ROM inspection verified all 17 embedded audio files byte-for-byte against the manifest (40,300,756 bytes).
- Full suite: 120 host checks and 45 Python tests. Final strict and undefined-behavior-trapping audio stream/backend checks passed; the backend check is additionally registered in the runner.
- Repeat import produced identical assets; source/output hashes, alignment, sample counts and unclipped peaks checked.
- V36 host framebuffer previews generated; Sound footer inspected (host reports SYNTH FALLBACK by design).
- Static ARM9 allocated union: 909,868 bytes; high-water: 909,904. Resident effects add 660,340 dynamic bytes plus the 4,096-byte stream sample buffer and library overhead. These are not runtime free-memory measurements.
- V35 release hash matches the preserved V35 manifest. Existing user saves and installed FTL archive were not changed.

Logs: build/v36-build.log, v36-check.log, v36-audio-final-checks.log, v36-rom-audio.log, v36-memory.txt and original-audio-import-check.log.

## Limits and exact next steps

1. Launch releases/v36/ftl-ds.nds through the actual flashcart menu; confirm Sound reports ORIGINAL FTL. NitroFS support varies by loader.
2. Test all volume levels, repeated UI taps, pause, saving during playback, music transitions and simultaneous combat effects. Physical DS volume, quality, underruns, latency and frame pacing remain pending; emulator playback is also unverified.
3. Measure several minutes of combat and available runtime memory before increasing stream buffers or the effect roster.
4. Music restarts on mode changes; synchronized exploration/battle crossfades remain unfinished. Stereo assets are downmixed to mono. Cue timing, enemy AI and the interim finale remain authored DS approximations.

Recovery: releases/v35/ftl-ds.nds and checkpoints/v35-crew-timing.zip remain available. V36 source/assets/ROM and evidence are preserved in checkpoints/v36-original-audio.zip with a hashed file manifest.
