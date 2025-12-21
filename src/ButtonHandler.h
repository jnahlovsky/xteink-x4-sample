#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

// Button enum
enum Button
{
  NONE = 0,
  RIGHT,
  LEFT,
  CONFIRM,
  BACK,
  VOLUME_UP,
  VOLUME_DOWN,
  POWER
};

class ButtonHandler
{
public:
  ButtonHandler();

  // Initialize button pins
  void begin();

  // Get currently pressed button by reading ADC values
  Button getPressedButton();

  // Get button name as string
  const char *getButtonName(Button btn);

  // Get last button state
  Button getLastButton() const { return lastButton; }

  // Set last button state
  void setLastButton(Button btn) { lastButton = btn; }

private:
  Button lastButton;
};

#endif // BUTTON_HANDLER_H
