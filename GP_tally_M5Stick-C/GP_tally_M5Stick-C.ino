// ************************************************************************************************************** //
// Based on https://github.com/josephdadams/TallyArbiter/listener_clients/m5stickc-listener/m5stickc-listener.ino //
// by Joseph Adams                                                                                                //
// ************************************************************************************************************** //

String version = "4.5.3";

//
// Automatic support for board selection with Arduino IDE
//
// Board defines used by Arduino IDE as preprocessor flags when selecting boards
// Boards > M5Stack Arduino > M5StickC / M5StickCPlus / M5StickCPlus2
//   M5StickC       ARDUINO_m5stack_stickc
//   M5StickC-Plus  ARDUINO_m5stack_stickc_plus
//   M5StickC-Plus2 ARDUINO_m5stack_stickc_plus2
//
// Libraries used:
//   WebSockets 2.3.4, ArduinoJson v7,
//   WifiManager (tzapu), MultiButton, ArduinoOTA
//
#pragma GCC optimize ("-O2")

#if defined(ARDUINO_M5STACK_STICKC)
  #define STICK_C
#endif
#if defined(ARDUINO_M5STACK_STICKC_PLUS)
  #define STICK_C_PLUS
#endif
#if defined(ARDUINO_M5STACK_STICKC_PLUS2)
  #define STICK_C_PLUS2
#endif

// Manual override — uncomment exactly one if IDE auto-detection is unavailable:
//#define STICK_C
//#define STICK_C_PLUS
//#define STICK_C_PLUS2

// Default to M5StickC Plus if nothing is defined
#if !defined(STICK_C) && !defined(STICK_C_PLUS) && !defined(STICK_C_PLUS2)
  #define STICK_C_PLUS
#endif

// Guard against multiple defines
#if defined(STICK_C_PLUS2)
  #include <M5StickCPlus2.h>
  #if defined(STICK_C) || defined(STICK_C_PLUS)
    #error "Multiple m5stick board types defined"
  #endif
#endif
#if defined(STICK_C_PLUS)
  #include <M5StickCPlus.h>
  #if defined(STICK_C) || defined(STICK_C_PLUS2)
    #error "Multiple m5stick board types defined"
  #endif
#endif
#if defined(STICK_C)
  #include <M5StickC.h>
  #if defined(STICK_C_PLUS) || defined(STICK_C_PLUS2)
    #error "Multiple m5stick board types defined"
  #endif
#endif

#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <PinButton.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#if !defined(STICK_C_PLUS2)
  #include "Free_Fonts.h"
#else
  // M5GFX (used by Plus2) includes FreeSans fonts via M5StickCPlus2.h.
  // Define the same aliases used elsewhere in the code.
  #define FSS9  &fonts::FreeSans9pt7b
  #define FSS24 &fonts::FreeSans24pt7b
#endif
#include "config.h"

// ============================================================================
// USER CONFIG — change before flashing
// ============================================================================
bool LAST_MSG = false;              // true = show last log message on tally screen
IPAddress apIP(192, 168, 6, 1);    // captive portal IP
char tallyarbiter_host[40] = "0.0.0.0";
char tallyarbiter_port[6]  = "4455";
// ============================================================================

// External LED output enable
#define TALLY_EXTRA_OUTPUT true

// ============================================================================
// Board-specific device name strings
// ============================================================================
#if defined(STICK_C_PLUS2)
  String listenerDeviceName = "m5StickC-plus2-";
  String listenerDeviceHW   = "M5StickC-Plus2";
#elif defined(STICK_C_PLUS)
  String listenerDeviceName = "m5StickC-plus-";
  String listenerDeviceHW   = "M5StickC-Plus";
#else
  String listenerDeviceName = "m5StickC-";
  String listenerDeviceHW   = "M5StickC";
#endif

// ============================================================================
// External LED pin assignments & PWM config
// ============================================================================
#if TALLY_EXTRA_OUTPUT
  #if defined(STICK_C_PLUS2)
    const int led_program = 19;
  #else
    const int led_program = 0;
  #endif
  const int led_preview  = 26;
  const int led_aux      = 25;
  const int freq         = 5000;
  const int resolution   = 8;
  int dutyCycle          = 0;
