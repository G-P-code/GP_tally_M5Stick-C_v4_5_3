// ============================================================================
// m5hal.ino — M5Stack hardware abstraction layer
//
// All direct hardware calls are isolated here so the rest of the code
// stays board-independent.  Add a new board variant by extending each
// function with the appropriate #elif block.
// ============================================================================

// ----------------------------------------------------------------------------
// Logging
// ----------------------------------------------------------------------------

void logger(String strLog, String strType) {
  if (strType == "error") {
    Serial.println("[ERROR] " + strLog);
  } else if (strType == "info") {
    Serial.println("[INFO]  " + strLog);
  } else if (strType == "info-quiet") {
    // Uncomment for verbose debug output:
    // Serial.println("[DEBUG] " + strLog);
  } else {
    Serial.println(strLog);
  }
}

// ----------------------------------------------------------------------------
// Initialisation
// ----------------------------------------------------------------------------

void m5_begin() {
#if defined(STICK_C_PLUS2)
  auto cfg = M5.config();
  StickCP2.begin(cfg);
#else
  M5.begin();
#endif
}

void m5_configInitialScreen() {
#if defined(STICK_C_PLUS2)
  StickCP2.Display.setCursor(0, 0);
  StickCP2.Display.setFont(FSS9);
#elif defined(STICK_C_PLUS)
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setFreeFont(FSS9);
#else
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(1);
#endif
#if defined(STICK_C_PLUS2)
  StickCP2.Display.setBrightness(lcdBrightnessPlus2());
#else
  M5.Axp.ScreenBreath(lcdBrightness());
#endif
}

void m5_configSettingsScreen() {
#if defined(STICK_C_PLUS2)
  StickCP2.Display.setCursor(0, 0);
  StickCP2.Display.setFont(FSS9);
#elif defined(STICK_C_PLUS)
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setFreeFont(FSS9);
#else
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(1);
#endif
}

// ----------------------------------------------------------------------------
// Rotation
// ----------------------------------------------------------------------------

void m5_setRotation(uint_fast8_t rotation) {
#if defined(STICK_C_PLUS2)
  StickCP2.Display.setRotation(rotation);
#else
  M5.Lcd.setRotation(rotation);
#endif
}

// ----------------------------------------------------------------------------
// Text output
// ----------------------------------------------------------------------------

void m5_println() {
#if defined(STICK_C_PLUS2)
  StickCP2.Display.println();
#else
  M5.Lcd.println();
#endif
}

void m5_println(const String &s) {
#if defined(STICK_C_PLUS2)
  StickCP2.Display.println(s);
#else
  M5.Lcd.println(s);
#endif
}

void m5_println(const char c[]) {
#if defined(STICK_C_PLUS2)
  StickCP2.Display.println(c);
#else
  M5.Lcd.println(c);
#endif
}

void m5_print(const char str[]) {
#if defined(STICK_C_PLUS2)
  StickCP2.Display.print(str);
#else
  M5.Lcd.print(str);
#endif
}

void m5_setTextColor(uint16_t color) {
#if defined(STICK_C_PLUS2)
  StickCP2.Display.setTextColor(color);
#else
  M5.Lcd.setTextColor(color);
#endif
}

void m5_setTextColor(uint16_t fgcolor, uint16_t bgcolor) {
#if defined(STICK_C_PLUS2)
  StickCP2.Display.setTextColor(fgcolor, bgcolor);
#else
  M5.Lcd.setTextColor(fgcolor, bgcolor);
#endif
}

// ----------------------------------------------------------------------------
// Drawing
// ----------------------------------------------------------------------------

void m5_fillScreen(uint32_t color) {
#if defined(STICK_C_PLUS2)
  StickCP2.Display.fillScreen(color);
#else
  M5.Lcd.fillScreen(color);
#endif
}

void m5_fillRect(int x, int y, int w, int h, uint32_t color) {
#if defined(STICK_C_PLUS2)
  StickCP2.Display.fillRect(x, y, w, h, color);
#else
  M5.Lcd.fillRect(x, y, w, h, color);
#endif
}

