// BatteryDisplay.cpp
#include "BatteryDisplay.h"

BatteryDisplay::BatteryDisplay(BatteryMonitor &battery, UIRenderer &renderer)
    : _battery(battery),
      _renderer(renderer),
      _lastDisplayedBatteryLevel(-1),
      _lastChargingState(false),
      _lastBatteryCheckTime(0)
{
}

bool BatteryDisplay::isCharging() const
{
  return digitalRead(UART0_RXD) == HIGH;
}

void BatteryDisplay::checkAndUpdate()
{
  unsigned long now = millis();
  if (now - _lastBatteryCheckTime < BATTERY_CHECK_INTERVAL_MS)
  {
    return; // Not time to check yet
  }
  _lastBatteryCheckTime = now;

  // Read current battery state
  int currentPercent = _battery.readPercentage();
  bool charging = isCharging();

  // Round to 10% level (0-10)
  int currentLevel = (currentPercent + 5) / 10;
  if (currentLevel < 0)
    currentLevel = 0;
  if (currentLevel > 10)
    currentLevel = 10;

  // Check if update needed (level changed by 10% or charging state changed)
  bool levelChanged = (currentLevel != _lastDisplayedBatteryLevel);
  bool chargingChanged = (charging != _lastChargingState);

  if (levelChanged || chargingChanged)
  {
    _renderer.updateBatteryDisplay(currentPercent, charging);
    _lastDisplayedBatteryLevel = currentLevel;
    _lastChargingState = charging;
  }
}

void BatteryDisplay::forceUpdate()
{
  int currentPercent = _battery.readPercentage();
  bool charging = isCharging();
  _renderer.updateBatteryDisplay(currentPercent, charging);

  // Update tracking state
  int currentLevel = (currentPercent + 5) / 10;
  if (currentLevel < 0)
    currentLevel = 0;
  if (currentLevel > 10)
    currentLevel = 10;
  _lastDisplayedBatteryLevel = currentLevel;
  _lastChargingState = charging;
  _lastBatteryCheckTime = millis();
}

void BatteryDisplay::reset()
{
  _lastDisplayedBatteryLevel = -1;
  _lastChargingState = false;
  _lastBatteryCheckTime = 0;
}
