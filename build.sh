#!/bin/bash
# Build script for Cardputer Pocket Bible v1.0 - FINAL / M5Burner base
# Produces PocketBible.bin (huge_app)
#
# ALWAYS use huge_app partition - this is REQUIRED to avoid the black screen
# "need 0x100000 (1MB) use cardbi pa did not load" error on M5Launcher.
#
# CRITICAL FLASH STEPS (do every time for clean result):
# 1. In M5Launcher (or web flasher), DELETE any old "CardBible", "cardbible",
#    "Pocket Bible" or similar entry COMPLETELY.
# 2. Reboot launcher.
# 3. Flash ONLY PocketBible.bin
# 4. If launcher asks for partition/app size for this app, choose "huge_app",
#    "Doom", or any large 3MB+ scheme.
# 5. Power cycle the Cardputer.
#
# v1.0 base: 4-item main menu on HOME for maximum ease (1 Read Bible w/ OT/NT, 2 VOTD, 3 Extras, 4 About direct).
# Extras reduced to clean 3 items (Prayer, Hymns lyrics, Topical Gideon).
# No keyboard help screen (limited screen real estate).
# Reader red letters, full nav + filter. Core fully self-contained.
# Optional SD only for complete 66 books + L/R ch.
# No audio, no bookmarks. Perfect simple base for M5Burner.

BOARD="m5stack:esp32:m5stack_cardputer"
OUTDIR="build"
SRC_DIR="$(cd "$(dirname "$0")" && pwd)"
SRC_INO="$SRC_DIR/CardBible.ino"

# Clean previous builds to avoid accumulation of old versions
rm -rf "$OUTDIR"/*
mkdir -p "$OUTDIR"

TMP_SKETCH="/tmp/CardBible"
rm -rf "$TMP_SKETCH"
mkdir -p "$TMP_SKETCH"
cp "$SRC_INO" "$TMP_SKETCH/CardBible.ino"

echo "Compiling Pocket Bible v1.0 (huge_app) - 4-item easy home menu (Read, VOTD, Extras, About), clean 3-item Extras, self-contained core"
echo "Using temp sketch: $TMP_SKETCH/CardBible.ino (folder name must match ino basename for arduino-cli)"

~/bin/arduino-cli compile \
  --fqbn "$BOARD" \
  --output-dir "$OUTDIR" \
  --build-property "build.partitions=huge_app" \
  "$TMP_SKETCH/CardBible.ino"

STATUS=$?

rm -rf "$TMP_SKETCH" 2>/dev/null || true

if [ $STATUS -eq 0 ]; then
  echo "SUCCESS! Fresh build complete."
  ls -lh "$OUTDIR"/*.merged.bin 2>/dev/null

  # === FINAL DISTRIBUTION ===
  # For the final product we produce exactly:
  #   PocketBible.bin
  # This is the only file needed for distribution / flashing.
  cp "$OUTDIR/CardBible.ino.merged.bin" ./PocketBible.bin
  cp ./PocketBible.bin ~/Downloads/PocketBible.bin 2>/dev/null || true

  echo ""
  echo "=== v1.0 BUILD COMPLETE (M5Burner ready) ==="
  echo "  PocketBible.bin  (the file to submit / flash)"
  echo "  /home/Commodore/Downloads/PocketBible.bin"
  echo "  ./PocketBible.bin in this folder"
  echo ""
  echo "FLASH INSTRUCTIONS (critical for final):"
  echo " 1. In M5Launcher, DELETE every old CardBible / Pocket Bible entry (long press → remove)."
  echo " 2. Reboot the launcher or power cycle the Cardputer."
  echo " 3. Flash ONLY PocketBible.bin"
  echo " 4. If it asks for partition, choose huge_app or large."
  echo " 5. Power cycle and load the new entry."
  echo ""
  echo "SD card: copy sd-card-bible/bible/ to the root of the SD as /bible/"
else
  echo "Compile failed (status $STATUS)."
  echo " - Make sure m5stack:esp32 core is installed in arduino-cli."
  echo " - Easiest: open CardBible.ino in Arduino IDE and export the merged binary."
  echo " - Then copy it to ~/Downloads/PocketBible-current-merged.bin"
fi
