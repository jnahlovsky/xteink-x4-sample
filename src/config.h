#ifndef CONFIG_H
#define CONFIG_H

// SPI Frequency
#define SPI_FQ 40000000

// Display SPI pins (custom pins for XteinkX4, not hardware SPI defaults)
#define EPD_SCLK 8  // SPI Clock
#define EPD_MOSI 10 // SPI MOSI (Master Out Slave In)
#define EPD_CS 21   // Chip Select
#define EPD_DC 4    // Data/Command
#define EPD_RST 5   // Reset
#define EPD_BUSY 6  // Busy

// Button pins
#define BTN_GPIO1 1 // 4 buttons on ADC resistor ladder: Back, Confirm, Left, Right
#define BTN_GPIO2 2 // 2 buttons on ADC resistor ladder: Volume Up, Volume Down
#define BTN_GPIO3 3 // Power button (digital)

// Other GPIO pins
#define UART0_RXD 20 // Used for USB connection detection
#define BAT_GPIO0 0  // Battery voltage

// SD Card pins
#define SD_SPI_CS 12
#define SD_SPI_MISO 7

// Button ADC thresholds
const int BTN_THRESHOLD = 100; // Threshold tolerance
const int BTN_RIGHT_VAL = 3;
const int BTN_LEFT_VAL = 1470;
const int BTN_CONFIRM_VAL = 2655;
const int BTN_BACK_VAL = 3470;
const int BTN_VOLUME_DOWN_VAL = 3;
const int BTN_VOLUME_UP_VAL = 2205;

// Power button timing
const unsigned long POWER_BUTTON_WAKEUP_MS = 1000; // Time required to confirm boot from sleep
const unsigned long POWER_BUTTON_SLEEP_MS = 1000;  // Time required to enter sleep mode

// Display refresh configuration
const int FULL_REFRESH_INTERVAL = -1;                        // -1 = never, 0 = always, N = every N updates
const unsigned long BUTTON_DEBOUNCE_FULL_MS = 500;           // Debounce time after full refresh
const unsigned long BUTTON_DEBOUNCE_PARTIAL_SINGLE_MS = 500; // Debounce time after single-region partial refresh (question only)
const unsigned long BUTTON_DEBOUNCE_PARTIAL_DUAL_MS = 500;   // Debounce time after dual-region partial refresh (question + banner)

#endif // CONFIG_H
