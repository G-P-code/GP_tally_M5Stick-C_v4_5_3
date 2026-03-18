# GP Tally M5StickC — User Manual / Lietošanas instrukcija

---

## English

### First-time setup

1. Power on the device. It will create a Wi-Fi access point named **m5StickC-plus-xxxxxx** (last 6 characters are unique to your device).
2. Connect to that network from your phone or computer (no password required).
3. Open a browser and go to **192.168.6.1**.
4. Enter your Wi-Fi network credentials and the **Tally Arbiter server IP address and port** (default port: 4455).
5. Click Save. The device will restart and connect automatically.

### Controls

| Control | Action |
|---|---|
| **M5 button** — single click | Toggle between Tally screen and Settings screen |
| **M5 button** — double click | Toggle external LEDs on / off |
| **M5 button** — hold 5 s | Reset Wi-Fi settings and restart (forces setup portal) |
| **Action button** — single click | Cycle display brightness |
| **Action button** — long press | Run LED sequential check (Program → Preview → Aux → All) |
| **Grove pin G33** — short press | Open configuration portal |
| **Grove pin G33** — hold 3 s | Erase all settings and restart |

### Tally screen

- Background colour shows the current tally state as defined in Tally Arbiter (red = Program, green = Preview, custom colours for Aux and others).
- Device name is shown in the centre.
- When external LEDs are disabled, a small **LED OFF** indicator appears just above the device name.
- The screen rotates automatically when the device is turned upside down.

### Settings screen

Shown automatically on disconnect. Displays:
- Connected Wi-Fi network name
- Device IP address
- Tally Arbiter server address and port
- Connection status (green = connected, red = not connected)
- Battery level
- Firmware version

While the Settings screen is open, the configuration web portal is also active — you can access it from any browser at the device's IP address to change the Tally Arbiter server address and port without pressing any buttons.

### External LEDs (optional)

Connect LEDs to the following GPIO pins:

| Signal | Pin |
|---|---|
| Program | G0 (G19 on Plus2) |
| Preview | G26 |
| Aux | G25 |

LED brightness follows the display brightness setting.

### Over-the-air (OTA) firmware update

The device supports OTA updates via Arduino IDE. Hostname: **m5StickC-plus-xxxxxx**. Password: **tallyarbiter**.

---

## Latviešu

### Pirmreizējā iestatīšana

1. Ieslēdz ierīci. Tā izveidos Wi-Fi piekļuves punktu ar nosaukumu **m5StickC-plus-xxxxxx** (pēdējie 6 simboli ir unikāli katrai ierīcei).
2. Pieslēdzies šim tīklam no telefona vai datora (parole nav nepieciešama).
3. Atver pārlūkprogrammu un dodies uz **192.168.6.1**.
4. Ievadi sava Wi-Fi tīkla datus un **Tally Arbiter servera IP adresi un portu** (noklusējuma ports: 4455).
5. Nospied Saglabāt. Ierīce restartēsies un savienosies automātiski.

### Vadība

| Vadība | Darbība |
|---|---|
| **M5 poga** — viens klikšķis | Pārslēgties starp Tally ekrānu un Iestatījumu ekrānu |
| **M5 poga** — dubultklikšķis | Ieslēgt / izslēgt ārējos LED |
| **M5 poga** — turēt 5 s | Atiestatīt Wi-Fi iestatījumus un restartēt (atver iestatīšanas portālu) |
| **Action poga** — viens klikšķis | Mainīt ekrāna spilgtumu |
| **Action poga** — garš spiediens | Palaist LED secīgo pārbaudi (Program → Preview → Aux → Visi) |
| **Grove kontakts G33** — īss spiediens | Atvērt konfigurācijas portālu |
| **Grove kontakts G33** — turēt 3 s | Dzēst visus iestatījumus un restartēt |

### Tally ekrāns

- Fona krāsa norāda pašreizējo tally stāvokli, kā definēts Tally Arbiter (sarkans = Program, zaļš = Preview, pielāgotas krāsas Aux un citiem).
- Ekrāna centrā redzams ierīces nosaukums.
- Kad ārējie LED ir izslēgti, tieši virs ierīces nosaukuma parādās mazs indikators **LED OFF**.
- Ekrāns automātiski rotē, kad ierīce tiek apgriezta otrādi.

### Iestatījumu ekrāns

Parādās automātiski, zaudējot savienojumu. Rāda:
- Pieslēgtā Wi-Fi tīkla nosaukumu
- Ierīces IP adresi
- Tally Arbiter servera adresi un portu
- Savienojuma statusu (zaļš = savienots, sarkans = nav savienojuma)
- Akumulatora uzlādes līmeni
- Programmatūras versiju

Kamēr Iestatījumu ekrāns ir atvērts, darbojas arī konfigurācijas web portāls — no jebkura pārlūka var doties uz ierīces IP adresi un mainīt Tally Arbiter servera adresi un portu, nespiedot nevienu pogu.

### Ārējie LED (pēc izvēles)

Pieslēdz LED pie šādiem GPIO kontaktiem:

| Signāls | Kontakts |
|---|---|
| Program | G0 (G19 uz Plus2) |
| Preview | G26 |
| Aux | G25 |

LED spilgtums seko ekrāna spilgtuma iestatījumam.

### Programmatūras atjaunināšana pa gaisu (OTA)

Ierīce atbalsta OTA atjaunināšanu caur Arduino IDE. Resursdatora nosaukums: **m5StickC-plus-xxxxxx**. Parole: **tallyarbiter**.
