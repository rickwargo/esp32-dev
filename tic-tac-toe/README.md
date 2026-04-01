# Tic Tac Toe — ESP32-S3 Super Mini

A tic-tac-toe web app served directly from the ESP32-S3 Super Mini.
Connect any browser on the same network to play — no app or internet required.

## Features

- Single-page app served over WiFi
- **vs AI** mode using minimax (unbeatable)
- **2-player** mode on the same device or across two browsers on the LAN
- Accessible at `http://tictactoe.local` (mDNS) or by IP

## Setup

### 1. WiFi credentials

```sh
cp include/secrets.h.example include/secrets.h
```

Edit `include/secrets.h` and fill in your SSID and password.

### 2. Flash

```sh
cd tic-tac-toe
pio run --target upload
pio device monitor
```

The serial monitor will print the board's IP address once it connects.

### 3. Play

Open `http://tictactoe.local` (or the printed IP) in any browser.

## Hardware

| Board | ESP32-S3 Super Mini |
|-------|-------------------|
| Framework | Arduino (PlatformIO) |
| Serial | USB CDC (native USB) |

## API

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/` | HTML game page |
| GET | `/state` | Current game state (JSON) |
| POST | `/move?cell=0-8` | Place a piece on cell 0–8 |
| POST | `/reset?mode=PvP\|AI` | Start a new game |
| GET | `/ip` | Returns board IP as plain text |
