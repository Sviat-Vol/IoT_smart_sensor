# IoT Smart Sensor

An ESP32-based IoT device that monitors room temperature and humidity using a
Sensirion SHT41 sensor and reports live readings through a Telegram bot.

## Overview

The firmware connects the ESP32 to Wi-Fi, reads temperature and humidity from
an SHT41 sensor over I2C, and exposes the data through a Telegram chat bot. It
automatically pushes an update whenever the readings change beyond a
configurable threshold, and it also answers a set of chat commands for
on-demand control.

## Hardware

- ESP32 development board
- Sensirion SHT41 temperature/humidity sensor (I2C address `0x44`)
- I2C wiring: SDA on GPIO 8, SCL on GPIO 9 (see `Wire.begin(8, 9)` in [main.cpp](main.cpp))

## Features

- Wi-Fi connection with status reporting over the serial console
- Periodic sensor polling with high-precision measurement
- Automatic Telegram notifications when temperature or humidity drift past a
  configurable threshold
- Telegram bot commands for remote control:
  - `/command1` — reserved (screen on/off)
  - `/command2` — stop regular updates
  - `/command3` — resume regular updates
  - `/command4` — get the current temperature and humidity on demand
  - `/command5 <value>` — set the temperature change threshold (°C)
  - `/command6 <value>` — set the humidity change threshold (%)

## Project structure

```
IoT_smart_sensor/
├── main.cpp                       # application entry point (setup/loop)
└── src/
    ├── SensirionI2cSht4x.h/.cpp   # Sensirion SHT4x sensor driver
    ├── UniversalTelegramBot.h/.cpp# Telegram Bot API client
    ├── TelegramCertificate.h      # root certificate for Telegram API TLS
    └── secrets.h                  # Wi-Fi/Telegram credentials (git-ignored, not included in repo)
```

## Dependencies

- [Arduino core for ESP32](https://github.com/espressif/arduino-esp32)
- [UniversalTelegramBot](https://github.com/witnessmenow/universal-arduino-telegram-bot) (bundled in `src/`)
- [ArduinoJson](https://arduinojson.org/) (required by `UniversalTelegramBot`)
- Sensirion `SensirionI2cSht4x` driver (bundled in `src/`)

## Configuration

Credentials are kept out of version control in `src/secrets.h` (ignored by
git). Create this file before building:

```cpp
#pragma once

#define WIFI_SSID "<your Wi-Fi SSID>"
#define WIFI_PASSWORD "<your Wi-Fi password>"

#define BOT_TOKEN "<your Telegram bot token>"
#define CHAT_ID "<your Telegram chat id>"
```

`main.cpp` includes this header to pick up the values at compile time.

## Building and flashing

This project targets the Arduino framework on ESP32 and is set up as a CLion
project. Build and upload it with your preferred Arduino/ESP32 toolchain
(Arduino IDE, PlatformIO, or `arduino-cli`), pointing it at `main.cpp` and the
`src/` directory as the sketch sources.

## Usage

1. Flash the firmware to the ESP32 and open the serial monitor (9600 baud) to
   confirm Wi-Fi and sensor initialization.
2. Open a chat with your Telegram bot.
3. Send `/command4` to get an immediate temperature/humidity reading, or wait
   for automatic updates whenever conditions change beyond the configured
   thresholds.
4. Use `/command2` / `/command3` to pause or resume automatic updates, and
   `/command5` / `/command6` to tune the change thresholds.

## License

Third-party components keep their original licenses:
- `UniversalTelegramBot` — LGPL-2.1 (Brian Lough)
- `SensirionI2cSht4x` — BSD-3-Clause (Sensirion AG)
