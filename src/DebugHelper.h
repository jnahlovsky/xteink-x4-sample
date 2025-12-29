// DebugHelper.h
#pragma once

#include <Arduino.h>
#include "BatteryMonitor.h"
#include "config.h"

class DebugHelper
{
public:
  explicit DebugHelper(BatteryMonitor &battery);

  // Log debug information (button states, battery info)
  void logDebugInfo();

private:
  BatteryMonitor &_battery;
};
