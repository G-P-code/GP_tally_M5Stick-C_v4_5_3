# Requirements

## Arduino IDE

Version 2.x recommended.

**Tools > Board:** M5Stack Arduino > M5StickC / M5StickCPlus / M5StickCPlus2  
**Tools > Partition Scheme:** Minimal SPIFFS (... MB App with OTA / ... SPIFFS)

---

## Board Support Packages

Install via **Boards Manager** (Tools > Board > Boards Manager):

| Package | Version |
|---|---|
| esp32 by Espressif | 3.x |
| M5Stack | latest |

---

## Libraries

Install via **Library Manager** (Sketch > Include Library > Manage Libraries):

| Library | Author | Version | Notes |
|---|---|---|---|
| M5StickC | M5Stack | latest | For M5StickC only |
| M5StickC-Plus | M5Stack | latest | For M5StickC-Plus only |
| M5StickC-Plus2 | M5Stack | latest | For M5StickC-Plus2 only |
| WebSockets | Links2004 | **2.3.4** | Exact version required |
| ArduinoJson | Benoît Blanchon | **7.x** | v6 not compatible |
| WiFiManager | tzapu | latest | |
| MultiButton | poelstra | latest | |

> **Note:** Only install the M5Stack library matching your hardware. Having multiple M5Stack libraries installed simultaneously causes a `multiple definition of 'M5'` linker error.

> **Note:** `timum-viw/SocketIoClient` and `Arduino_JSON` are not used as of v4.5.3 and can be uninstalled from the Arduino Library Manager.

---

## OTA Updates

Firmware can be updated over the air without Arduino IDE via the built-in web portal:

1. Open the Settings screen on the device (M5 button single click)
2. Navigate to the device IP address shown on screen from any browser on the same network
3. Go to **Update** in the portal menu and upload a compiled `.bin` file

OTA is also supported directly from Arduino IDE (Tools > Port > Network ports):
- Hostname: `m5StickC-plus-xxxxxx`
- Password: `tallyarbiter`
