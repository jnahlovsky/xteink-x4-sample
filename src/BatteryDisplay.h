// BatteryDisplay.h
#pragma once

#include <Arduino.h>
#include "BatteryMonitor.h"
#include "UIRenderer.h"
#include "config.h"

class BatteryDisplay
{
public:
  BatteryDisplay(BatteryMonitor &battery, UIRenderer &renderer);

  // Check if battery display needs updating and update if needed
  void checkAndUpdate();

  // Force battery display update
  void forceUpdate();

  // Reset tracking state
  void reset();

private:
  BatteryMonitor &_battery;
  UIRenderer &_renderer;

  // Battery state tracking for independent updates
  int _lastDisplayedBatteryLevel; // Battery level in 10% increments (0-10)
  bool _lastChargingState;
  unsigned long _lastBatteryCheckTime;

  static const unsigned long BATTERY_CHECK_INTERVAL_MS = 5000; // Check every 5 seconds

  // Check if device is charging
  bool isCharging() const;
};
