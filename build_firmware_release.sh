#! /usr/bin/env bash

# Bump the version in settings.h
search="^    String grill_firmware_version.*"
replace="    String grill_firmware_version       = \"$(date +%y.%m.%d)\";"

sed -i "" "s#${search}#${replace}#" src/Settings.h

# Build
~/.platformio/penv/bin/platformio run

# Copy ota update
cp .pio/build/esp32dev/firmware.bin dist/free-grilly-$(date +%Y-%m-%d)-ota.bin

# Build full flash firmware including partitions, bootloader,...
esptool.py --chip esp32 merge_bin -o dist/free-grilly-$(date +%Y-%m-%d)-full.bin \
  --flash_mode dio --flash_size 4MB \
  0x1000 .pio/build/esp32dev/bootloader.bin \
  0x8000 .pio/build/esp32dev/partitions.bin \
  0xe000 ~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin \
  0x10000 .pio/build/esp32dev/firmware.bin