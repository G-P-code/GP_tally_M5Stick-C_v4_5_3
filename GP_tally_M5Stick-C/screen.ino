// ============================================================================
// screen.ino — Screen state management and painting
//
//  switchToSettings(byUser) — set state + start portal + paint
//  switchToTally()          — set state + stop portal  + paint
//  paintSettingsScreen()    — pure paint, no state changes
//  paintTallyScreen()       — pure paint via evaluateMode()
//  repaintCurrentScreen()   — paint current screen, no state changes
//  evaluateMode()           — update LEDs + repaint tally display
//
// Rule: anything that only needs to refresh pixels calls repaintCurrentScreen().
//       Only intentional user/socket transitions call switchTo*().
// ============================================================================

void paintSettingsScreen() {
  m5_fillScreen(TFT_BLACK);
  m5_configSettingsScreen();
  m5_setTextColor(WHITE, BLACK);
  m5_println("SSID: " + String(WiFi.SSID()));
  m5_println(WiFi.localIP().toString());
  m5_println("Tally Arbiter Server:");
  m5_println(String(tallyarbiter_host) + ":" + String(tallyarbiter_port));

  if (taConnected) {
    m5_setTextColor(GREENYELLOW);
    m5_println("Connected.");
    m5_setTextColor(WHITE, BLACK);
  } else {
    m5_setTextColor(RED_COLOR);
    m5_println("Not connected!");
    m5_setTextColor(WHITE, BLACK);
  }

  m5_print("Battery: ");
#if defined(STICK_C_PLUS2)
  String batStr = batteryCache.isCharging ? "Charging..." : (batteryCache.isCharged ? "Charged." : (String(batteryCache.level) + "%"));
  StickCP2.Display.print(batStr);
  String verStr = "v" + version + "   ";
  int verX = StickCP2.Display.width() - StickCP2.Display.textWidth(verStr);
  int verY = StickCP2.Display.getCursorY();
  StickCP2.Display.setTextColor(DARKGREY);
  StickCP2.Display.setCursor(verX, verY);
  StickCP2.Display.println(verStr);
  StickCP2.Display.setTextColor(WHITE, BLACK);
#else
  String batStr = batteryCache.isCharging ? "Charging..." : (batteryCache.isCharged ? "Charged." : (String(batteryCache.level) + "%"));
  M5.Lcd.print(batStr);
  String verStr = "v" + version + "   ";
  int verX = M5.Lcd.width() - M5.Lcd.textWidth(verStr);
  int verY = M5.Lcd.getCursorY();
  M5.Lcd.setTextColor(DARKGREY, BLACK);
  M5.Lcd.setCursor(verX, verY);
  M5.Lcd.println(verStr);
  M5.Lcd.setTextColor(WHITE, BLACK);
#endif
}

void paintTallyScreen() {
  prevType = "__invalidated__";
  evaluateMode();
}

void repaintCurrentScreen() {
  if      (currentScreen == 0) paintTallyScreen();
  else if (currentScreen == 1) paintSettingsScreen();
}

void switchToSettings(bool byUser) {
  currentScreen = 1;
  if (byUser) userRequestedSettings = true;
  logger("switchToSettings(byUser=" + String(byUser) + ")", "info-quiet");
  if (!portalRunning) {
    wm.startWebPortal();
    portalRunning = true;
  }
  paintSettingsScreen();
}

void switchToTally() {
  currentScreen = 0;
  // Do NOT clear userRequestedSettings here — it must only be cleared when
  // the user explicitly presses btnM5 to leave settings.
  logger("switchToTally()", "info-quiet");
  if (portalRunning) {
    wm.stopWebPortal();
    portalRunning = false;
  }
  paintTallyScreen();
}

// Legacy wrappers used by animation restore points.
// These must NOT change userRequestedSettings.
void showSettings()   { switchToSettings(false); }
void showDeviceInfo() { switchToTally(); }

void evaluateMode() {
  // --- LED update: always runs, never blocked by animations or screen state ---
#if TALLY_EXTRA_OUTPUT
  if (!ledCheckActive && actualType != prevLedType) {
    externalLed();
    prevLedType = actualType;
  }
#endif

  // --- Display update: blocked during animations, skipped on settings screen ---
  if (animationActive()) return;
  if (currentScreen != 0) return;

  if (actualType != prevType) {
    String colorStr = actualColor;
    colorStr.replace("\"", "");
    colorStr.replace("#", "");

    int r = 0, g = 0, b = 0;
    bool validColor = (colorStr.length() == 6);
    if (validColor) {
      long number = strtol(colorStr.c_str(), NULL, 16);
      r = number >> 16;
      g = (number >> 8) & 0xFF;
      b = number & 0xFF;
    }

    if (actualType != "" && validColor) {
      uint16_t bgColor = m5_color565(r, g, b);
      m5_fillScreen(bgColor);
      configureDisplayToEvaluateMode();
      m5_setTextColor(TFT_BLACK, bgColor);
      m5_println(DeviceName);
    } else {
      m5_fillScreen(TFT_BLACK);
      configureDisplayToEvaluateMode();
      m5_setTextColor(DARKGREY, TFT_BLACK);
      m5_println(DeviceName);
    }

    // LED muted indicator — small notice at bottom when LEDs are disabled
#if TALLY_EXTRA_OUTPUT
    if (!ledsEnabled) {
      int indW = m5_displayWidth();
      int indH = 14;
      int indY = m5_displayHeight() - indH - 2;
      uint16_t bgc = (actualType != "" && validColor) ? m5_color565(r, g, b) : (uint16_t)TFT_BLACK;
      m5_fillRect(0, indY, indW, indH, bgc);
      configureDisplayToCheckReset();
      m5_setTextColor(DARKGREY, bgc);
      m5_println("  LED OFF");
    }
#endif

    // DeviceId indicator — small grey text at bottom-right corner
    // Same positioning logic as version string in settings screen.
    // Not shown during brightness OSD (brightnessOsdActive check omitted here
    // because evaluateMode() is only called after prevType changes, not during OSD).
    if (!brightnessOsdActive) {
      configureDisplayToCheckReset();
      // Active tally: black text on tally colour (same as DeviceName).
      // No signal: very dark grey on black — present but unobtrusive.
      // suffix is always exactly 6 hex chars after setup() padding fix
      String idStr = listenerDeviceName.substring(listenerDeviceName.length() - 6) + "   ";
      uint16_t idFg, idBg;
      if (actualType != "" && validColor) {
        idFg = TFT_BLACK;
        idBg = m5_color565(r, g, b);
      } else {
        idFg = 0x2104;   // very dark grey
        idBg = TFT_BLACK;
      }
#if defined(STICK_C_PLUS2)
      int idW = StickCP2.Display.textWidth(idStr);
      int idX = StickCP2.Display.width() - idW;
      int idY = StickCP2.Display.height() - 10;
      StickCP2.Display.setTextColor(idFg, idBg);
      StickCP2.Display.setCursor(idX, idY);
      StickCP2.Display.print(idStr);
#else
      int idW = M5.Lcd.textWidth(idStr);
      int idX = M5.Lcd.width() - idW;
      int idY = M5.Lcd.height() - 10;
      M5.Lcd.setTextColor(idFg, idBg);
      M5.Lcd.setCursor(idX, idY);
      M5.Lcd.print(idStr);
#endif
    }

    logger("Device is in " + actualType + " (color " + colorStr + " priority " + String(actualPriority) + ")", "info");
    prevType = actualType;
  }

  if (LAST_MSG && currentScreen == 0) {
    m5_println(LastMessage);
  }
}
