// QuestionManager.h
#pragma once

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include "BatteryMonitor.h"
#include "BatteryDisplay.h"
#include "UIRenderer.h"
#include "StateManager.h"
#include "Questions.h"

class QuestionManager
{
public:
  QuestionManager(
      GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> &display,
      UIRenderer &uiRenderer,
      BatteryMonitor &battery,
      BatteryDisplay &batteryDisplay,
      StateManager &stateManager);

  // Initialize - loads last card index
  void begin();

  // Display initial question screen
  void displayInitialQuestion();

  // Navigation
  void nextQuestion();
  void previousQuestion();
  void randomQuestion();
  void nextCategory();
  void previousCategory();

  // Get current state
  int getCurrentIndex() const { return _currentIndex; }
  const char *getCurrentCategory() const;

  // Handle display update after navigation
  void updateDisplay();

  // Save current state
  void saveState();

private:
  GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> &_display;
  UIRenderer &_uiRenderer;
  BatteryMonitor &_battery;
  BatteryDisplay &_batteryDisplay;
  StateManager &_stateManager;

  int _currentIndex;
  char _lastCategory[32];
  int _refreshCount;
  unsigned long _debounceEndTime;

  // Helper methods
  void drawQuestionWithBattery(const char *questionText, const char *category);
  bool shouldUseFullRefresh();
  bool hasCategoryChanged();
  void updateCategoryTracking();
  unsigned long getDebounceTime(bool fullRefresh, bool categoryChanged);
};
