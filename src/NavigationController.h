// NavigationController.h
#pragma once

#include <Arduino.h>
#include "ButtonHandler.h"
#include "QuestionManager.h"
#include "SleepManager.h"

#ifdef DEBUG_IO
#include "DebugHelper.h"
#endif

class NavigationController
{
public:
  NavigationController(
      ButtonHandler &buttonHandler,
      QuestionManager &questionManager,
      SleepManager &sleepManager
#ifdef DEBUG_IO
      ,
      DebugHelper &debugHelper
#endif
  );

  // Process button input and handle navigation
  void processInput();

private:
  ButtonHandler &_buttonHandler;
  QuestionManager &_questionManager;
  SleepManager &_sleepManager;
#ifdef DEBUG_IO
  DebugHelper &_debugHelper;
#endif
};
