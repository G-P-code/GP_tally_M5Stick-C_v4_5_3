// ============================================================================
// buttons.ino — Button-driven actions and their tick functions
//
//   updateBrightness()    — cycle brightness, show OSD
//   showBrightnessOsd()   — draw brightness bar overlay
//   brightnessTick()      — dismiss OSD after timeout
//   ledSequentialCheck()  — start LED sequential check animation
//   ledCheckTick()        — advance LED check state machine
//   resetTick()           — handle TRIGGER_PIN hold-to-reset
// ============================================================================

// ----------------------------------------------------------------------------
// Brightness
// ----------------------------------------------------------------------------

void updateBrightness() {
  if (!brightnessOsdActive) {
    // First click — just show the bar without changing brightness.
    // Subsequent clicks while OSD is visible will change brightness.
    brightnessOsdOriginScreen = currentScreen;
    showBrightnessOsd();
    brightnessOsdActive = true;
    brightnessOsdLastMs = millis();
    return;
  }

  // OSD already visible — cycle brightness on this click
  if (currentBrightness >= MAX_BRIGHTNESS) {
    currentBrightness = START_BRIGHTNESS;
  } else {
    currentBrightness += 10;
  }

  preferences.begin(PREF_NAMESPACE, false);
  preferences.putInt(PREF_BRIGHTNESS, currentBrightness);
  preferences.end();

  logger("Set brightness: " + String(currentBrightness), "info-quiet");

#if TALLY_EXTRA_OUTPUT
  set_dutyCycle();
  logger("dutyCycle: " + String(dutyCycle), "info-quiet");
  externalLed();
#endif

#if defined(STICK_C_PLUS2)
  StickCP2.Display.setBrightness(lcdBrightnessPlus2());
#else
  M5.Axp.ScreenBreath(lcdBrightness());
#endif

  // Refresh OSD and reset dismiss timer
  showBrightnessOsd();
  brightnessOsdLastMs = millis();
}

// Draw a brightness bar at the bottom of the screen without clearing it.
void showBrightnessOsd() {
  const int barH = 18;
  const int barW = m5_displayWidth() - 8;
  const int barX = (m5_displayWidth() - barW) / 2;
  const int barY = m5_displayHeight() - barH - 4;

  m5_fillRect(0, barY - 4, m5_displayWidth(), barH + 8, TFT_BLACK);
  m5_drawRect(barX, barY, barW, barH, DARKGREY);

  int filled = (int)((float)(currentBrightness - START_BRIGHTNESS) /
                     (float)(MAX_BRIGHTNESS    - START_BRIGHTNESS) * barW);
  filled = constrain(filled, 1, barW);
  m5_fillRect(barX, barY, filled, barH, TFT_DARKGREY);
}

// Dismiss brightness OSD after BRIGHTNESS_OSD_MS and restore the previous screen.
void brightnessTick() {
  if (!brightnessOsdActive) return;
  if (millis() - brightnessOsdLastMs < BRIGHTNESS_OSD_MS) return;

  brightnessOsdActive = false;
  prevType = "__invalidated__";
  switch (brightnessOsdOriginScreen) {
    case 0: showDeviceInfo(); break;
    case 1: showSettings();   break;
  }
}

// ----------------------------------------------------------------------------
// LED sequential check
// ----------------------------------------------------------------------------

void ledSequentialCheck() {
#if TALLY_EXTRA_OUTPUT
  logger("ledSequentialCheck() start", "info");
  ledCheckActive       = true;
  ledCheckStep         = 0;
  ledCheckLastMs       = 0;
  ledCheckOriginScreen = currentScreen;
#else
  logger("TALLY_EXTRA_OUTPUT disabled, no LEDs to check", "info");
#endif
}

#if TALLY_EXTRA_OUTPUT
void ledCheckTick() {
  if (!ledCheckActive) return;

  unsigned long now = millis();
  if (now - ledCheckLastMs < LED_CHECK_INTERVAL) return;
  ledCheckLastMs = now;

  switch (ledCheckStep) {
    case 0:
      ledcWrite(led_program, dutyCycle);
      ledcWrite(led_preview, 0);
      ledcWrite(led_aux, 0);
#if defined(STICK_C_PLUS2)
      ledCheckScreen(TFT_RED,      TFT_WHITE,  "PROGRAM", "G19",        1, 4);
#else
      ledCheckScreen(TFT_RED,      TFT_WHITE,  "PROGRAM", "G0",         1, 4);
#endif
      break;

    case 1:
      ledcWrite(led_program, 0);
      ledcWrite(led_preview, dutyCycle);
      ledcWrite(led_aux, 0);
      ledCheckScreen(TFT_DARKGREEN, TFT_WHITE,  "PREVIEW", "G26",       2, 4);
      break;

    case 2:
      ledcWrite(led_program, 0);
      ledcWrite(led_preview, 0);
      ledcWrite(led_aux, dutyCycle / 4);
      ledCheckScreen(TFT_NAVY,      TFT_YELLOW, "AUX",    "G25",        3, 4);
      break;

    case 3:
      ledcWrite(led_program, dutyCycle);
      ledcWrite(led_preview, dutyCycle);
      ledcWrite(led_aux, dutyCycle / 4);
      ledCheckScreen(TFT_DARKGREY,  TFT_WHITE,  "ALL ON", "G0+G26+G25", 4, 4);
      break;

    default:
      ledcWrite(led_program, 0);
      ledcWrite(led_preview, 0);
      ledcWrite(led_aux, 0);
      ledCheckActive = false;
      logger("ledSequentialCheck() done", "info");
      prevType = "__invalidated__";
      switch (ledCheckOriginScreen) {
        case 0: showDeviceInfo(); break;
        case 1: showSettings();   break;
      }
      return;
  }

  ledCheckStep++;
}
#endif  // TALLY_EXTRA_OUTPUT

// ----------------------------------------------------------------------------
// Reset button (TRIGGER_PIN)
// States: idle → debounce → holding → action
// ----------------------------------------------------------------------------

void resetTick() {
  bool pinLow = (digitalRead(TRIGGER_PIN) == LOW);

  if (!resetWaiting && !resetHolding) {
    if (pinLow) {
      resetWaiting   = true;
      resetPressedMs = millis();
    }
    return;
  }

  if (resetWaiting) {
    if (millis() - resetPressedMs < RESET_DEBOUNCE_MS) return;
    if (!pinLow) {
      resetWaiting = false;
      return;
    }
    resetWaiting   = false;
    resetHolding   = true;
    resetPressedMs = millis();
    m5_fillScreen(TFT_BLACK);
    configureDisplayToCheckReset();
    m5_setTextColor(WHITE, BLACK);
    m5_println("Reset button pushed....");
    logger("Button Pressed", "info");
    return;
  }

  if (resetHolding) {
    if (pinLow && millis() - resetPressedMs >= RESET_HOLD_MS) {
      m5_println("Erasing....");
      logger("Erasing Config, restarting", "info");
      wm.resetSettings();
      ESP.restart();
    } else if (!pinLow) {
      resetHolding = false;
      m5_println("Starting Portal...");
      logger("Starting config portal", "info");
      wm.setConfigPortalTimeout(120);
      if (!wm.startConfigPortal(listenerDeviceName.c_str())) {
        logger("failed to connect or hit timeout", "error");
      } else {
        logger("connected...yeey :)", "info");
      }
      prevType = "__invalidated__";
      switch (currentScreen) {
        case 0: showDeviceInfo(); break;
        case 1: showSettings();   break;
      }
    }
  }
}
