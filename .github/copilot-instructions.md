# Xteink X4 Question Display - Copilot Instructions

## Project Overview

This is a PlatformIO-based custom firmware project for the **Xteink X4** e-ink device that displays interactive questions. Built on ESP32-C3 using the Arduino framework, this application shows randomly selected questions from various categories, with navigation via physical buttons.

### Application Purpose

The device displays thought-provoking questions in a clean, readable interface using the **Lexend** custom font. Users can:
- Navigate through 320+ questions across multiple categories
- Jump to random questions
- Browse different question categories
- See category names with custom icons
- Track progress through question sets

This firmware is based on an open-source starter repository for the Xteink X4 hardware, with completely custom application logic for the question display system.

**See [TODO.md](../TODO.md) for the complete development roadmap.**

## Hardware Specifications

- **MCU**: ESP32-C3 (QFN32)
- **Flash**: 16MB with 6.5MB app0/app1 partitions + SPIFFS
- **RAM**: 400KB (327680 bytes usable, NO PSRAM)
- **Display**: 4.26" E-Ink (800×480px, GDEQ0426T82, SSD1677 controller)
- **Battery**: 650mAh with voltage monitoring
- **Storage**: microSD card slot
- **Buttons**:
  - GPIO1: 4 buttons on ADC resistor ladder (Back, Confirm, Left, Right)
  - GPIO2: 2 buttons on ADC resistor ladder (Volume Up, Volume Down)
  - GPIO3: Power button (digital)

### Custom SPI Pin Configuration

**IMPORTANT**: This device uses CUSTOM SPI pins, NOT the default ESP32-C3 hardware SPI pins:
- SCLK: GPIO8
- MOSI: GPIO10
- CS: GPIO21
- DC: GPIO4
- RST: GPIO5
- BUSY: GPIO6
- SD Card CS: GPIO12
- SD Card MISO: GPIO7

## Project Architecture

### Application Flow

```
Boot → Load Questions → Display Random Question → Wait for Input
         ↑                                              ↓
         └──────────── Navigation Loop ────────────────┘

Inputs: Next/Prev Question, Random, Next/Prev Category, Menu
```

### Module Structure

The codebase follows a modular component architecture with hardware abstraction layers:

1. **BatteryMonitor** (`BatteryMonitor.cpp/h`)
   - Monitors battery voltage via ADC on GPIO0
   - Provides battery level percentage calculation

2. **ButtonHandler** (`ButtonHandler.cpp/h`)
   - Handles button input via ADC resistor ladders (GPIO1, GPIO2)
   - Digital power button handling (GPIO3)
   - Debouncing and threshold-based button detection

3. **DisplayManager** (`DisplayManager.cpp/h`)
   - Manages GxEPD2 e-ink display operations
   - Handles display initialization, rendering, and updates
   - Display is 800×480 pixels, 4.26" GDEQ0426T82

4. **PowerManager** (`PowerManager.cpp/h`)
   - Deep sleep and wake management
   - Power button long-press detection for sleep/wake
   - USB connection detection via UART0_RXD (GPIO20)

5. **SDCardManager** (`SDCardManager.cpp/h`)
   - MicroSD card initialization and file operations
   - Uses custom SPI pins (CS: GPIO12, MISO: GPIO7)

### Configuration

All hardware configuration constants are in [`src/config.h`](src/config.h):
- Pin definitions
- ADC button thresholds
- Power button timing constants
- SPI frequency settings

## Development Guidelines

### Memory Constraints

**CRITICAL**: This device has only 400KB of usable RAM and NO PSRAM. Always:
- Avoid large dynamic allocations
- Use `PROGMEM` for large constant data (images, fonts)
- Be mindful of stack usage
- Monitor heap fragmentation
- Prefer static allocation where possible

### Display Programming

- Use `GxEPD2_BW` library (black & white e-ink)
- Display controller: GxEPD2_426_GDEQ0426T82
- **Orientation**: Landscape mode only (800×480px) for MVP
- Always use `display.hibernate()` when done to save power
- E-ink updates are slow (~2 seconds for full refresh)
- Minimize partial updates to extend display lifespan

### Button Input

- GPIO1 buttons: Right(3), Left(1470), Confirm(2655), Back(3470)
- GPIO2 buttons: Volume Down(3), Volume Up(2205)
- Use `BTN_THRESHOLD` (100) for ADC tolerance
- Power button requires long press (1000ms) for sleep/wake

**Application Button Mapping**:
- **Right**: Next question in current category
- **Left**: Previous question in current category
- **Confirm**: Random question from any category
- **Back**: Return to main menu (future feature)
- **Volume Up**: Next category
- **Volume Down**: Previous category
- **Power long press**: Sleep mode

