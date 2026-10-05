#!/bin/sh
set -eu
cd "$(dirname -- "$0")"
cc -std=c11 -O2 -Wall -Wextra -Werror -Wno-unused-parameter -fPIC -shared \
  mixer.c -o audio-mixer.so.new $(pkg-config --cflags --libs gtk+-3.0 gtk-layer-shell-0 libpulse-mainloop-glib playerctl)

mv -f audio-mixer.so.new audio-mixer.so

# Waybar's CFFI loader needs an absolute path; keep it in an ignored local include.
python3 - <<'PYGEN'
import json
from pathlib import Path
p = Path.cwd()
(p / 'module.local.json').write_text(json.dumps({'cffi/audio-mixer': {'module_path': str(p / 'audio-mixer.so')}}, indent=2) + '\n')
PYGEN
