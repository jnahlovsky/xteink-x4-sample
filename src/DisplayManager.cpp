#include "DisplayManager.h"
#include "config.h"
#include "image.h"
#include "logo.h"
#include "SDCardManager.h"
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <SPI.h>

// Static pointer for FreeRTOS task callback
static DisplayManager *g_displayManagerInstance = nullptr;

// FreeRTOS task wrapper
void displayUpdateTaskWrapper(void *parameter)
{
  while (1)
  {
    if (g_displayManagerInstance != nullptr)
    {
      g_displayManagerInstance->updateDisplay();
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

DisplayManager::DisplayManager(GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> &disp)
    : display(disp), batteryMonitor(nullptr), displayCommand(DISPLAY_NONE),
      currentPressedButton(NONE), displayTaskHandle(NULL)
{
  g_displayManagerInstance = this;
}

void DisplayManager::begin()
{
  // Initialize SPI with custom pins
  SPI.begin(EPD_SCLK, SD_SPI_MISO, EPD_MOSI, EPD_CS);

  // Initialize display
  SPISettings spi_settings(SPI_FQ, MSBFIRST, SPI_MODE0);
  display.init(115200, true, 2, false, SPI, spi_settings);

  // Setup display properties
  display.setRotation(3); // 270 degrees
  display.setTextColor(GxEPD_BLACK);
}

void DisplayManager::startDisplayTask()
{
  // Create display update task on core 0 (main loop runs on core 1)
  xTaskCreatePinnedToCore(displayUpdateTaskWrapper, // Task function
                          "DisplayUpdate",          // Task name
                          4096,                     // Stack size
                          NULL,                     // Parameters
                          1,                        // Priority
                          &displayTaskHandle,       // Task handle
                          0                         // Core 0
  );
}

void DisplayManager::setDisplayCommand(DisplayCommand cmd)
{
  displayCommand = cmd;
}

bool DisplayManager::isCharging()
{
  // U0RXD/GPIO20 reads HIGH when USB is connected
  return digitalRead(UART0_RXD) == HIGH;
}

const char *DisplayManager::getButtonName(Button btn)
{
  switch (btn)
  {
  case NONE:
    return "Press any button";
  case RIGHT:
    return "RIGHT pressed!";
  case LEFT:
    return "LEFT pressed!";
  case CONFIRM:
    return "CONFIRM pressed!";
  case BACK:
    return "BACK pressed!";
  case VOLUME_UP:
    return "VOLUME UP pressed!";
  case VOLUME_DOWN:
    return "VOLUME DOWN pressed!";
  case POWER:
    return "POWER pressed!";
  default:
    return "";
  }
}

void DisplayManager::drawBatteryInfo()
{
  if (batteryMonitor == nullptr)
    return;

  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(20, 160);

  bool charging = isCharging();
  display.printf("Power: %s", charging ? "Charging" : "Battery");

  display.setCursor(40, 200);
  display.printf("Raw: %i", batteryMonitor->readRawMillivolts());
  display.setCursor(40, 240);
  display.printf("Volts: %.2f V", batteryMonitor->readVolts());
  display.setCursor(40, 280);
  display.printf("Charge: %i%%", batteryMonitor->readPercentage());
}

void DisplayManager::drawSdTopFiles()
{
  // Layout constants aligned with drawBatteryInfo() block
  const int startX = 40;
  const int startY = 350;
  const int lineHeight = 26;
  const int maxLines = 5;
  const int maxChars = 30;

  display.setFont(&FreeMonoBold12pt7b);

  display.setCursor(20, 320);
  display.print("Top 5 files on SD:");

  auto drawTruncated = [&](int lineIdx, const char *text)
  {
    // Render a single line, truncating with ellipsis if needed
    String s(text ? text : "");
    if ((int)s.length() > maxChars)
    {
      s.remove(maxChars - 1);
      s += "…";
    }
    display.setCursor(startX, startY + lineIdx * lineHeight);
    display.print(s);
  };

  // Use SDCardManager to get file list
  SDCardManager sdManager;
  auto files = sdManager.getTopFiles(maxLines);

  if (files.empty())
  {
    drawTruncated(0, sdManager.isReady() ? "Empty" : "No card");
    return;
  }

  for (size_t i = 0; i < files.size(); i++)
  {
    drawTruncated(i, files[i].c_str());
  }
}

void DisplayManager::updateDisplay()
{
  if (displayCommand != DISPLAY_NONE)
  {
    DisplayCommand cmd = displayCommand;
    displayCommand = DISPLAY_NONE;

    if (cmd == DISPLAY_INITIAL)
    {
      // Use full window for initial welcome screen
      display.setFullWindow();
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);

        // Header font
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(20, 50);
        display.print("Xteink X4 Sample");

        // Button text with smaller font
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(20, 100);
        display.print(getButtonName(currentPressedButton));

        // Draw battery information
        drawBatteryInfo();
        // Draw top 3 SD files below the battery block
        drawSdTopFiles();

        // Draw image at bottom right
        int16_t imgWidth = 263;
        int16_t imgHeight = 280;
        int16_t imgMargin = 20;
        int16_t imgX = 480 - imgMargin - imgWidth;
        int16_t imgY = 800 - imgMargin - imgHeight;
        display.drawBitmap(imgX, imgY, dr_mario, imgWidth, imgHeight, GxEPD_BLACK);
      } while (display.nextPage());
    }
    else if (cmd == DISPLAY_TEXT)
    {
      // Use partial refresh for text updates
      display.setPartialWindow(0, 75, display.width(), 225);
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(20, 100);
        display.print(getButtonName(currentPressedButton));
        drawBatteryInfo();
      } while (display.nextPage());
    }
    else if (cmd == DISPLAY_BATTERY)
    {
      // Use partial refresh for battery updates
      display.setPartialWindow(0, 135, display.width(), 200);
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);
        drawBatteryInfo();
      } while (display.nextPage());
    }
    else if (cmd == DISPLAY_SLEEP)
    {
      // Use full window for sleep screen
      display.setFullWindow();
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);

        // Draw image at bottom right
        int16_t imgWidth = 400;
        int16_t imgHeight = 400;
        int16_t imgX = 480 - imgWidth - 40;
        int16_t imgY = 800 - imgHeight - 200;
        display.drawBitmap(imgX, imgY, logo, imgWidth, imgHeight, GxEPD_BLACK);
      } while (display.nextPage());
    }
  }
}
