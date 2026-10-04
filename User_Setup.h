//                            USER DEFINED SETTINGS
//   ST7735 128x160 SPI TFT on ESP32

#define USER_SETUP_INFO "User_Setup"

// ##################################################################################
// Section 1. Driver selection
// ##################################################################################

#define ST7735_DRIVER      // ST7735 driver

// Colour order: try TFT_RGB first; if red/blue are swapped, switch to TFT_BGR
#define TFT_RGB_ORDER TFT_RGB
//#define TFT_RGB_ORDER TFT_BGR

// Portrait dimensions for 1.8" 128x160 ST7735
#define TFT_WIDTH  128
#define TFT_HEIGHT 160

// ST7735 tab type — try GREENTAB first.
// If colors are wrong or image is shifted, try REDTAB or BLACKTAB.
//#define ST7735_GREENTAB
//#define ST7735_GREENTAB2
//#define ST7735_GREENTAB3
#define ST7735_ROBOTLCD

// If white shows as black (or vice versa), uncomment ONE of these:
// #define TFT_INVERSION_ON
// #define TFT_INVERSION_OFF


// ##################################################################################
// Section 2. ESP32 pin mapping  (EDIT these to match YOUR wiring)
// ##################################################################################

// Hardware SPI pins on ESP32 (can be remapped to any GPIO)
#define TFT_MISO -1    // not used by ST7735 (leave unconnected is fine)
#define TFT_MOSI 23    // SDA
#define TFT_SCLK 18    // SCL / SCK
#define TFT_CS   5    // Chip select
#define TFT_DC    2    // Data/Command (A0 / RS)
#define TFT_RST   4    // Reset  (set to -1 if tied to ESP32 EN/RST)

// Backlight control (optional — comment out if BLK is tied to 3.3V)
// #define TFT_BL   32
// #define TFT_BACKLIGHT_ON HIGH

// If you use the second SPI peripheral (HSPI) instead of VSPI, uncomment:
// #define USE_HSPI_PORT


// ##################################################################################
// Section 3. Fonts
// ##################################################################################

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT


// ##################################################################################
// Section 4. Other options
// ##################################################################################

// ST7735 is unreliable above ~27 MHz. Start at 20 MHz if you see glitches.
// #define SPI_FREQUENCY  10000000
 #define SPI_FREQUENCY  20000000
//#define SPI_FREQUENCY  27000000

#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
