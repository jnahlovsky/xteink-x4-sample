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
#include "logo.h"

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

// Category tracking for selective refresh (global scope for initialization in setup)
static char lastCategory[32] = "";

// Display helper functions
void drawBorder()
{
  // Draw 5px thick rounded rectangle border
  for (int i = 0; i < 5; i++)
  {
    display.drawRoundRect(50 + i, 50 + i, 700 - i * 2, 320 - i * 2, 20, GxEPD_BLACK);
  }
}

void drawQuestionText(const char *text)
{
  // Display question centered in rectangle (50,50,700,320)
  // Rectangle center: (400, 210)
  // Inner box is 700x320, with padding we use 640x280 for text
  drawUtf8MultiLineWrapped(display, &Lexend_Light40pt7b, text, 400, 210, 640, 280, GxEPD_BLACK);
}

void drawCategoryBanner(const char *category)
{
  // Draw category banner at bottom (inverted colors)
  display.fillRoundRect(250, 400, 300, 50, 10, GxEPD_BLACK);
  drawUtf8StringCentered(display, &Lexend_Bold24pt7b, category, 400, 435, GxEPD_WHITE);
}

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

  // Draw initial question screen with full refresh (includes border)
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    drawBorder();

    const char *questionText = getQuestionText(currentQuestionIndex);
    drawQuestionText(questionText);

    const char *categoryText = getQuestionCategory(currentQuestionIndex);
    drawCategoryBanner(categoryText);

  } while (display.nextPage());
  display.hibernate();
  Serial.println("Question displayed");

  // Initialize category tracking for partial refresh optimization
  strncpy(lastCategory, getQuestionCategory(currentQuestionIndex), 31);
  lastCategory[31] = '\0';

  // DisplayManager task not used - using direct synchronous rendering for button-driven navigation
  // This approach is simpler, more predictable, and saves ~4KB RAM
  // g_displayManager.setCurrentButton(NONE);
  // g_displayManager.setDisplayCommand(DISPLAY_INITIAL);
  // g_displayManager.startDisplayTask();
  // Serial.println("Display task created");

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
  // Debounce tracking
  static unsigned long debounceEndTime = 0;
  static int refreshCount = 0;

  // Check if still in debounce window
  if (millis() < debounceEndTime)
  {
    // Silently ignore button presses during debounce
    g_buttonHandler.setLastButton(g_buttonHandler.getPressedButton());
    delay(50);
    return;
  }

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
    else if (currentButton == VOLUME_UP)
    {
      // Next category - find next question in different category
      const char *currentCat = getQuestionCategory(currentQuestionIndex);
      int nextIndex = (currentQuestionIndex + 1) % getQuestionCount();
      while (strcmp(getQuestionCategory(nextIndex), currentCat) == 0 && nextIndex != currentQuestionIndex)
      {
        nextIndex = (nextIndex + 1) % getQuestionCount();
      }
      currentQuestionIndex = nextIndex;
      needsRedraw = true;
      Serial.printf("Next category question: %d\n", currentQuestionIndex);
    }
    else if (currentButton == VOLUME_DOWN)
    {
      // Previous category - find previous question in different category
      const char *currentCat = getQuestionCategory(currentQuestionIndex);
      int prevIndex = (currentQuestionIndex - 1 + getQuestionCount()) % getQuestionCount();
      while (strcmp(getQuestionCategory(prevIndex), currentCat) == 0 && prevIndex != currentQuestionIndex)
      {
        prevIndex = (prevIndex - 1 + getQuestionCount()) % getQuestionCount();
      }
      currentQuestionIndex = prevIndex;
      needsRedraw = true;
      Serial.printf("Previous category question: %d\n", currentQuestionIndex);
    }

    // Redraw display if needed
    if (needsRedraw)
    {
      const char *currentCategory = getQuestionCategory(currentQuestionIndex);
      bool categoryChanged = (strcmp(lastCategory, currentCategory) != 0);

      // Determine if we need full refresh
      bool useFullRefresh = false;
      if (FULL_REFRESH_INTERVAL >= 0) // -1 means never do periodic full refresh
      {
        refreshCount++;
        if (FULL_REFRESH_INTERVAL == 0 || refreshCount >= FULL_REFRESH_INTERVAL)
        {
          useFullRefresh = true;
          refreshCount = 0;
          Serial.println("Periodic full refresh");
        }
      }

      if (useFullRefresh)
      {
        // Full refresh - redraw everything including border
        display.setFullWindow();
        display.firstPage();
        do
        {
          display.fillScreen(GxEPD_WHITE);
          drawBorder();
          drawQuestionText(getQuestionText(currentQuestionIndex));
          drawCategoryBanner(currentCategory);
        } while (display.nextPage());
        display.hibernate();

        debounceEndTime = millis() + BUTTON_DEBOUNCE_FULL_MS;
      }
      else if (categoryChanged)
      {
        // Category changed - refresh both question and banner
        Serial.println("Partial refresh (dual-region): question + banner");

        // Single partial window covering both regions - redraw border to fix any erasure
        display.setPartialWindow(60, 60, 675, 395);
        display.firstPage();
        do
        {
          display.fillScreen(GxEPD_WHITE);

          // Redraw border in case it gets affected by the partial window
          drawBorder();

          drawQuestionText(getQuestionText(currentQuestionIndex));
          drawCategoryBanner(currentCategory);
        } while (display.nextPage());
        display.hibernate();

        debounceEndTime = millis() + BUTTON_DEBOUNCE_PARTIAL_DUAL_MS;

        // Update category tracking
        strncpy(lastCategory, currentCategory, 31);
        lastCategory[31] = '\0';
      }
      else
      {
        // Same category - only refresh question area
        Serial.println("Partial refresh (single-region): question only");

        // Partial window with 10px margin from 5px border, stops before bottom border
        // Border is at (50, 50, 700, 320), inner edge at (55, 55) to (745, 365)
        // Partial window: 60 to 735 horizontally, 60 to 355 vertically (5px margin from bottom border)
        display.setPartialWindow(60, 60, 675, 295);
        display.firstPage();
        do
        {
          display.fillScreen(GxEPD_WHITE);
          drawQuestionText(getQuestionText(currentQuestionIndex));
        } while (display.nextPage());
        display.hibernate();

        debounceEndTime = millis() + BUTTON_DEBOUNCE_PARTIAL_SINGLE_MS;
      }
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
      {
        // Display sleep screen with logo
        Serial.println("Displaying sleep screen...");
        display.setFullWindow();
        display.firstPage();
        do
        {
          display.fillScreen(GxEPD_WHITE);
          display.drawBitmap(0, 0, logo, 800, 480, GxEPD_BLACK);
        } while (display.nextPage());
        display.hibernate();

        Serial.println("Entering deep sleep...");
        delay(1000);

        // Enter deep sleep (without DisplayManager command)
        esp_deep_sleep_enable_gpio_wakeup(1ULL << BTN_GPIO3, ESP_GPIO_WAKEUP_GPIO_LOW);
        esp_deep_sleep_start();
      }
    }
  }

  g_buttonHandler.setLastButton(currentButton);

  delay(50);
}
