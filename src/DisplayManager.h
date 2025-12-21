#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include "ButtonHandler.h"
#include "BatteryMonitor.h"

// Display command enum
enum DisplayCommand
{
  DISPLAY_NONE = 0,
  DISPLAY_INITIAL,
  DISPLAY_TEXT,
  DISPLAY_BATTERY,
  DISPLAY_SLEEP
};

class DisplayManager
{
public:
  DisplayManager(GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> &display);

  // Initialize display
  void begin();

  // Start display update task on core 0
  void startDisplayTask();

  // Set display command
  void setDisplayCommand(DisplayCommand cmd);

  // Get display command
  DisplayCommand getDisplayCommand() const { return displayCommand; }

  // Clear display command
  void clearDisplayCommand() { displayCommand = DISPLAY_NONE; }

  // Set current pressed button for display
  void setCurrentButton(Button btn) { currentPressedButton = btn; }

  // Set battery monitor reference
  void setBatteryMonitor(BatteryMonitor *monitor) { batteryMonitor = monitor; }

  // Display update task (called from FreeRTOS task)
  void updateDisplay();

private:
  GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> &display;
  BatteryMonitor *batteryMonitor;
  volatile DisplayCommand displayCommand;
  Button currentPressedButton;
  TaskHandle_t displayTaskHandle;

  // Drawing functions
  void drawBatteryInfo();
  void drawSdTopFiles();
  const char *getButtonName(Button btn);
  bool isCharging();
};

#endif // DISPLAY_MANAGER_H
