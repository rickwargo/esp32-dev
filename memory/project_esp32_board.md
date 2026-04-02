---
name: ESP32-S3 Super Mini custom board
description: Custom PlatformIO board definition used for all ESP32 projects in this repo
type: project
---

The ESP32-S3 Super Mini board is a custom PlatformIO board definition created by the user, referenced as `board = esp32-s3-supermini` in platformio.ini.

**Why:** The user has a specific ESP32-S3 Super Mini variant that required a custom board JSON definition rather than a generic devkit board.

**How to apply:** Always use `board = esp32-s3-supermini` (not `esp32-s3-devkitc-1` or other generics) when creating new PlatformIO projects in this repo. Build flags `-DARDUINO_USB_MODE=1` and `-DARDUINO_USB_CDC_ON_BOOT=1` are required for USB CDC serial.