#endif

// ============================================================================
// Tally state
// ============================================================================
String prevType    = "";               // last displayed tally type (display only)
String prevLedType = "";               // last applied LED tally type (LED only)
String actualType  = "";
String actualColor = "";
int    actualPriority = 0;

// ============================================================================
// TallyArbiter / socket variables
// ============================================================================
WebSocketsClient socket;

// ArduinoJson documents — bus options and device data
// JsonDocument is reused; device_states is re-parsed each time
JsonDocument jBusOptions;   // bus definitions (type, color, priority)
JsonDocument jDevices;      // device list (id, name)
JsonDocument jDeviceStates; // current tally state per device per bus
String DeviceId   = "unassigned";
String DeviceName = "Unassigned";
String LastMessage = "";

bool taConnected          = false;
bool everConnected        = false;  // set true on first successful CONNECT
bool busOptionsReady      = false;  // set true once bus_options received from TA
bool userRequestedSettings = false;   // true = user opened settings manually

// ============================================================================
// General state
// ============================================================================
bool networkConnected  = false;
int  currentScreen     = 0;           // 0 = tally, 1 = settings
int  currentBrightness = DEFAULT_BRIGHTNESS;  // overridden by saved preference on boot
int  currentRotation   = 1;

WiFiManager wm;
bool portalRunning = false;

float accX = 0, accY = 0, accZ = 0;

// ============================================================================
// Animation state machines
// ============================================================================
#if TALLY_EXTRA_OUTPUT
  bool          ledCheckActive       = false;
  int           ledCheckStep         = 0;
  unsigned long ledCheckLastMs       = 0;
  int           ledCheckOriginScreen = 0;
  bool          ledsEnabled          = true;   // toggled by btnM5 double-click
#endif

bool          flashActive    = false;
int           flashStep      = 0;
unsigned long flashLastMs    = 0;

bool          reassignActive  = false;
int           reassignStep    = 0;
unsigned long reassignLastMs  = 0;

bool          resetWaiting    = false;
bool          resetHolding    = false;
unsigned long resetPressedMs  = 0;

bool          brightnessOsdActive       = false;
unsigned long brightnessOsdLastMs       = 0;
int           brightnessOsdOriginScreen = 0;

// ============================================================================
// Battery cache
// ============================================================================
struct BatteryInfo {
  int           level      = 0;
  bool          isCharging = false;  // current flowing into battery
  bool          isCharged  = false;  // full and on charger (current == 0 but USB present)
  unsigned long lastRead   = 0;
} batteryCache;

// ============================================================================
// Reconnection
// ============================================================================
unsigned long lastReconnectAttempt = 0;
int           reconnectDelay       = RECONNECT_BASE_DELAY;

// ============================================================================
// IMU throttle
// ============================================================================
unsigned long lastImuRead = 0;

// ============================================================================
// Preferences
// ============================================================================
Preferences preferences;

// ============================================================================
// Buttons
// ============================================================================
const MultiButtonConfig BTN_CONFIG = { 20, 250, 600 };
PinButton btnM5(37, &BTN_CONFIG);
PinButton btnAction(39, &BTN_CONFIG);

// ============================================================================
// Animation guard — true while any animation is running
// ============================================================================
bool animationActive() {
  bool active = flashActive || reassignActive || brightnessOsdActive;
#if TALLY_EXTRA_OUTPUT
  active = active || ledCheckActive;
#endif
  return active;
}

// ============================================================================
// Forward declarations
// (Arduino IDE auto-generates these for the main .ino but not for other .ino
//  files — listing them here ensures all files can call all functions.)
// ============================================================================

// screen.ino
void paintSettingsScreen();
void paintTallyScreen();
void repaintCurrentScreen();
void switchToSettings(bool byUser = false);
void switchToTally();
void showSettings();
void showDeviceInfo();
void evaluateMode();

// buttons.ino
void updateBrightness();
void showBrightnessOsd();
void brightnessTick();
void ledSequentialCheck();
void resetTick();

