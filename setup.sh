#!/bin/sh
# FTL DS setup: finds your own copy of FTL, converts its assets and builds ftl-ds.nds.
set -e
cd "$(dirname "$0")"
PY=$(command -v python3 || command -v python || true)
if [ -z "$PY" ]; then
  echo "Python 3 is required (https://www.python.org/downloads/)." >&2
  exit 1
fi
"$PY" -c "import PIL" 2>/dev/null || "$PY" -m pip install --user Pillow || {
  echo "Could not install Pillow automatically. Install it with your package manager (for example: sudo apt install python3-pil)." >&2
  exit 1
}
exec "$PY" tools/setup_assets.py "$@"
