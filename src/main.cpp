#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <SPI.h>

#include "config.h"
#include "BatteryMonitor.h"
#include "ButtonHandler.h"
#include "DisplayManager.h"
#include "PowerManager.h"
#include "SDCardManager.h"
#include "Lexend_Bold24pt7b.h"
#include "Lexend_Light40pt7b.h"
#include "Utf8GfxHelper.h"
#include "Questions.h"

// Current question state
int currentQuestionIndex = 0;

// Global objects
static BatteryMonitor g_battery(BAT_GPIO0);
static ButtonHandler g_buttonHandler;
static SDCardManager g_sdManager;

static int rawBat = 0;

// GxEPD2 display - Using GxEPD2_426_GDEQ0426T82
// Note: XteinkX4 has 4.26" 800x480 display
GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> display(
    GxEPD2_426_GDEQ0426T82(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

// Global managers
static DisplayManager g_displayManager(display);
static PowerManager g_powerManager(&g_displayManager);

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

  Serial.println("\n=================================");
  Serial.println("  xteink x4 sample");
  Serial.println("=================================");
  Serial.println();

  // Initialize button handler
  g_buttonHandler.begin();

  // Initialize battery pin
  pinMode(BAT_GPIO0, INPUT);

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

  // Draw initial question screen
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    // Draw inner rounded rectangle for question area (3px thick border)
    for (int i = 0; i < 3; i++)
    {
      display.drawRoundRect(50 + i, 50 + i, 700 - i * 2, 320 - i * 2, 20, GxEPD_BLACK);
    }

    // Display question text centered in rectangle (50,50,700,320)
    // Rectangle center: (400, 210)
    const char *questionText = getQuestionText(currentQuestionIndex);
    drawUtf8MultiLineCentered(display, &Lexend_Light40pt7b, questionText, 400, 210, GxEPD_BLACK);

    // Draw category banner at bottom (inverted colors)
    display.fillRoundRect(250, 400, 300, 50, 10, GxEPD_BLACK);

    const char *categoryText = getQuestionCategory(currentQuestionIndex);
    drawUtf8StringCentered(display, &Lexend_Bold24pt7b, categoryText, 400, 435, GxEPD_WHITE);

  } while (display.nextPage());
  display.hibernate();
  Serial.println("Question displayed");

  // Start display task for sleep screen with logo
  g_displayManager.setCurrentButton(NONE);
  g_displayManager.setDisplayCommand(DISPLAY_INITIAL);
  g_displayManager.startDisplayTask();
  Serial.println("Display task created");

  Serial.println("Setup complete!\n");
}

#ifdef DEBUG_IO
void debugIO()
{
  // Log raw analog levels of BTN1 and BTN2 not more often than once per second
  rawBat = analogRead(BAT_GPIO0);
  int rawBtn1 = analogRead(BTN_GPIO1);
  int rawBtn2 = analogRead(BTN_GPIO2);
  int rawBtn3 = digitalRead(BTN_GPIO3);
  Serial.print("ADC BTN1=");
  Serial.print(rawBtn1);
  Serial.print("    BTN2=");
  Serial.print(rawBtn2);
  Serial.print("    BTN3=");
  Serial.print(rawBtn3);
  Serial.println("");

  // log battery info
  bool charging = digitalRead(UART0_RXD) == HIGH;
  Serial.printf("== Battery (charging: %s) ==\n", charging ? "yes" : "no");
  Serial.print("Value from pin (raw/calibrated): ");
  Serial.print(rawBat);
  Serial.print(" / ");
  Serial.println(BatteryMonitor::millivoltsFromRawAdc(rawBat));
  Serial.print("Volts: ");
  Serial.println(g_battery.readVolts());
  Serial.print("Charge level: ");
  Serial.println(g_battery.readPercentage());
  Serial.println("");

  // SD card
}
#endif

void loop()
{
  Button currentButton = g_buttonHandler.getPressedButton();
  Button lastButton = g_buttonHandler.getLastButton();

  // Detect button press (transition from NONE to a button)
  if (currentButton != NONE && lastButton == NONE)
  {
    Serial.print("Button: ");
    Serial.println(g_buttonHandler.getButtonName(currentButton));

    // Handle navigation
    bool needsRedraw = false;

    if (currentButton == RIGHT)
    {
      // Next question
      currentQuestionIndex = (currentQuestionIndex + 1) % getQuestionCount();
      needsRedraw = true;
      Serial.printf("Next question: %d\n", currentQuestionIndex);
    }
    else if (currentButton == LEFT)
    {
      // Previous question
      currentQuestionIndex = (currentQuestionIndex - 1 + getQuestionCount()) % getQuestionCount();
      needsRedraw = true;
      Serial.printf("Previous question: %d\n", currentQuestionIndex);
    }
    else if (currentButton == CONFIRM)
    {
      // Random question
      currentQuestionIndex = random(0, getQuestionCount());
      needsRedraw = true;
      Serial.printf("Random question: %d\n", currentQuestionIndex);
    }

    // Redraw display if needed
    if (needsRedraw)
    {
      display.setFullWindow();
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);

        // Draw rounded rectangle border (3px thick)
        for (int i = 0; i < 3; i++)
        {
          display.drawRoundRect(50 + i, 50 + i, 700 - i * 2, 320 - i * 2, 20, GxEPD_BLACK);
        }

        // Display question centered in rectangle (50,50,700,320)
        // Rectangle center: (400, 210)
        const char *questionText = getQuestionText(currentQuestionIndex);
        drawUtf8MultiLineCentered(display, &Lexend_Light40pt7b, questionText, 400, 210, GxEPD_BLACK);

        // Draw category banner (inverted)
        display.fillRoundRect(250, 400, 300, 50, 10, GxEPD_BLACK);

        const char *categoryText = getQuestionCategory(currentQuestionIndex);
        drawUtf8StringCentered(display, &Lexend_Bold24pt7b, categoryText, 400, 435, GxEPD_WHITE);

      } while (display.nextPage());
      display.hibernate();
    }

    // COMMENTED OUT: Old display update logic
    // g_displayManager.setCurrentButton(currentButton);
    // g_displayManager.setDisplayCommand(DISPLAY_TEXT);

#ifdef DEBUG_IO
    debugIO();
#endif

    if (currentButton == POWER)
    {
      unsigned long startTime = millis();
      // Wait for button release
      while (digitalRead(BTN_GPIO3) == LOW)
        delay(50);

      unsigned long currentTime = millis();
      // Power button long pressed => go to sleep
      if (currentTime - startTime > POWER_BUTTON_SLEEP_MS)
        g_powerManager.enterDeepSleep();
    }
  }

  g_buttonHandler.setLastButton(currentButton);

  delay(50);
}
