# GP Tally M5StickC

An Arduino firmware for the [M5StickC, M5StickC-Plus, and M5StickC-Plus2](https://m5stack.com) that turns the device into a wireless tally light listener client for [TallyArbiter](https://github.com/josephdadams/TallyArbiter).

Based on the original [m5stickc-listener](https://github.com/josephdadams/TallyArbiter/tree/master/listener_clients/m5stickc-listener) by Joseph Adams.

---

## Features

- Connects to a TallyArbiter server over Wi-Fi via Socket.IO
- Displays tally state (Program / Preview / Aux) as a full-screen background colour with device name
- Device MAC suffix shown in bottom-right corner of the tally screen for easy identification
- Optional external LED outputs for Program, Preview, and Aux signals with PWM brightness control
- Automatic screen rotation via IMU
- Captive portal configuration (Wi-Fi credentials + TallyArbiter server address) via WifiManager
- Settings screen with live connection status, IP address, battery level, and firmware version
- Battery status: **Charging...** / **Charged.** / **nn%** — auto-updates while Settings screen is open
- Adjustable display and LED brightness with OSD bar
- LED toggle (enable/disable external LEDs at runtime)
- LED sequential check animation for wiring verification
- Flash and reassign animations triggered by TallyArbiter server events
- Over-the-air (OTA) firmware updates
- Exponential backoff reconnection
- Adaptive Wi-Fi TX power and CPU frequency for battery efficiency

---

## Supported Hardware

| Board | Arduino IDE Board |
|---|---|
| M5StickC | `M5Stack Arduino > M5StickC` |
| M5StickC-Plus | `M5Stack Arduino > M5StickCPlus` |
| M5StickC-Plus2 | `M5Stack Arduino > M5StickCPlus2` |

Board selection is automatic when using the Arduino IDE board selector. Manual override defines are available in `GP_tally_M5Stick-C.ino` if needed.

---

## Requirements

### Libraries

Install via **Library Manager** (Sketch > Include Library > Manage Libraries):

| Library | Author | Version |
|---|---|---|
| M5StickC | M5Stack | latest |
| M5StickC-Plus | M5Stack | latest |
| M5StickC-Plus2 | M5Stack | latest |
| WebSockets | Links2004 | **2.3.4** |
| ArduinoJson | Benoît Blanchon | **7.x** |
| WiFiManager | tzapu | latest |
| MultiButton | poelstra | latest |

> **Note:** Install only the M5Stack library that matches your hardware. Having multiple M5Stack libraries installed simultaneously causes a `multiple definition of 'M5'` linker error.

> **Note:** `timum-viw/SocketIoClient` and `Arduino_JSON` are not used and can be removed if previously installed.

---

## Installation

1. Clone or download this repository.
2. Open `GP_tally_M5Stick-C/GP_tally_M5Stick-C.ino` in Arduino IDE.
3. Select the correct board under **Tools > Board**.
4. Set **Tools > Partition Scheme** to **Minimal SPIFFS (... MB App with OTA / ... SPIFFS)**. This is required to fit the firmware with OTA support.
5. Install all required libraries.
6. Flash to your device.

---

## First-time Setup

1. Power on the device. It will create a Wi-Fi access point named **m5StickC-plus-xxxxxx**.
2. Connect to that network from your phone or computer (no password required).
3. Open a browser and go to **192.168.6.1**.
4. Enter your Wi-Fi credentials and the TallyArbiter server IP and port (default: `4455`).
5. Click Save. The device restarts and connects automatically.

---

## Controls

| Control | Action |
|---|---|
| **M5 button** — single click | Toggle between Tally screen and Settings screen |
| **M5 button** — double click | Toggle external LEDs on / off |
| **M5 button** — hold 5 s | Reset Wi-Fi settings and restart |
| **Action button** — single click | Show brightness bar (first click), then cycle brightness |
| **Action button** — long press | Run LED sequential check (Program → Preview → Aux → All) |
| **Grove pin G33** — short press | Open configuration portal |
| **Grove pin G33** — hold 3 s | Erase all settings and restart |

---

## Tally Screen

The main screen shows the current tally state as a full-screen background colour matching the TallyArbiter bus colour (red for Program, green for Preview, custom for Aux). The assigned device name is displayed in the centre.

- The last 6 characters of the device identifier (MAC suffix) are shown in the bottom-right corner for easy identification when multiple devices are in use.
- When external LEDs are disabled, a **LED OFF** indicator appears just above the device name.
- The screen rotates automatically when the device is turned upside down.

---

## Settings Screen

Shown automatically on disconnect and on startup until a connection is established. Displays:

- Connected Wi-Fi network name and IP address
- TallyArbiter server address and port
- Connection status (green = connected, red = not connected)
- Battery level: **Charging...** / **Charged.** / **nn%**
- Firmware version

While the Settings screen is open, the configuration web portal is also active — navigate to the device IP address from any browser on the same network to change the TallyArbiter server address and port.

---

## External LED Wiring (optional)

| Signal | Pin | Pin (Plus2) |
|---|---|---|
| Program | G0 | G19 |
| Preview | G26 | G26 |
| Aux | G25 | G25 |

LED brightness tracks the display brightness setting. LEDs can be toggled on/off at runtime with a double click on the M5 button without affecting the tally display.

---

## OTA Firmware Updates

### Via web portal

1. Open the Settings screen on the device (M5 button single click).
2. Navigate to the device IP address shown on screen from any browser on the same network.
3. Go to **Update** in the portal menu and upload a compiled `.bin` file.

### Via Arduino IDE

Select the device under **Tools > Port > Network ports** and upload normally.

- Hostname: `m5StickC-plus-xxxxxx`
- Password: `tallyarbiter`

---

## Project Structure

```
GP_tally_M5Stick-C/
├── GP_tally_M5Stick-C.ino  — board detection, globals, setup(), loop()
├── config.h                 — all #define constants and NVS preference keys
├── screen.ino               — screen state management and painting
├── buttons.ino              — brightness OSD, LED check, reset button
├── tally.ino                — Socket.IO events and tally data processing
├── network.ino              — Wi-Fi, WifiManager, reconnection
├── leds.ino                 — external LED control and animations
├── power.ino                — CPU performance mode and battery cache
└── m5hal.ino                — M5Stack hardware abstraction layer
```

---

## License

MIT
