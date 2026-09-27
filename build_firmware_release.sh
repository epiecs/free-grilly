#! /usr/bin/env bash

# Stop at the first failing step, a failed build must never end up in dist/
set -euo pipefail

# Find platformio: on the PATH, or in the default install (bin on Linux/macOS, Scripts on Windows)
if command -v pio >/dev/null 2>&1; then
  platformio="pio"
elif command -v platformio >/dev/null 2>&1; then
  platformio="platformio"
elif [ -x ~/.platformio/penv/bin/platformio ]; then
  platformio=~/.platformio/penv/bin/platformio
elif [ -x ~/.platformio/penv/Scripts/platformio.exe ]; then
  platformio=~/.platformio/penv/Scripts/platformio.exe
else
  echo "platformio not found" >&2
  exit 1
fi

# Bump the version in settings.h
search="^    String grill_firmware_version.*"
replace="    String grill_firmware_version       = \"$(date +%y.%m.%d)\";"

# -i.bak works with both GNU sed (Linux, Git Bash) and BSD sed (macOS), plain -i "" is BSD only
sed -i.bak "s#${search}#${replace}#" src/Settings.h
rm src/Settings.h.bak

if ! grep -q "grill_firmware_version       = \"$(date +%y.%m.%d)\";" src/Settings.h; then
  echo "Version bump in src/Settings.h failed" >&2
  exit 1
fi

# Build, a failed build stops the script here (set -e) before anything is copied
"$platformio" run

# Copy ota update
cp .pio/build/esp32dev/firmware.bin dist/grilly-plus-$(date +%Y-%m-%d)-ota.bin

# Build full flash firmware including partitions, bootloader,...
esptool --chip esp32 merge-bin -o dist/grilly-plus-$(date +%Y-%m-%d)-full.bin \
  --flash-mode dio --flash-size 4MB \
  0x1000 .pio/build/esp32dev/bootloader.bin \
  0x8000 .pio/build/esp32dev/partitions.bin \
  0xe000 ~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin \
  0x10000 .pio/build/esp32dev/firmware.bin

# How to release
# - Commit all code with correct commit comments
# - run ./build_firmware_release.sh
# - update changelog.md
# - commit the updated src with version bump, changelog and new dist
# - run git log to get the latest git commit hash
# - git tag -a yy.mm.dd <git log hash>
# - git push --tags
# - prep release on github
