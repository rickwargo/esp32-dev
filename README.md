# ESP32 Development Monorepo

A monorepo for ESP32 application development projects using [PlatformIO](https://platformio.org/).

## Structure

```
SoC/
├── shared/          # Shared libraries and utilities used across projects
├── <project-name>/  # Individual ESP32 projects, each self-contained
└── README.md
```

## Getting Started

Each project lives in its own subdirectory and is a self-contained PlatformIO project with its own `platformio.ini`. To build a project:

```sh
cd <project-name>
pio run
```

## Shared Code

Common drivers, utilities, and helpers live in `shared/`. Projects can reference shared code via PlatformIO's `lib_extra_dirs` or symlinks in their `lib/` directory.

## Requirements

- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html)
- ESP32 toolchain (installed automatically by PlatformIO)
