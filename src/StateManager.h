// StateManager.h
#pragma once

#include <Arduino.h>
#include <Preferences.h>

class StateManager
{
public:
  StateManager();

  // Initialize preferences
  void begin();

  // Load last viewed card index
  int loadLastCardIndex(int maxIndex);

  // Save current card index
  void saveCardIndex(int index);

private:
  Preferences _preferences;
  bool _initialized;
};
