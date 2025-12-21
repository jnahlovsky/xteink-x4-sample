#include "PowerManager.h"
#include "config.h"
#include <esp_sleep.h>

PowerManager::PowerManager(DisplayManager *displayMgr)
    : displayManager(displayMgr)
{
}

void PowerManager::begin()
{
  // Initialize power-related pins
  pinMode(UART0_RXD, INPUT);
}

bool PowerManager::wasWokenByGpio()
{
  return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO;
}

bool PowerManager::verifyWakeupLongPress()
{
  // Temporarily configure pin as digital input to check state
  pinMode(BTN_GPIO3, INPUT_PULLUP);

  unsigned long pressStart = millis();
  bool abortBoot = false;

  // Monitor button state for the duration
  while (millis() - pressStart < POWER_BUTTON_WAKEUP_MS)
  {
    // If button reads HIGH (released) before time is up
    if (digitalRead(BTN_GPIO3) == HIGH)
    {
      abortBoot = true;
      break;
    }
    delay(10);
  }

  if (abortBoot)
  {
    // Button released too early. Returning to sleep.
    // IMPORTANT: Re-arm the wakeup trigger before sleeping again
    esp_deep_sleep_enable_gpio_wakeup(1ULL << BTN_GPIO3, ESP_GPIO_WAKEUP_GPIO_LOW);
    esp_deep_sleep_start();
    return false; // Won't reach here
  }

  return true; // Boot confirmed
}

void PowerManager::enterDeepSleep()
{
  if (displayManager != nullptr)
  {
    displayManager->setDisplayCommand(DISPLAY_SLEEP);
  }

  Serial.println("Power button released after a long press. Entering deep sleep.");
  delay(2000); // Allow Serial buffer to empty and display to update

  // Enable Wakeup on LOW (button press)
  esp_deep_sleep_enable_gpio_wakeup(1ULL << BTN_GPIO3, ESP_GPIO_WAKEUP_GPIO_LOW);

  // Enter Deep Sleep
  esp_deep_sleep_start();
}