// tally.ino
void connectToServer();
void wsEvent(WStype_t type, uint8_t *payload, size_t length);
void onSocketConnect();
void onSocketDisconnect();
void onSocketEvent(String raw);
void socket_Flash();
void socket_Reassign(String oldDeviceId, String newDeviceId);
void processTallyData();
void getBusInfo(const String &busId, String &outType, String &outColor, int &outPriority);
void SetDeviceName();

// network.ino
void connectToNetwork();
void saveParamCallback();
String getParam(String name);
void WiFiEvent(WiFiEvent_t event);
void handleReconnection();
void checkPendingRestart();
void optimizeWiFiPower();

// leds.ino
void externalLed();
void toggleLeds();
void set_dutyCycle();
void flashTick();
void reassignTick();
#if TALLY_EXTRA_OUTPUT
void ledCheckTick();
#endif

// power.ino
void setPerformanceMode(bool high);
void updateBatteryInfo();

// m5hal.ino
void logger(String strLog, String strType);
void m5_begin();
void m5_configInitialScreen();
void m5_configSettingsScreen();
void m5_setRotation(uint_fast8_t rotation);
void m5_println();
void m5_println(const String &s);
void m5_println(const char c[]);
void m5_print(const char str[]);
void m5_setTextColor(uint16_t color);
void m5_setTextColor(uint16_t fgcolor, uint16_t bgcolor);
void m5_fillScreen(uint32_t color);
void m5_fillRect(int x, int y, int w, int h, uint32_t color);
void m5_drawRect(int x, int y, int w, int h, uint32_t color);
uint16_t m5_color565(uint8_t r, uint8_t g, uint8_t b);
uint8_t  m5_BtnA_pressedFor(uint32_t ms);
int  m5_displayWidth();
int  m5_displayHeight();
int  lcdBrightness();
int  lcdBrightnessPlus2();
void configureDisplayToEvaluateMode();
void configureDisplayToCheckReset();
void ledCheckScreen(uint32_t bgColor, uint16_t textColor, String label, String pinNum, int stepNum, int totalSteps);

// ============================================================================
// setup()
// ============================================================================
void setup() {
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  unsigned long serialWaitStart = millis();
  while (!Serial && (millis() - serialWaitStart < 2000));

  logger("Initializing " + listenerDeviceHW + ".", "info-quiet");

  setCpuFrequencyMhz(80);
  btStop();

  byte mac[6];
  Network.macAddress(mac);
  // Pad each byte to 2 hex digits so the suffix is always exactly 6 chars
  auto hexByte = [](uint8_t b) -> String {
    return (b < 0x10 ? "0" : "") + String(b, HEX);
  };
  listenerDeviceName += hexByte(mac[3]) + hexByte(mac[4]) + hexByte(mac[5]);
  wm.setHostname(listenerDeviceName.c_str());

  m5_begin();

  preferences.begin(PREF_NAMESPACE, false);
  logger("Reading preferences", "info-quiet");
  if (preferences.getString(PREF_DEVICE_ID, "").length() > 0)
    DeviceId = preferences.getString(PREF_DEVICE_ID, "");
  if (preferences.getString(PREF_DEVICE_NAME, "").length() > 0)
    DeviceName = preferences.getString(PREF_DEVICE_NAME, "");
  if (preferences.getString(PREF_TA_HOST, "").length() > 0) {
    String h = preferences.getString(PREF_TA_HOST, "");
    logger("Setting TallyArbiter host as " + h, "info-quiet");
    h.toCharArray(tallyarbiter_host, 40);
  }
  if (preferences.getString(PREF_TA_PORT, "").length() > 0) {
    String p = preferences.getString(PREF_TA_PORT, "");
    logger("Setting TallyArbiter port as " + p, "info-quiet");
    p.toCharArray(tallyarbiter_port, 6);
  }
  if (preferences.getInt(PREF_BRIGHTNESS, 0) > 0)
    currentBrightness = preferences.getInt(PREF_BRIGHTNESS, DEFAULT_BRIGHTNESS);
  preferences.end();

#if defined(STICK_C_PLUS2)
  M5.Imu.begin();
#else
  M5.IMU.Init();
#endif
  m5_setRotation(1);
  m5_fillScreen(TFT_BLACK);
  m5_configInitialScreen();
  m5_setTextColor(WHITE, BLACK);
  m5_println("booting device " + hexByte(mac[3]) + hexByte(mac[4]) + hexByte(mac[5]) + " ...");

  logger("Tally Arbiter " + listenerDeviceHW + " Listener Client booting.", "info");
  logger("Listener device name: " + listenerDeviceName, "info");

  connectToNetwork();
  unsigned long networkWaitStart = millis();
  while (!networkConnected) {
    delay(200);
    if (millis() - networkWaitStart > 180000) {
      logger("Network wait timeout. Continuing without network.", "error");
      break;
    }
  }

  ArduinoOTA.setHostname(listenerDeviceName.c_str());
  ArduinoOTA.setPassword("tallyarbiter");
  ArduinoOTA
    .onStart([]() {
      String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
      Serial.println("Start updating " + type);
    })
    .onEnd([]()   { Serial.println("\nEnd"); })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", progress / (total / 100));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if      (error == OTA_AUTH_ERROR)    logger("Auth Failed",    "error");
      else if (error == OTA_BEGIN_ERROR)   logger("Begin Failed",   "error");
      else if (error == OTA_CONNECT_ERROR) logger("Connect Failed", "error");
      else if (error == OTA_RECEIVE_ERROR) logger("Receive Failed", "error");
      else if (error == OTA_END_ERROR)     logger("End Failed",     "error");
    });
  ArduinoOTA.begin();

