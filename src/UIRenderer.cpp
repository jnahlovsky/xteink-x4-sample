// UIRenderer.cpp
#include "UIRenderer.h"
#include "Lexend_Bold18pt7b.h"
#include "Lexend_Bold24pt7b.h"
#include "Lexend_Light40pt7b.h"
#include "Utf8GfxHelper.h"

UIRenderer::UIRenderer(GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> &display)
    : _display(display)
{
}

void UIRenderer::drawBorder()
{
  // Draw 5px thick rounded rectangle border
  for (int i = 0; i < 5; i++)
  {
    _display.drawRoundRect(50 + i, 50 + i, 700 - i * 2, 320 - i * 2, 20, GxEPD_BLACK);
  }
}

void UIRenderer::drawBatteryIcon(int x, int y, int percentage)
{
  // Battery icon dimensions
  const int ICON_WIDTH = 28;
  const int ICON_HEIGHT = 16;
  const int TIP_WIDTH = 3;
  const int TIP_HEIGHT = 8;
  const int BORDER = 2;

  // Main battery body outline (right-aligned)
  int bodyX = x - ICON_WIDTH;
  _display.drawRect(bodyX, y, ICON_WIDTH - TIP_WIDTH, ICON_HEIGHT, GxEPD_BLACK);
  _display.drawRect(bodyX + 1, y + 1, ICON_WIDTH - TIP_WIDTH - 2, ICON_HEIGHT - 2, GxEPD_BLACK);

  // Positive terminal nub (right side)
  int tipY = y + (ICON_HEIGHT - TIP_HEIGHT) / 2;
  _display.fillRect(x - TIP_WIDTH, tipY, TIP_WIDTH, TIP_HEIGHT, GxEPD_BLACK);

  // Calculate fill width based on percentage (inside 2px border)
  // Round to 10% increments: 0%, 10%, 20%, ..., 100%
  int fillLevel = (percentage + 5) / 10; // Round to nearest 10%
  if (fillLevel < 0)
    fillLevel = 0;
  if (fillLevel > 10)
    fillLevel = 10;

  int innerX = bodyX + BORDER;
  int innerY = y + BORDER;
  int innerHeight = ICON_HEIGHT - (BORDER * 2);
  int maxFillWidth = ICON_WIDTH - TIP_WIDTH - (BORDER * 2);
  int fillWidth = (maxFillWidth * fillLevel) / 10;

  // Draw fill bar (empty if <10%)
  if (fillWidth > 0)
  {
    _display.fillRect(innerX, innerY, fillWidth, innerHeight, GxEPD_BLACK);
  }
}

void UIRenderer::drawBatteryStatus(const char *statusText, int x, int y)
{
  // Draw status text to the left of battery icon
  if (statusText != nullptr && statusText[0] != '\0')
  {
    _display.setFont(&Lexend_Bold18pt7b);
    _display.setCursor(x, y);
    _display.setTextColor(GxEPD_BLACK);
    _display.print(statusText);
  }
}

void UIRenderer::drawQuestionText(const char *text)
{
  // Display question centered in rectangle (50,50,700,320)
  // Rectangle center: (400, 210)
  // Inner box is 700x320, with padding we use 640x280 for text
  drawUtf8MultiLineWrapped(_display, &Lexend_Light40pt7b, text, 400, 210, 640, 280, GxEPD_BLACK);
}

void UIRenderer::drawCategoryBanner(const char *category)
{
  // Draw category banner at bottom (inverted colors)
  _display.fillRoundRect(250, 400, 300, 50, 10, GxEPD_BLACK);
  drawUtf8StringCentered(_display, &Lexend_Bold24pt7b, category, 400, 435, GxEPD_WHITE);
}

void UIRenderer::updateBatteryDisplay(int percentage, bool isCharging)
{
  // Independent battery update using small partial window
  // Window covers percentage + icon area in top-right corner
  // Percentage right-aligned next to icon with 5px gap
  const int WINDOW_X = 640;
  const int WINDOW_Y = 10;
  const int WINDOW_W = 150;
  const int WINDOW_H = 30;
  const int BATTERY_ICON_RIGHT_X = 780; // 20px from screen edge
  const int TEXT_ICON_GAP = 5;

  _display.setPartialWindow(WINDOW_X, WINDOW_Y, WINDOW_W, WINDOW_H);
  _display.firstPage();
  do
  {
    _display.fillRect(WINDOW_X, WINDOW_Y, WINDOW_W, WINDOW_H, GxEPD_WHITE);

    // Draw battery icon first
    drawBatteryIcon(BATTERY_ICON_RIGHT_X, 17, percentage);

    // Show percentage when charging
    if (isCharging)
    {
      char percentText[8];
      int displayPercent = (percentage >= 95) ? 100 : percentage;
      snprintf(percentText, sizeof(percentText), "%d%%", displayPercent);

      _display.setFont(&Lexend_Bold18pt7b);
      int16_t x1, y1;
      uint16_t w, h;
      _display.getTextBounds(percentText, 0, 0, &x1, &y1, &w, &h);
      int textX = BATTERY_ICON_RIGHT_X - 28 - TEXT_ICON_GAP - w; // 28 = ICON_WIDTH
      drawBatteryStatus(percentText, textX, 30);
    }
  } while (_display.nextPage());
  _display.hibernate();

  Serial.printf("Battery display updated: %d%% (charging: %s)\n", percentage, isCharging ? "yes" : "no");
}
