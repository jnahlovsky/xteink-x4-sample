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

  // Setup display properties - Try rotation 0 for GDEQ0426T82
  display.setRotation(0); // No rotation, native landscape
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
      // Use full window for sleep screen with logo only
      display.setFullWindow();
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);

        // Draw fullscreen logo image
        display.drawBitmap(0, 0, logo, 800, 480, GxEPD_BLACK);
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

        // Draw fullscreen logo image
        display.drawBitmap(0, 0, logo, 800, 480, GxEPD_BLACK);
      } while (display.nextPage());
    }
  }
}
