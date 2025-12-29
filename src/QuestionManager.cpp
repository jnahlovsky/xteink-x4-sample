// QuestionManager.cpp
#include "QuestionManager.h"
#include "config.h"

QuestionManager::QuestionManager(
    GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> &display,
    UIRenderer &uiRenderer,
    BatteryMonitor &battery,
    BatteryDisplay &batteryDisplay,
    StateManager &stateManager)
    : _display(display),
      _uiRenderer(uiRenderer),
      _battery(battery),
      _batteryDisplay(batteryDisplay),
      _stateManager(stateManager),
      _currentIndex(0),
      _refreshCount(0),
      _debounceEndTime(0)
{
  _lastCategory[0] = '\0';
}

void QuestionManager::begin()
{
  _currentIndex = _stateManager.loadLastCardIndex(getQuestionCount());

  // Initialize category tracking
  strncpy(_lastCategory, getQuestionCategory(_currentIndex), 31);
  _lastCategory[31] = '\0';
}

void QuestionManager::displayInitialQuestion()
{
  const char *questionText = getQuestionText(_currentIndex);
  const char *categoryText = getQuestionCategory(_currentIndex);

  _display.setFullWindow();
  _display.firstPage();
  do
  {
    _display.fillScreen(GxEPD_WHITE);

    // Layer 1: Border
    _uiRenderer.drawBorder();

    // Layer 2: Card number (top-left)
    _uiRenderer.drawCardNumber(_currentIndex, getQuestionCount());

    // Layer 3: Question text (center)
    _uiRenderer.drawQuestionText(questionText);

    // Layer 4: Parrot with cutout effect (bottom-left)
    _uiRenderer.drawParrotWithCutout();

    // Layer 5: Category banner overlay (bottom, over border)
    _uiRenderer.drawCategoryBanner(categoryText);

    // Layer 6: Category icon in circle (right side of banner)
    _uiRenderer.drawCategoryIconInCircle();

    // Layer 7: Battery (top-right)
    int batteryPercent = _battery.readPercentage();
    bool isCharging = digitalRead(UART0_RXD) == HIGH;

    const int BATTERY_ICON_RIGHT_X = 780;
    const int TEXT_ICON_GAP = 5;
    _uiRenderer.drawBatteryIcon(BATTERY_ICON_RIGHT_X, 17, batteryPercent);

    if (isCharging)
    {
      char percentText[8];
      int displayPercent = (batteryPercent >= 95) ? 100 : batteryPercent;
      snprintf(percentText, sizeof(percentText), "%d%%", displayPercent);

      int16_t x1, y1;
      uint16_t w, h;
      _display.getTextBounds(percentText, 0, 0, &x1, &y1, &w, &h);
      int textX = BATTERY_ICON_RIGHT_X - 28 - TEXT_ICON_GAP - w;
      _uiRenderer.drawBatteryStatus(percentText, textX, 30);
    }
  } while (_display.nextPage());
  _display.hibernate();

  Serial.println("Question displayed");
}

void QuestionManager::nextQuestion()
{
  _currentIndex = (_currentIndex + 1) % getQuestionCount();
  Serial.printf("Next question: %d\n", _currentIndex);
}

void QuestionManager::previousQuestion()
{
  _currentIndex = (_currentIndex - 1 + getQuestionCount()) % getQuestionCount();
  Serial.printf("Previous question: %d\n", _currentIndex);
}

void QuestionManager::randomQuestion()
{
  _currentIndex = random(0, getQuestionCount());
  Serial.printf("Random question: %d\n", _currentIndex);
}

void QuestionManager::nextCategory()
{
  const char *currentCat = getQuestionCategory(_currentIndex);
  int nextIndex = (_currentIndex + 1) % getQuestionCount();
  while (strcmp(getQuestionCategory(nextIndex), currentCat) == 0 && nextIndex != _currentIndex)
  {
    nextIndex = (nextIndex + 1) % getQuestionCount();
  }
  _currentIndex = nextIndex;
  Serial.printf("Next category question: %d\n", _currentIndex);
}

void QuestionManager::previousCategory()
{
  const char *currentCat = getQuestionCategory(_currentIndex);
  int prevIndex = (_currentIndex - 1 + getQuestionCount()) % getQuestionCount();
  while (strcmp(getQuestionCategory(prevIndex), currentCat) == 0 && prevIndex != _currentIndex)
  {
    prevIndex = (prevIndex - 1 + getQuestionCount()) % getQuestionCount();
  }
  _currentIndex = prevIndex;
  Serial.printf("Previous category question: %d\n", _currentIndex);
}

const char *QuestionManager::getCurrentCategory() const
{
  return getQuestionCategory(_currentIndex);
}

