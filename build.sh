#!/bin/bash
# Build Star Drag and stamp a pure-DS header (see qa/DS-BLANK-WHITE-FIX.md
# in folded-letter-ds — calico's ARM7 makes ndstool write a DSi-enhanced
# header, which white-screens strict emulators / DSi-mode boots).
set -e
export DEVKITPRO=/home/hatch/workspace/devkitpro-image/root/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
export PATH=$DEVKITPRO/tools/bin:$DEVKITARM/bin:$PATH
cd "$(dirname "$0")"
make clean >/dev/null 2>&1 || true
make
python3 tools/fix_ds_header.py star-drag.nds
cp star-drag.nds ~/workspace/your_files/star-drag-v19.nds
echo "--- verify ---"
ndstool -i star-drag.nds | grep -E "Unit code|Header CRC|Logo CRC|DSi|Game code|Maker code"
md5sum ~/workspace/your_files/star-drag-v19.nds
ls -lh ~/workspace/your_files/star-drag-v19.nds
