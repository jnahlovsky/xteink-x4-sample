// NavigationController.cpp
#include "NavigationController.h"

NavigationController::NavigationController(
    ButtonHandler &buttonHandler,
    QuestionManager &questionManager,
    SleepManager &sleepManager
#ifdef DEBUG_IO
    ,
    DebugHelper &debugHelper
#endif
    )
    : _buttonHandler(buttonHandler),
      _questionManager(questionManager),
      _sleepManager(sleepManager)
#ifdef DEBUG_IO
      ,
      _debugHelper(debugHelper)
#endif
{
}

void NavigationController::processInput()
{
  Button currentButton = _buttonHandler.getPressedButton();
  Button lastButton = _buttonHandler.getLastButton();

  // Detect button press (transition from NONE to a button)
  if (currentButton != NONE && lastButton == NONE)
  {
    Serial.print("Button: ");
    Serial.println(_buttonHandler.getButtonName(currentButton));

    // Handle navigation
    bool needsRedraw = false;

    switch (currentButton)
    {
    case RIGHT:
      _questionManager.nextQuestion();
      needsRedraw = true;
      break;

    case LEFT:
      _questionManager.previousQuestion();
      needsRedraw = true;
      break;

    case CONFIRM:
      _questionManager.randomQuestion();
      needsRedraw = true;
      break;

    case VOLUME_UP:
      _questionManager.nextCategory();
      needsRedraw = true;
      break;

    case VOLUME_DOWN:
      _questionManager.previousCategory();
      needsRedraw = true;
      break;

    case POWER:
      _sleepManager.handleSleep();
      break;

    default:
      break;
    }

    // Update display if navigation occurred
    if (needsRedraw)
    {
      _questionManager.updateDisplay();
    }

#ifdef DEBUG_IO
    _debugHelper.logDebugInfo();
#endif
  }

  _buttonHandler.setLastButton(currentButton);
}
