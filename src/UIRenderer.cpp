// UIRenderer.cpp
#include "UIRenderer.h"
#include "Lexend_Bold18pt7b.h"
#include "Lexend_Bold24pt7b.h"
#include "Lexend_Bold32pt7b.h"
#include "Lexend_Light40pt7b.h"
#include "Utf8GfxHelper.h"
#include "parrot.h"
#include "icons.h"

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

void UIRenderer::drawCardNumber(int currentCard, int totalCards)
{
  // Draw card number in top-left corner, aligned with battery on right
  // Battery is 20px from right edge, so card number is 20px from left edge
  // Same Y position as battery (baseline at Y=30)
  char cardText[16];
  snprintf(cardText, sizeof(cardText), "%03d/%03d", currentCard + 1, totalCards);

  _display.setFont(&Lexend_Bold18pt7b);
  _display.setCursor(20, 30);
  _display.setTextColor(GxEPD_BLACK);
  _display.print(cardText);
}

void UIRenderer::drawParrotWithCutout()
{
  // Parrot logo position: 64px from left, 37px from bottom
  // Bottom = 480px, so Y = 480 - 37 - 140 = 303
  const int PARROT_X = 64;
  const int PARROT_Y = 303;
  const int PARROT_W = 140;
  const int PARROT_H = 140;

  // Step 1: Erase border area exactly where parrot will be (no padding)
  // This removes the border line without creating a visible gap
  _display.fillRect(PARROT_X, PARROT_Y, PARROT_W, PARROT_H, GxEPD_WHITE);

  // Step 2: Draw actual parrot bitmap (inverted to correct colors)
  _display.drawInvertedBitmap(PARROT_X, PARROT_Y, parrot, PARROT_W, PARROT_H, GxEPD_BLACK);
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
  // Draw category banner overlaying the border bottom
  // Banner: 420x80px (extended 20px to the right), 60px from bottom
  // Banner right edge now aligns with circle center at X=670
  // Display height = 480px, so Y = 480 - 60 - 80 = 340
  const int BANNER_X = 250; // Left edge
  const int BANNER_Y = 340;
  const int BANNER_WIDTH = 420; // Extended from 400 to 420 (20px wider)
  const int BANNER_HEIGHT = 80;
  const int BANNER_RADIUS = 40; // Fully rounded corners

  _display.fillRoundRect(BANNER_X, BANNER_Y, BANNER_WIDTH, BANNER_HEIGHT, BANNER_RADIUS, GxEPD_BLACK);

  // Center text vertically and horizontally in banner, shifted 10px left
  // Banner center X = 250 + 400/2 = 450, minus 10px = 440
  // Text baseline Y = 340 + 80/2 + font_offset
  drawUtf8StringCentered(_display, &Lexend_Bold32pt7b, category, 440, 390, GxEPD_WHITE);
}

void UIRenderer::drawCategoryIconInCircle(uint8_t categoryIndex)
{
  // Draw category icon in white circle, moved 20px right from banner edge
  // Banner right edge at X=650, circle moved 20px right = 670
  // Circle: 120x120px (radius 60), center 100px from bottom (Y=380)
  const int BANNER_RIGHT_X = 250 + 400;            // = 650 (banner right edge)
  const int CIRCLE_CENTER_X = BANNER_RIGHT_X + 20; // Moved 20px to the right
  const int CIRCLE_CENTER_Y = 480 - 100;           // 100px from bottom = 380
  const int CIRCLE_RADIUS = 60;                    // 120px diameter
  const int BORDER_THICKNESS = 5;                  // Same as main border

  // Draw white filled circle with thick black border (5px)
  _display.fillCircle(CIRCLE_CENTER_X, CIRCLE_CENTER_Y, CIRCLE_RADIUS, GxEPD_WHITE);
  for (int i = 0; i < BORDER_THICKNESS; i++)
  {
    _display.drawCircle(CIRCLE_CENTER_X, CIRCLE_CENTER_Y, CIRCLE_RADIUS - i, GxEPD_BLACK);
  }

  // Extract icon from sprite sheet (80x80px)
  uint8_t iconBuffer[800]; // 80x80 pixels / 8 bits = 800 bytes
  extractIcon(categoryIndex, iconBuffer);

  // Center 80x80 icon in the circle
  const int ICON_X = CIRCLE_CENTER_X - CATEGORY_ICON_WIDTH / 2;
  const int ICON_Y = CIRCLE_CENTER_Y - CATEGORY_ICON_HEIGHT / 2;

  // Draw the category icon (inverted to correct colors)
  _display.drawInvertedBitmap(ICON_X, ICON_Y, iconBuffer, CATEGORY_ICON_WIDTH, CATEGORY_ICON_HEIGHT, GxEPD_BLACK);
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
