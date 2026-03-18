# Changelog

All notable changes to GP Tally M5StickC are documented here.

---

## [4.5.3] — 2026-03-18

### Changed
- **Socket.IO library replaced** — `timum-viw/SocketIoClient` (abandoned since 2021) replaced with a direct Socket.IO v2 implementation over `links2004/WebSockets`. Eliminates an unmaintained dependency and gives full control over the Engine.IO/Socket.IO handshake.
- **JSON library replaced** — `Arduino_JSON` replaced with `ArduinoJson` v7 (Benoît Blanchon). Faster parsing, lower RAM usage, type-safe API, actively maintained.
- **Cleaner event dispatch** — single large `socket_event()` switch replaced with `wsEvent()` (WebSocket/Engine.IO layer) and `onSocketEvent()` (Socket.IO application layer).
- **Bus lookup consolidated** — three separate `getBusTypeById()` / `getBusColorById()` / `getBusPriorityById()` functions replaced with a single `getBusInfo()` that iterates the bus list once.

### Removed
- `timum-viw/SocketIoClient` dependency
- `Arduino_JSON` dependency
- `strip_quot()` helper (no longer needed — ArduinoJson returns clean strings)
- `socket_Connected()`, `socket_Messaging()` (logic folded into `onSocketConnect()` / `onSocketEvent()`)

---

## [4.5.3] — 2026-03-18

### Added
- **Device identifier on tally screen** — last 6 chars of the listener device name (MAC suffix, e.g. `53f278`) shown in bottom-right corner in small text. Matches `DeviceName` colour during active tally, near-invisible dark grey when idle. Hidden during brightness OSD.
- **Brightness OSD preview** — first Action button click shows the brightness bar without changing the level. Subsequent clicks while the OSD is visible cycle brightness as before.
- `DEFAULT_BRIGHTNESS 51` constant — sets ~50% brightness on first boot (no saved preference). The adjustable range `START_BRIGHTNESS`–`MAX_BRIGHTNESS` is unchanged.

### Changed
- **Socket.IO library replaced** — `timum-viw/SocketIoClient` (abandoned since 2021) replaced with a direct Socket.IO v2 implementation over `links2004/WebSockets`. Endpoint: `/socket.io/?EIO=4&transport=websocket`.
- **JSON library replaced** — `Arduino_JSON` replaced with `ArduinoJson` v7 (Benoît Blanchon). Faster parsing, lower RAM usage, type-safe API, actively maintained.
- **Cleaner event dispatch** — `socket_event()` replaced with `wsEvent()` (WebSocket/Engine.IO) and `onSocketEvent()` (Socket.IO application layer).
- **Bus lookup consolidated** — `getBusTypeById()` / `getBusColorById()` / `getBusPriorityById()` replaced with single `getBusInfo()`.
- **`btnM5` single click** — changed from `isClick()` to `isSingleClick()` so double-click is always handled independently without triggering a screen toggle.
- **WiFi config preserved on portal save** — `saveParamCallback()` no longer calls `ESP.restart()` directly. Restart is deferred to `saveConfigCallback()` (after WiFi credentials are saved) or a 2 s timeout fallback via `checkPendingRestart()`.

### Removed
- `timum-viw/SocketIoClient` dependency
- `Arduino_JSON` dependency
- `strip_quot()`, `socket_Connected()`, `socket_Messaging()` (folded into new handlers)

---

## [4.5.2] — 2026-03-17

### Fixed
- **Reassign tally state** — tally state (`actualType`, `actualColor`, LEDs) is now cleared immediately in `socket_Reassign()` before the new device name is set. Previously the stale colour from the previous device persisted until the next `device_states` event arrived.
- **Race condition on reassign** — `device_states` from TA can arrive before `devices` event completes device name update. Tally state is now reset at the correct point in the reassign sequence so stale state never bleeds into the new device display.
- **`bus_options` race on connect** — TA sends `device_states` before `bus_options` on initial connect. Added `busOptionsReady` flag so `processTallyData()` is deferred until bus colour and priority data is available. Buffered `DeviceStates` are processed immediately when `bus_options` arrives.
- **Device name update** — screen repaints immediately when TallyArbiter assigns a new device name.

---

## [4.5.1] — 2026-03-17

### Added
- **Multi-file sketch** — codebase split into logical modules for maintainability: `screen.ino`, `buttons.ino`, `tally.ino`, `network.ino`, `leds.ino`, `power.ino`, `m5hal.ino`, `config.h`
- **LED toggle** — M5 button double-click enables/disables external LEDs at runtime. A small **LED OFF** indicator appears on the tally screen when LEDs are disabled
- **Settings screen on startup** — device now opens the Settings screen on boot and switches to tally automatically on first successful connection, eliminating the tally→settings flash on startup
- **Battery status auto-update** — Settings screen refreshes battery level without manual navigation
- **Three-state battery indicator** — displays `Charging...`, `Charged.`, or `nn%`
- **Exponential backoff reconnection** — reconnect delay doubles from 1 s up to 30 s
- **Adaptive Wi-Fi TX power** — reduces transmit power when signal is strong to save battery
- **Adaptive CPU frequency** — switches between 80 MHz (idle) and 160 MHz (active)
- **LED state independence** — external LEDs update regardless of current screen or animation state

### Fixed
- **WiFiManager portal save** — saving new TallyArbiter host/port via web portal now triggers `ESP.restart()` so the new address takes effect immediately
- **Slow boot after config save** — added `wm.setConnectTimeout(15)` so WiFi connect attempt takes at most 15 s before opening the AP portal
- **Battery voltage formula** — corrected for both AXP192 (StickC/Plus) and Plus2; upper limit changed to 4.2 V
- **USB detection on StickC-Plus** — switched from `GetVinVoltage()` to `GetVBusVoltage()` which correctly reflects USB connection on this variant
- **Charging detection on AXP192** — replaced unreliable `GetBatCurrent()` with `GetVBusVoltage()` + SoC threshold logic
- **Plus2 display brightness** — implemented `StickCP2.Display.setBrightness()` replacing the previously unimplemented TODO
- **Plus2 IMU API** — corrected `M5.IMU.Init()` / `getAccelData()` to `M5.Imu.begin()` / `getAccel()` for M5Unified compatibility
- **Plus2 font API** — replaced deprecated `setFreeFont()` with `setFont()` for M5GFX
- **Dead code removed** — `configureDisplayToShowDeviceInfo()` (duplicate of `configureDisplayToEvaluateMode()`) and `#include <ESPmDNS.h>` removed

### Changed
- All `#define` constants and NVS preference keys consolidated in `config.h`
- `MIN_DUTY` / `MAX_DUTY` duplicate definition resolved
- `logger()` calls for portal save callback promoted from `info-quiet` to `info` for easier diagnostics

