#pragma once

// ============================================================================
// Hardware pins
// ============================================================================
#define TRIGGER_PIN 33          // reset pin (pin 4 on grove connector)

// ============================================================================
// Display colours
// ============================================================================
#define GREY      0x0020
#define RED_COLOR 0xF800

// ============================================================================
// Brightness
// ============================================================================
#define START_BRIGHTNESS  11   // minimum brightness (bottom of cycle range)
#define MAX_BRIGHTNESS   100
#define DEFAULT_BRIGHTNESS 51  // brightness on first boot (no saved preference)
#define LCD_BOOST         12    // extra AXP units added at minimum brightness

// ============================================================================
// LED PWM
// ============================================================================
#define MIN_DUTY   8
#define MAX_DUTY 250

// ============================================================================
// Timing constants  (all in milliseconds unless noted)
// ============================================================================
#define FLASH_INTERVAL        500
#define FLASH_COUNT             6   // total half-cycles
#define REASSIGN_INTERVAL     200
#define REASSIGN_COUNT          4
#define RESET_DEBOUNCE_MS      50
#define RESET_HOLD_MS        3000
#define BRIGHTNESS_OSD_MS    1500
#define LED_CHECK_INTERVAL   1000
#define IMU_READ_INTERVAL     100
#define BATTERY_UPDATE_INTERVAL 5000

// ============================================================================
// Network / reconnection
// ============================================================================
#define RECONNECT_BASE_DELAY   1000
#define RECONNECT_MAX_DELAY   30000
#define WIFI_POWER_SAVE_RSSI    -50  // dBm — use low TX power above this RSSI

// ============================================================================
// NVS preference keys  (all stored under PREF_NAMESPACE)
// ============================================================================
#define PREF_NAMESPACE  "tally-arbiter"
#define PREF_DEVICE_ID  "deviceid"
#define PREF_DEVICE_NAME "devicename"
#define PREF_TA_HOST    "taHost"
#define PREF_TA_PORT    "taPort"
#define PREF_BRIGHTNESS "brightness"
