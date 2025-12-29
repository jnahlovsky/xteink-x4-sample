// StateManager.cpp
#include "StateManager.h"

StateManager::StateManager() : _initialized(false)
{
}

void StateManager::begin()
{
  _initialized = true;
}

int StateManager::loadLastCardIndex(int maxIndex)
{
  if (!_preferences.begin("cardApp", true)) // true = read-only mode
  {
    Serial.println("Failed to open NVS for reading");
    return 0;
  }

  int savedIndex = _preferences.getInt("lastCardId", 0);
  _preferences.end();

  // Validate the index is within bounds
  if (savedIndex < 0 || savedIndex >= maxIndex)
  {
    Serial.printf("Invalid saved index %d, defaulting to 0\n", savedIndex);
    return 0;
  }

  Serial.printf("Loaded last card index: %d\n", savedIndex);
  return savedIndex;
}

void StateManager::saveCardIndex(int index)
{
  if (!_preferences.begin("cardApp", false)) // false = read-write mode
  {
    Serial.println("Failed to open NVS for writing");
    return;
  }

  _preferences.putInt("lastCardId", index);
  _preferences.end();
  Serial.printf("Saved last card index: %d\n", index);
}