### Power Management

- Device supports deep sleep via ESP32-C3 wake on GPIO
- Power button (GPIO3) can wake device from deep sleep
- USB connection detection available on GPIO20
- Always verify wake source with `g_powerManager.wasWokenByGpio()`

### Build System

- **Platform**: PlatformIO with Arduino framework
- **Board**: esp32-c3-devkitm-1
- **Upload speed**: 921600 baud
- **Monitor speed**: 115200 baud
- **Partition**: Custom 16MB layout (`default_16MB.csv`)

### Code Style

- Use C++11/14 standards
- Prefer composition over inheritance
- Keep manager classes stateless where possible
- Initialize hardware in `setup()`, not constructors
- Use global static objects for managers (declared in `main.cpp`)
- Store large data (fonts, images, questions) in PROGMEM or SPIFFS
- Use descriptive variable names for question/category state

### Application-Specific Guidelines

**Question Data Management**:
- Store questions in JSON format on SPIFFS or SD card
- Load questions lazily to conserve RAM (only current question in memory)
- Use `ArduinoJson` library for parsing (efficient for embedded systems)

**Font Usage**:
- Use Lexend font family for all question text
- Convert fonts using Adafruit fontconvert tool
- Store font data in PROGMEM header files
- Sizes: 16pt (UI elements), 24pt (questions), 32pt (headers)

**UI Rendering** (based on physical card design):
- Draw rounded rectangle border around question area
- Center-align question text within the bordered area
- Implement text wrapping for long questions (multi-line support)
- Display bird icon at bottom left corner (category mascot)
- Draw category name in a filled rounded rectangle banner at bottom
- Display category icon (door/symbol) at bottom right of banner
- Show card number vertically on left edge (e.g., "002.350")
- Add small battery indicator at very bottom center
- Clear display between question changes (prevent ghosting)
- Use `display.setFont()` to switch between Lexend sizes

## Common Patterns

### Display Update Pattern
```cpp
g_displayManager.begin();
display.setFullWindow();
display.firstPage();
do {
  // Draw content here
  display.setFont(&LexendRegular24pt);
  display.setCursor(x, y);
  display.print("Question text");
} while (display.nextPage());
display.hibernate();
```

### Question Display Pattern
```cpp
// Load current question
String questionText = loadQuestion(currentCategory, currentIndex);

// Render question on display
display.setFullWindow();
display.firstPage();
do {
  drawCategoryHeader(currentCategory);
  drawQuestionText(questionText);
  drawFooter(currentIndex, totalQuestions);
} while (display.nextPage());
display.hibernate();
```

### Button Reading Pattern
```cpp
g_buttonHandler.update();
if (g_buttonHandler.wasPressed(ButtonHandler::BUTTON_CONFIRM)) {
  // Handle confirm button
}
```

### Power Management Pattern
```cpp
if (g_powerManager.wasWokenByGpio()) {
  g_powerManager.verifyWakeupLongPress();
}
// ... normal operation ...
if (/* sleep condition */) {
  g_powerManager.enterDeepSleep();
}
```

## Key Dependencies

- **GxEPD2** v1.5.9: E-ink display driver library
- **Arduino framework**: Core ESP32-C3 support
- **ArduinoJson** (to be added): JSON parsing for question data
- **Adafruit GFX Library**: Font rendering support
- Built-in ESP32 libraries: SPI, SD, Wire, SPIFFS

## Build Commands

```bash
# Build firmware
pio run

# Upload to device
pio run -t upload

# Serial monitor
pio device monitor
```

## Important Notes

1. **NO PSRAM**: Always be memory-conscious - load questions lazily!
2. **Custom SPI pins**: Never use default ESP32 SPI pins
3. **E-ink refresh time**: ~2 seconds, plan UI accordingly (users expect delay)
4. **Power button**: Requires 1-second long press to sleep/wake
5. **ADC buttons**: Use resistor ladder values, not digital reads
6. **USB CDC**: Enabled for serial communication over USB
7. **Font storage**: Always use PROGMEM for Lexend font data
8. **Question data**: Prefer SPIFFS over hardcoded arrays for 320 questions
9. **Text wrapping**: Implement proper word-wrap for long questions (800px width)
10. **Category icons**: Keep icons simple, 1-bit black & white for e-ink

## Resources

- [Hardware Schematics](https://github.com/sunwoods/Xteink-X4)
- [Display Datasheet](https://www.good-display.com/product/457.html)
- [Discord Community](https://discord.gg/2cdKUbWRE8)
- [GxEPD2 Library](https://github.com/ZinggJM/GxEPD2)
