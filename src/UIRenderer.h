// UIRenderer.h
#pragma once

#include <Arduino.h>
#include <GxEPD2_BW.h>

class UIRenderer
{
public:
  explicit UIRenderer(GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> &display);

  // Draw border around question area
  void drawBorder();

  // Draw question text centered in the display area
  void drawQuestionText(const char *text);

  // Draw card number (top-left corner)
  void drawCardNumber(int currentCard, int totalCards);

  // Draw parrot logo with cutout effect (bottom-left)
  void drawParrotWithCutout();

  // Draw category banner at bottom (overlay on border)
  void drawCategoryBanner(const char *category);

  // Draw category icon in circle (right side of banner)
  void drawCategoryIconInCircle(uint8_t categoryIndex);

  // Draw battery icon at specified position
  void drawBatteryIcon(int x, int y, int percentage);

  // Draw battery status text (percentage when charging)
  void drawBatteryStatus(const char *statusText, int x, int y);

  // Update battery display in top-right corner (partial window)
  void updateBatteryDisplay(int percentage, bool isCharging);

private:
  GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> &_display;
};
