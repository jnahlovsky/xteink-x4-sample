#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "ButtonHandler.h"
#include "DisplayManager.h"

class PowerManager
{
public:
  PowerManager(DisplayManager *displayMgr);

  // Initialize power management
  void begin();

  // Check if woken from deep sleep and verify long press
  bool verifyWakeupLongPress();

  // Enter deep sleep mode
  void enterDeepSleep();

  // Check if device was woken by GPIO
  bool wasWokenByGpio();

private:
  DisplayManager *displayManager;
};

#endif // POWER_MANAGER_H
