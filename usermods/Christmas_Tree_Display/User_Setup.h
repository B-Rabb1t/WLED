// User_Setup.h for Lilygo T-Display S3 with ST7789 display
// Configured for SPI interface

#define ST7789_DRIVER

// Display resolution (portrait, will be rotated to landscape)
#define TFT_WIDTH 170
#define TFT_HEIGHT 320

// Pin definitions for Lilygo T-Display S3 SPI interface
#define TFT_CS   5
#define TFT_DC   4
#define TFT_RST  5
#define TFT_MOSI 35
#define TFT_MISO 37
#define TFT_SCLK 36

// Font loading
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT

// SPI frequency
#define SPI_FREQUENCY 40000000
#define SPI_READ_FREQUENCY 20000000
