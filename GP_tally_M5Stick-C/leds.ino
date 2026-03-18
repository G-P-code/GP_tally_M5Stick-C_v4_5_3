// ============================================================================
// leds.ino — External LED control, flash and reassign animations
// ============================================================================

// Update external LEDs to reflect actualType.
// Called from evaluateMode() — never blocked by screen state.
void externalLed() {
#if TALLY_EXTRA_OUTPUT
  if (ledCheckActive) return;  // LED check animation owns the LEDs
  if (!ledsEnabled) {          // LEDs disabled by user (btnM5 double-click)
    ledcWrite(led_program, 0);
    ledcWrite(led_preview, 0);
    ledcWrite(led_aux, 0);
    return;
  }

  if (actualType == "program") {
    logger("---- " + actualType, "info");
    ledcWrite(led_program, dutyCycle);
    ledcWrite(led_preview, 0);
    ledcWrite(led_aux, 0);
  } else if (actualType == "preview") {
    logger("---- " + actualType, "info");
    ledcWrite(led_program, 0);
    ledcWrite(led_preview, dutyCycle);
    ledcWrite(led_aux, 0);
  } else if (actualType == "aux") {
    logger("---- " + actualType, "info");
    ledcWrite(led_program, 0);
    ledcWrite(led_preview, 0);
    ledcWrite(led_aux, dutyCycle / 4);
  } else {
    ledcWrite(led_program, 0);
    ledcWrite(led_preview, 0);
    ledcWrite(led_aux, 0);
  }
#endif
}

// Toggle external LEDs on/off (btnM5 double-click).
// When disabled: all LEDs off immediately and stay off until re-enabled.
// When re-enabled: externalLed() is called to restore correct tally state.
void toggleLeds() {
  ledsEnabled = !ledsEnabled;
  logger("ledsEnabled: " + String(ledsEnabled), "info");
  if (!ledsEnabled) {
    ledcWrite(led_program, 0);
    ledcWrite(led_preview, 0);
    ledcWrite(led_aux, 0);
  } else {
    prevLedType = "__invalidated__";  // force externalLed() to reapply tally state
    externalLed();
    prevLedType = actualType;
  }
  // Repaint so LED status indicator updates on screen
  repaintCurrentScreen();
}

// Recalculate dutyCycle from currentBrightness using a quadratic curve so
// that equal brightness steps feel perceptually equal.
void set_dutyCycle() {
#if TALLY_EXTRA_OUTPUT
  float norm = (float)(currentBrightness - START_BRIGHTNESS) /
               (float)(MAX_BRIGHTNESS    - START_BRIGHTNESS);
  norm = constrain(norm, 0.0f, 1.0f);
  dutyCycle = (int)(MIN_DUTY + (MAX_DUTY - MIN_DUTY) * norm * norm);
#endif
}

// ----------------------------------------------------------------------------
// Flash animation (white blink on "flash" socket event)
// ----------------------------------------------------------------------------

void flashTick() {
  if (!flashActive) return;
  unsigned long now = millis();
  if (now - flashLastMs < FLASH_INTERVAL) return;
  flashLastMs = now;

  if (flashStep >= FLASH_COUNT) {
    flashActive = false;
    prevType = "__invalidated__";
    switch (currentScreen) {
      case 0: showDeviceInfo(); break;
      case 1: showSettings();   break;
    }
    return;
  }
  m5_fillScreen((flashStep % 2 == 0) ? WHITE : TFT_BLACK);
  flashStep++;
}

// ----------------------------------------------------------------------------
// Reassign blink animation (red blink on device reassignment)
// ----------------------------------------------------------------------------

void reassignTick() {
  if (!reassignActive) return;
  unsigned long now = millis();
  if (now - reassignLastMs < REASSIGN_INTERVAL) return;
  reassignLastMs = now;

  if (reassignStep >= REASSIGN_COUNT) {
    reassignActive = false;
    prevType = "__invalidated__";
    switch (currentScreen) {
      case 0: showDeviceInfo(); break;
      case 1: showSettings();   break;
    }
    return;
  }
  m5_fillScreen((reassignStep % 2 == 0) ? (uint32_t)RED_COLOR : (uint32_t)TFT_BLACK);
  reassignStep++;
}
