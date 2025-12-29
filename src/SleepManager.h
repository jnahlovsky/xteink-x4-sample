// SleepManager.h
#pragma once

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include "QuestionManager.h"
#include "config.h"

class SleepManager
{
public:
  SleepManager(
      GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> &display,
      QuestionManager &questionManager);

  // Handle sleep mode (checks for long press and enters sleep if needed)
  void handleSleep();

private:
  GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> &_display;
  QuestionManager &_questionManager;
};
