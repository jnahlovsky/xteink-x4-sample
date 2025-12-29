# Question Card Game

An interactive question card game for the Xteink X4 e-ink device. Navigate through 320+ thought-provoking questions across 8 different categories using physical buttons. Features a clean UI with custom Lexend typography and category icons.


## Hardware

- **Device**: [Xteink X4](https://www.xteink.com/products/xteink-x4)
- **Board**: ESP32-C3 (QFN32)
- **Flash**: 16MB (SPI), 6.5MB app0 / app1 partitions + spiffs
- **RAM**: 400KB (327680 bytes usable with PlatformIO, no PSRAM)
- **Display**: [4.26" E-Ink (800×480px, GDEQ0426T82, SSD1677 controller)](https://www.good-display.com/product/457.html) (220PPI)
- **Custom SPI pins**: SCLK=8, MOSI=10, CS=21, DC=4, RST=5, BUSY=6
- **Battery**: 650mAh
- **Storage**: microSD card slot

### Resources

- [Hardware Schematics](https://github.com/sunwoods/Xteink-X4) - Xteink X4 circuit diagrams
- [Display Datasheet](https://www.good-display.com/product/457.html) - 4.26" GDEQ0426T82 specifications

## Features

- 320+ questions across 8 categories (Emotions, Fantasy, Relationships, My Body, School of Life, Nature, What If, Hidden Corners)
- Interactive navigation with physical buttons
- Clean UI with Lexend font family
- Category icons and themed display
- Battery level indicator
- Deep sleep power management
- Auto-conversion of CSV questions to C++ header
- Progress tracking and random question selection

## Building

```powershell
# Build
platformio run

# Upload
platformio run -t upload

# Monitor
platformio device monitor
```

## Firmware Backup & Restore

### Backup Original Firmware

Before flashing custom firmware, back up the factory firmware:

```powershell
# Read entire 16MB flash
python -m esptool --chip esp32c3 --port COM6 read_flash 0x0 0x1000000 backup/firmware_backup.bin
```

```powershell
# Read only app0 (faster)
python -m esptool --chip esp32c3 --port COM6 read_flash 0x10000 0x640000 backup/app0_backup.bin
```

### Restore Original Firmware

To restore the backed-up firmware:

```powershell
# Write back the entire flash
python -m esptool --chip esp32c3 --port COM6 write_flash 0x0 backup/firmware_backup.bin
```

```powershell
# Write back only app0 (faster)
python -m esptool --chip esp32c3 --port COM6 write_flash 0x10000 backup/app0_backup.bin
```

**Important**: Make sure to use the correct COM port for your device.

### Switching Boot Partitions (app0/app1)

```powershell
# Backup current OTA data first
python -m esptool --port COM6 read_flash 0xE000 0x2000 backup/otadata_backup.bin

# Flash to switch boot partition
# Boot app0
python -m esptool --port COM6 write_flash 0xE000 backup/otadata_boot_app0.bin

# Boot app1
python -m esptool --port COM6 write_flash 0xE000 backup/otadata_boot_app1.bin
```

### Generating custom fonts with diacritics

```powershell
python src/ttf_to_gfx.py src/Lexend-Bold.ttf 32 32 383 src/Lexend_Bold32pt7b.h
```

## Notes

- This uses `GxEPD2_426_GDEQ0426T82` as the display class for the 4.26" 800x480 display
- Display rotation is set to 3 (270 degrees)
- Partial refresh is used for button presses to improve responsiveness

## Tasks

- [x] Drive screen
- [x] Read buttons
- [x] Wakeup and deep sleep
- [x] Read battery percentage
- [ ] Better rendering with grayscale support
- [x] SD card reader
- [ ] WiFi
- [ ] Bluetooth

## Button System

The device uses **resistor ladder networks** connected to two ADC pins for button detection. Each button press produces a unique analog voltage that's read via `analogRead()`.

### Button ADC Values

**GPIO1 (4 buttons)**:

- Back: ~3470
- Confirm: ~2655
- Left: ~1470
- Right: ~3

**GPIO2 (2 buttons)**:

- Volume Up: ~2205
- Volume Down: ~3

### Power Button

**GPIO3**:

- Pressed: LOW
- 1-second long press to enter sleep mode
- 1-second long press to wake from sleep

### Battery Voltage

- GPIO0 is connected to the battery via a voltage divider (2x10K resistors), reading 1/2 of the actual voltage
- UART0_RXD/GPIO20 can be used to detect USB connection (charging or not)

### microSD (TF) Card

SD SPI bus is shared with EPD, GPIO12 is used for CS (SS).

```
CS  (SS)    -> IO12
DO  (MISO)  -> IO7
DI  (MOSI)  -> IO10
SCK (SCLK)  -> IO8
```

### Implementation Notes

- Use threshold ranges (e.g., `value > 3200 && value < 3700`) to detect button presses
- Add debouncing with edge detection (track last button state) to prevent multiple triggers
- Polling every 50ms works well for responsive input
- The resistor ladder allows multiple buttons on a single ADC pin, saving GPIO pins
