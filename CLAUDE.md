# ESP32 Development Monorepo

## Project Structure

```
SoC/
├── shared/          # Shared libraries used across projects (referenced via lib_extra_dirs)
├── <project-name>/  # Individual ESP32 PlatformIO projects
├── CLAUDE.md
└── README.md
```

## Board

Always use the custom board definition `esp32-s3-supermini` — not `esp32-s3-devkitc-1` or any generic board.

Required build flags for USB CDC serial:
```ini
build_flags =
    -DARDUINO_USB_MODE=1
    -DARDUINO_USB_CDC_ON_BOOT=1
```

## New Projects

When creating a new project, use this `platformio.ini` template:

```ini
[env:esp32s3-supermini]
platform            = espressif32
board               = esp32-s3-supermini
framework           = arduino
monitor_speed       = 115200
upload_speed        = 921600

build_flags =
    -DARDUINO_USB_MODE=1
    -DARDUINO_USB_CDC_ON_BOOT=1

lib_extra_dirs = ../shared
```

## Build & Flash

```sh
cd <project-name>
pio run              # build
pio run -t upload    # flash
pio device monitor   # serial monitor
```

## Secrets

Never commit `secrets.h` or `credentials.h`. Each project includes a `secrets.h.example` as a template.

## Framework Notes

- Uses ESP32 Arduino framework 3.x — `MDNS.update()` is no longer needed and should not be called.
