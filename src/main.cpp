#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <SPI.h>

#include "config.h"
#include "BatteryMonitor.h"
#include "BatteryDisplay.h"
#include "ButtonHandler.h"
#include "DisplayManager.h"
#include "PowerManager.h"
#include "SDCardManager.h"
#include "StateManager.h"
#include "UIRenderer.h"
#include "QuestionManager.h"
#include "SleepManager.h"
#include "NavigationController.h"

#ifdef DEBUG_IO
#include "DebugHelper.h"
#endif

// Global objects
static BatteryMonitor g_battery(BAT_GPIO0);
static ButtonHandler g_buttonHandler;
static SDCardManager g_sdManager;
static StateManager g_stateManager;

// GxEPD2 display - Using GxEPD2_426_GDEQ0426T82
// Note: Xteink X4 has 4.26" 800x480 display
GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> display(
    GxEPD2_426_GDEQ0426T82(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

// Global managers
static DisplayManager g_displayManager(display);
static PowerManager g_powerManager(&g_displayManager);
static UIRenderer g_uiRenderer(display);
static BatteryDisplay g_batteryDisplay(g_battery, g_uiRenderer);
static QuestionManager g_questionManager(display, g_uiRenderer, g_battery, g_batteryDisplay, g_stateManager);
static SleepManager g_sleepManager(display, g_questionManager);

#ifdef DEBUG_IO
static DebugHelper g_debugHelper(g_battery);
static NavigationController g_navigationController(g_buttonHandler, g_questionManager, g_sleepManager, g_debugHelper);
#else
static NavigationController g_navigationController(g_buttonHandler, g_questionManager, g_sleepManager);
#endif

void setup()
{
  // Check if boot was triggered by the Power Button (Deep Sleep Wakeup)
  // If triggered by RST pin or Battery insertion, this will be false, allowing normal boot.
  if (g_powerManager.wasWokenByGpio())
  {
    g_powerManager.verifyWakeupLongPress();
  }

  Serial.begin(115200);

  // Wait for serial monitor
  unsigned long start = millis();
  while (!Serial && (millis() - start) < 3000)
  {
    delay(10);
  }

  if (Serial)
  {
    // delay for monitor to start reading
    delay(1000);
  }

  // Initialize button handler
  g_buttonHandler.begin();

  // Initialize battery pin
  pinMode(BAT_GPIO0, INPUT);

  // Initialize state manager
  g_stateManager.begin();

  // Initialize display
  g_displayManager.begin();
  g_displayManager.setBatteryMonitor(&g_battery);
  Serial.println("Display initialized");

  // Ensure landscape orientation (rotation already set in DisplayManager)
  Serial.printf("Display size: %d x %d\n", display.width(), display.height());

  // SD Card Initialization
  if (g_sdManager.begin())
  {
    Serial.println("\n SD card detected");
  }
  else
  {
    Serial.println("\n SD card not detected");
  }

  // Initialize question manager and display initial question
  g_questionManager.begin();
  g_questionManager.displayInitialQuestion();

  Serial.println("Setup complete!\n");
}

void loop()
{
  delay(50); // Debounce delay

  g_navigationController.processInput();
  g_batteryDisplay.checkAndUpdate();
}
