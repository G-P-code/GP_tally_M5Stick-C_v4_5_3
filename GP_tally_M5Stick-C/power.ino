// ============================================================================
// power.ino — CPU performance mode and battery info cache
// ============================================================================

// Switch CPU between 80 MHz (idle/power-save) and 160 MHz (active).
void setPerformanceMode(bool high) {
  static bool currentMode = false;
  if (high == currentMode) return;
  setCpuFrequencyMhz(high ? 160 : 80);
  currentMode = high;
}

// Read battery state into batteryCache every BATTERY_UPDATE_INTERVAL ms.
// The cache is used by paintSettingsScreen() to avoid blocking I²C reads
// during screen paint. If the value changes while the settings screen is
// open, paintSettingsScreen() is called to reflect the update immediately.
void updateBatteryInfo() {
  unsigned long now = millis();
  if (now - batteryCache.lastRead < BATTERY_UPDATE_INTERVAL) return;
  batteryCache.lastRead = now;

  // Snapshot previous state to detect changes
  int  prevLevel      = batteryCache.level;
  bool prevCharging   = batteryCache.isCharging;
  bool prevCharged    = batteryCache.isCharged;

#if defined(STICK_C_PLUS2)
  // getBatteryVoltage() returns millivolts on Plus2.
  // Usable Li-ion range: 3000 mV (empty) – 4200 mV (full).
  float voltage_mv = StickCP2.Power.getBatteryVoltage();
  float batCurrent = StickCP2.Power.getBatteryCurrent();
  batteryCache.isCharging = (batCurrent > 0);
  batteryCache.isCharged  = (!batteryCache.isCharging && StickCP2.Power.isCharging());
  int raw = (int)(100.0f * (voltage_mv - 3000.0f) / (4200.0f - 3000.0f));
#else
  // GetBatVoltage() returns volts on AXP192 (StickC / StickC-Plus).
  // Usable range: 3.0 V (empty) – 4.2 V (full).
  //
  // AXP192 charging state detection:
  //   GetBatCurrent() is unreliable at high SoC — returns 0 even while charging.
  //   GetVinVoltage() > 3.0 V reliably indicates USB is connected.
  //   Strategy: USB present + level < 95 → Charging
  //             USB present + level >= 95 → Charged.
  float voltage    = M5.Axp.GetBatVoltage();
  // M5StickC-Plus uses VBUS (not VIN) for USB power — GetVinVoltage() always
  // returns 0. GetVBusVoltage() > 3.0 V reliably indicates USB is connected.
  bool  usbPresent = (M5.Axp.GetVBusVoltage() > 3.0f);
  int raw = (int)(100.0f * (voltage - 3.0f) / (4.2f - 3.0f));
  int  soc = constrain(raw, 0, 100);
  batteryCache.isCharging = (usbPresent && soc < 95);
  batteryCache.isCharged  = (usbPresent && soc >= 95);
  raw = soc;
#endif

  batteryCache.level = constrain(raw, 0, 100);

  // If anything changed and the settings screen is showing, repaint immediately
  if (currentScreen == 1) {
    if (batteryCache.level    != prevLevel    ||
        batteryCache.isCharging != prevCharging ||
        batteryCache.isCharged  != prevCharged) {
      paintSettingsScreen();
    }
  }
}