void m5_drawRect(int x, int y, int w, int h, uint32_t color) {
#if defined(STICK_C_PLUS2)
  StickCP2.Display.drawRect(x, y, w, h, color);
#else
  M5.Lcd.drawRect(x, y, w, h, color);
#endif
}

uint16_t m5_color565(uint8_t r, uint8_t g, uint8_t b) {
#if defined(STICK_C_PLUS2)
  return StickCP2.Display.color565(r, g, b);
#else
  return M5.Lcd.color565(r, g, b);
#endif
}

// ----------------------------------------------------------------------------
// Dimensions
// ----------------------------------------------------------------------------

int m5_displayWidth() {
#if defined(STICK_C_PLUS2)
  return StickCP2.Display.width();
#else
  return M5.Lcd.width();
#endif
}

int m5_displayHeight() {
#if defined(STICK_C_PLUS2)
  return StickCP2.Display.height();
#else
  return M5.Lcd.height();
#endif
}

// ----------------------------------------------------------------------------
// Buttons
// ----------------------------------------------------------------------------

uint8_t m5_BtnA_pressedFor(uint32_t ms) {
#if defined(STICK_C_PLUS2)
  return StickCP2.BtnA.pressedFor(ms);
#else
  return M5.BtnA.pressedFor(ms);
#endif
}

// ----------------------------------------------------------------------------
// Cursor / font presets for each screen context
// ----------------------------------------------------------------------------

void configureDisplayToEvaluateMode() {
#if defined(STICK_C_PLUS2)
  StickCP2.Display.setCursor(4, 50);
  StickCP2.Display.setFont(FSS24);
#elif defined(STICK_C_PLUS)
  M5.Lcd.setCursor(4, 82);
  M5.Lcd.setFreeFont(FSS24);
#else
  M5.Lcd.setCursor(4, 30);
  M5.Lcd.setTextSize(2);
#endif
}


void configureDisplayToCheckReset() {
#if defined(STICK_C_PLUS2)
  StickCP2.Display.setCursor(0, 40);
  StickCP2.Display.setFont(FSS9);
#elif defined(STICK_C_PLUS)
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.setFreeFont(FSS9);
#else
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(1);
#endif
}

// ----------------------------------------------------------------------------
// LED check screen helper
// ----------------------------------------------------------------------------

void ledCheckScreen(uint32_t bgColor, uint16_t textColor, String label,
                    String pinNum, int stepNum, int totalSteps) {
  m5_fillScreen(bgColor);
  configureDisplayToCheckReset();
  m5_setTextColor(textColor, bgColor);
  m5_println("-- LED CHECK --");
  m5_println("Step " + String(stepNum) + "/" + String(totalSteps));
  m5_println("");
  m5_println(label);
  m5_println("Pin: " + pinNum);
}

// ----------------------------------------------------------------------------
// Brightness helpers
// ----------------------------------------------------------------------------

// Returns LCD brightness with a low-end boost so the screen is always
// comfortably readable even at minimum LED brightness.
int lcdBrightness() {
  float norm = (float)(currentBrightness - START_BRIGHTNESS) /
               (float)(MAX_BRIGHTNESS    - START_BRIGHTNESS);
  norm = constrain(norm, 0.0f, 1.0f);
  int boost = (int)(LCD_BOOST * (1.0f - norm));
  return constrain(currentBrightness + boost, START_BRIGHTNESS, MAX_BRIGHTNESS);
}

// Returns display brightness for StickC-Plus2 (0–255 scale).
// Maps internal brightness range (START_BRIGHTNESS–MAX_BRIGHTNESS) to 20–255
// so the screen is never completely dark at minimum setting.
int lcdBrightnessPlus2() {
  float norm = (float)(currentBrightness - START_BRIGHTNESS) /
               (float)(MAX_BRIGHTNESS    - START_BRIGHTNESS);
  norm = constrain(norm, 0.0f, 1.0f);
  return (int)(20.0f + (255.0f - 20.0f) * norm);
}
