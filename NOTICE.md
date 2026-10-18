# Notice: FTL content and this repository

**FTL DS is an unofficial fan project.** It is not affiliated with, endorsed by,
sponsored by or supported by Subset Games. *FTL: Faster Than Light*, its names,
logos, art, music, sound, text and data are the property of Subset Games.

## What is in this repository

Only original work: the Nintendo DS engine (`source/`), the conversion tools
(`tools/`), tests (`tests/`) and documentation (`docs/`). It contains no files
from FTL and no content converted from FTL. Where the code refers to game
content, it does so by internal identifier (such as a blueprint or event name),
so the content itself is read from the player's own copy at setup time.

`tools/audit_clean.py` enforces this: it rejects images, audio, ROMs and
converted files, and, given an `ftl.dat`, rejects any file that matches an
archive file or contains a sentence of the game's text. It runs on every commit
(when the hook is installed) and in CI.

## What the setup creates on your computer

`tools/setup_assets.py` reads `ftl.dat` from a copy of FTL that you obtained
legitimately, converts the parts the DS needs, and builds `ftl-ds.nds`. The
results (`generated/`, `nitrofiles/`, `.ftl-install/` and the `.nds` ROM)
contain Subset Games' content. They are for your own personal use with your own
copy of the game. **Do not upload, share, sell or distribute them**, including
in issues, pull requests, releases, forks or download sites.

The GitHub releases of this project, if any, contain source code only.

## Requests from the rights holder

If Subset Games asks for any change to, or the removal of, this project, the
maintainers will comply. Contact: open an issue, or reach the maintainer
listed on the repository page.

This notice is not legal advice.