void QuestionManager::drawQuestionWithBattery(const char *questionText, const char *category)
{
  _uiRenderer.drawQuestionText(questionText);
  _uiRenderer.drawCategoryBanner(category);

  // Draw battery with percentage if charging
  int batteryPercent = _battery.readPercentage();
  bool isCharging = digitalRead(UART0_RXD) == HIGH;

  const int BATTERY_ICON_RIGHT_X = 780;
  const int TEXT_ICON_GAP = 5;
  _uiRenderer.drawBatteryIcon(BATTERY_ICON_RIGHT_X, 17, batteryPercent);

  if (isCharging)
  {
    char percentText[8];
    int displayPercent = (batteryPercent >= 95) ? 100 : batteryPercent;
    snprintf(percentText, sizeof(percentText), "%d%%", displayPercent);

    int16_t x1, y1;
    uint16_t w, h;
    _display.getTextBounds(percentText, 0, 0, &x1, &y1, &w, &h);
    int textX = BATTERY_ICON_RIGHT_X - 28 - TEXT_ICON_GAP - w;
    _uiRenderer.drawBatteryStatus(percentText, textX, 30);
  }
}

bool QuestionManager::shouldUseFullRefresh()
{
  if (FULL_REFRESH_INTERVAL < 0) // -1 means never do periodic full refresh
    return false;

  _refreshCount++;
  if (FULL_REFRESH_INTERVAL == 0 || _refreshCount >= FULL_REFRESH_INTERVAL)
  {
    _refreshCount = 0;
    Serial.println("Periodic full refresh");
    return true;
  }
  return false;
}

bool QuestionManager::hasCategoryChanged()
{
  return (strcmp(_lastCategory, getCurrentCategory()) != 0);
}

void QuestionManager::updateCategoryTracking()
{
  strncpy(_lastCategory, getCurrentCategory(), 31);
  _lastCategory[31] = '\0';
}

unsigned long QuestionManager::getDebounceTime(bool fullRefresh, bool categoryChanged)
{
  if (fullRefresh)
    return BUTTON_DEBOUNCE_FULL_MS;
  else if (categoryChanged)
    return BUTTON_DEBOUNCE_PARTIAL_DUAL_MS;
  else
    return BUTTON_DEBOUNCE_PARTIAL_SINGLE_MS;
}

void QuestionManager::updateDisplay()
{
  const char *currentCategory = getCurrentCategory();
  bool categoryChanged = hasCategoryChanged();
  bool useFullRefresh = shouldUseFullRefresh();

  if (useFullRefresh)
  {
    // Full refresh - redraw everything including border
    _display.setFullWindow();
    _display.firstPage();
    do
    {
      _display.fillScreen(GxEPD_WHITE);

      // Layer 1: Border
      _uiRenderer.drawBorder();

      // Layer 2: Card number
      _uiRenderer.drawCardNumber(_currentIndex, getQuestionCount());

      // Layer 3-7: Question + battery
      drawQuestionWithBattery(getQuestionText(_currentIndex), currentCategory);

      // Layer 4: Parrot with cutout
      _uiRenderer.drawParrotWithCutout();

      // Layer 5: Category banner overlay
      _uiRenderer.drawCategoryBanner(currentCategory);

      // Layer 6: Category icon
      _uiRenderer.drawCategoryIconInCircle();
    } while (_display.nextPage());
    _display.hibernate();

    _debounceEndTime = millis() + getDebounceTime(true, false);
  }
  else if (categoryChanged)
  {
    // Category changed - refresh both question and banner
    Serial.println("Partial refresh (dual-region): question + banner + battery");

    _display.setPartialWindow(10, 10, 740, 445);
    _display.firstPage();
    do
    {
      _display.fillScreen(GxEPD_WHITE);
      _uiRenderer.drawBorder();
      _uiRenderer.drawCardNumber(_currentIndex, getQuestionCount());
      drawQuestionWithBattery(getQuestionText(_currentIndex), currentCategory);
      _uiRenderer.drawParrotWithCutout();
      _uiRenderer.drawCategoryBanner(currentCategory);
      _uiRenderer.drawCategoryIconInCircle();
    } while (_display.nextPage());
    _display.hibernate();

    _debounceEndTime = millis() + getDebounceTime(false, true);
    updateCategoryTracking();
  }
  else
  {
    // Same category - refresh question area + bottom elements
    Serial.println("Partial refresh (single-region): question + bottom elements");

    _display.setPartialWindow(10, 10, 740, 445);
    _display.firstPage();
    do
    {
      _display.fillScreen(GxEPD_WHITE);
      _uiRenderer.drawBorder();
      _uiRenderer.drawCardNumber(_currentIndex, getQuestionCount());
      _uiRenderer.drawQuestionText(getQuestionText(_currentIndex));
      _uiRenderer.drawParrotWithCutout();
      _uiRenderer.drawCategoryBanner(currentCategory);
      _uiRenderer.drawCategoryIconInCircle();
    } while (_display.nextPage());
    _display.hibernate();

    _debounceEndTime = millis() + getDebounceTime(false, false);
  }
}

void QuestionManager::saveState()
{
  _stateManager.saveCardIndex(_currentIndex);
}
