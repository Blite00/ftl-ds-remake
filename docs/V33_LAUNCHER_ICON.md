# V33 launcher icon update

The ROM now uses a static 32 x 32, 16-color Kestrel launcher icon with small FTL lettering. The ship is resized from locally imported original artwork; the border, background and lettering are authored for DS. Extracted artwork stays local. Launcher title: FTL DS Campaign / Base game demake / Personal adaptation.

Makefile selects generated/launcher_icon.gif and makes the ROM depend on the icon and title settings. Regenerate the icon with `python tools/make_launcher_icon.py` (Pillow), then run `./build.ps1`.

## Validation

Build succeeds (build/v33-icon-build.log). All 1,024 embedded icon pixels match the source GIF after RGB555 conversion; banner CRC and all six language title entries validate. The decoded ROM icon was visually inspected (build/launcher-icon-rom.png). ARM7 and ARM9 executable payloads are byte-identical to preserved V33. No gameplay or save-format changes. Physical flashcart menu display remains pending.

Current ROM: ftl-ds.nds. Preserved copy: releases/v33-icon/ftl-ds.nds. Size: 608,256 bytes. SHA-256: ad182bc918426100f8b3535481ae9e75091eefcfa5ee5c9d3f0b5e57e8e77bcc. Evidence: build/v33-icon-evidence.json. Original releases/v33/ftl-ds.nds remains hash-verified and untouched. Existing user saves and installed FTL files were not modified.

Recovery: checkpoints/v33-launcher-update.zip is an overlay on checkpoints/v33-menu-ships-input.zip, containing the changed build configuration, generator, icon, documentation, evidence and updated ROM. Each included file is hashed in build/v33-icon-manifest.json; the ZIP has a SHA-256 sidecar.

## Next steps

Copy the updated ROM to the flashcart and check its game-selection icon/title. Continue V33 physical input/menu/save validation and the full-game roadmap; this update completes launcher packaging only.