#if TALLY_EXTRA_OUTPUT
  set_dutyCycle();
  ledcAttach(led_program, freq, resolution);
  ledcAttach(led_preview, freq, resolution);
  ledcAttach(led_aux,     freq, resolution);
#endif

  connectToServer();
  switchToSettings();  // stay on settings until first successful connection
}

// ============================================================================
// loop()
// ============================================================================
void loop() {
  if (portalRunning) wm.process();

  ArduinoOTA.handle();
  socket.loop();
  btnM5.update();
  btnAction.update();
  M5.update();

  // IMU — read at reduced frequency, act on rotation change only
  unsigned long now = millis();
  if (now - lastImuRead >= IMU_READ_INTERVAL) {
#if defined(STICK_C_PLUS2)
    M5.Imu.getAccel(&accX, &accY, &accZ);
#else
    M5.IMU.getAccelData(&accX, &accY, &accZ);
#endif
    lastImuRead = now;

    int newRotation = currentRotation;
    if      (accX < -0.70) newRotation = 3;
    else if (accX >  0.70) newRotation = 1;

    if (newRotation != currentRotation && !animationActive()) {
      currentRotation = newRotation;
      m5_setRotation(currentRotation);
      prevType = "__invalidated__";
      repaintCurrentScreen();
      btnM5.update();
      btnAction.update();
      return;
    }
  }

  // WiFi reset: hold btnM5 for 5 s
  if (m5_BtnA_pressedFor(5000)) {
    logger("resetSettings()", "info");
    wm.resetSettings();
    ESP.restart();
  }

  // Screen toggle: single click btnM5
  if (btnM5.isSingleClick()) {
    if (currentScreen == 0) {
      switchToSettings(true);
    } else {
      userRequestedSettings = false;
      switchToTally();
    }
  }

  // LED toggle: double click btnM5
#if TALLY_EXTRA_OUTPUT
  if (btnM5.isDoubleClick()) toggleLeds();
#endif

  // Brightness cycle: single click btnAction
  if (btnAction.isSingleClick()) updateBrightness();

  // LED sequential check: long press btnAction
  if (btnAction.isLongClick()) ledSequentialCheck();

  // Tick functions
#if TALLY_EXTRA_OUTPUT
  ledCheckTick();
#endif
  flashTick();
  reassignTick();
  resetTick();
  brightnessTick();

  // Maintenance
  checkPendingRestart();
  handleReconnection();
  optimizeWiFiPower();
  updateBatteryInfo();
  setPerformanceMode(animationActive() || portalRunning || !taConnected);
}
