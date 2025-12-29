// SleepManager.cpp
#include "SleepManager.h"
#include "logo.h"

SleepManager::SleepManager(
    GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> &display,
    QuestionManager &questionManager)
    : _display(display),
      _questionManager(questionManager)
{
}

void SleepManager::handleSleep()
{
  unsigned long startTime = millis();

  // Wait for button release
  while (digitalRead(BTN_GPIO3) == LOW)
    delay(50);

  unsigned long currentTime = millis();

  // Power button long pressed => go to sleep
  if (currentTime - startTime > POWER_BUTTON_SLEEP_MS)
  {
    // Save current card state before sleep
    _questionManager.saveState();

    // Display sleep screen with logo
    Serial.println("Displaying sleep screen...");
    _display.setFullWindow();
    _display.firstPage();
    do
    {
      _display.fillScreen(GxEPD_WHITE);
      _display.drawBitmap(0, 0, logo, 800, 480, GxEPD_BLACK);
    } while (_display.nextPage());
    _display.hibernate();

    Serial.println("Entering deep sleep...");
    delay(1000);

    // Enter deep sleep
    esp_deep_sleep_enable_gpio_wakeup(1ULL << BTN_GPIO3, ESP_GPIO_WAKEUP_GPIO_LOW);
    esp_deep_sleep_start();
  }
}
