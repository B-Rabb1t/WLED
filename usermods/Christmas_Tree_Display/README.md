# Christmas Tree Display Usermod for WLED

**⚠️ IMPORTANT: This usermod is DISABLED by default to prevent WiFi issues. Enable it via WLED web UI (Config → Usermods) only after verifying WiFi works correctly.**

A festive usermod for WLED that displays an animated Christmas tree on Lilygo T-Display S3's built-in ST7789 TFT display (320x170 resolution).

## Features

- **Animated 4-tier Christmas tree** with 50 twinkling lights
- **30 falling snow particles** with realistic movement
- **Pulsing star** on top of the tree
- **3 display modes**: Full Tree, Hybrid (tree + info), Info only
- **3 color schemes**: Classic red/green, Rainbow, WLED Sync
- **Configurable animation speed** (1-10)
- **December-only mode option** - only display the tree during December
- **Backlight timeout control** to save power
- **WLED status display** showing WiFi status, IP address, brightness, and current effect

## Hardware

- **Board**: Lilygo T-Display S3 (ESP32-S3)
- **Display**: ST7789 TFT (320x170 pixels, 8-bit parallel interface)
- **Library**: Custom TFT_eSPI v2.5.43 (from T-Display-S3 repo)

## Pin Configuration

The display uses the following pins on the Lilygo T-Display S3:

### Control Pins
| Pin | Function | Notes |
|-----|----------|-------|
| 6   | CS (Chip Select) | Display chip select |
| 7   | DC (Data/Command) | Display data/command |
| 5   | RST (Reset) | Display reset |
| 8   | WR (Write) | Write strobe |
| 9   | RD (Read) | Read strobe |
| 38  | BL (Backlight) | Display backlight |

### Data Pins (8-bit Parallel)
| Pin | Function | Notes |
|-----|----------|-------|
| 39  | D0 | Data bit 0 |
| 40  | D1 | Data bit 1 |
| 41  | D2 | Data bit 2 |
| 42  | D3 | Data bit 3 |
| 45  | D4 | Data bit 4 |
| 46  | D5 | Data bit 5 |
| 47  | D6 | Data bit 6 |
| 48  | D7 | Data bit 7 |

### Pin Conflicts

These display pins **do NOT conflict** with:
- ✅ **WiFi** (uses pins 12-15 for flash, other pins for WiFi radio)
- ✅ **I2S Microphone** (pins 1, 2, 3)
- ✅ **WS2812B LED Strip** (pin 10)

## Installation

### Step 1: Copy Custom TFT_eSPI Library

The custom TFT_eSPI library from the T-Display-S3 repo has been copied to:
- `usermods/Christmas_Tree_Display/TFT_eSPI/`

This library includes 8-bit parallel interface support specifically for the Lilygo T-Display S3.

### Step 2: Configure PlatformIO

The custom TFT_eSPI library is already configured in the WLED platformio.ini. No additional configuration is needed.

### Step 3: Build and Flash

1. Open the project in VS Code with PlatformIO extension
2. Select `esp32s3dev_16MB_opi` environment
3. Build the firmware
4. Flash to your Lilygo T-Display S3

### Step 4: Enable the Usermod

After flashing:

1. Connect to your WLED device via web interface
2. Navigate to **Config** → **Usermods**
3. Find **Christmas Tree** section
4. Check the **enabled** checkbox
5. Save the configuration

The display will initialize and show the Christmas tree animation.

## Configuration Options

### Display Mode

- **Full Tree**: Shows only the animated Christmas tree with snow
- **Hybrid**: Shows the tree with WLED status information at the bottom
- **Info**: Shows only WLED status information with a small tree icon

### Animation Speed

Controls how fast the lights twinkle and snow falls (1-10, default: 5)

### Color Scheme

- **Classic**: Traditional red and green lights
- **Rainbow**: Lights cycle through rainbow colors
- **WLED Sync**: Lights match the current WLED palette color

### December Only

When enabled, the Christmas tree only displays during December. Other months show "WLED Ready" message.

### Backlight Timeout

Controls how long the backlight stays on before turning off (in seconds, 0 = never, default: 300 seconds = 5 minutes)

## Power Management

**Important: The display is now DISABLED by default to prevent WiFi initialization issues.**

When the usermod is enabled:
- The display initializes and configures the TFT hardware
- The display backlight is initially ON (GPIO15 for battery, GPIO38 for USB)
- The display shows the Christmas tree animation
- The backlight automatically turns OFF after the configured timeout (if set)
- The display is only updated when there are changes to WLED state

**Why disabled by default?**
The Lilygo T-Display S3 uses an 8-bit parallel interface for the display (pins 39-48). These pins are shared with PSRAM. To prevent any potential conflicts with WiFi initialization, the usermod is disabled by default. Enable it only after verifying that WiFi works correctly.

## Troubleshooting

### Display not working

1. Verify that the usermod is enabled in WLED Config → Usermods
2. Check that the custom TFT_eSPI library is properly configured
3. Verify that the pin configuration matches your hardware
4. Check for pin conflicts with other usermods

### WiFi not connecting

The display pins (5-9, 38-48) do not conflict with WiFi. If you experience WiFi issues, check:
- Other usermods that may be using conflicting pins
- Your WiFi credentials and signal strength
- Try disabling the Christmas Tree usermod temporarily to isolate the issue

### High power consumption

The display backlight can consume significant power. To reduce power usage:
- Set a shorter backlight timeout
- Use the "Info" display mode (less frequent redraws)
- Disable the usermod when not needed

## Technical Notes

The Lilygo T-Display S3 uses an **8-bit parallel interface** for the display, not SPI. The custom TFT_eSPI library from the T-Display-S3 repo includes the proper 8-bit parallel interface support.

The display uses `TFT_PARALLEL_8_BIT` configuration in TFT_eSPI, which enables the parallel interface mode with the appropriate pin definitions.

The 8-bit parallel interface provides faster display updates compared to SPI, but uses more GPIO pins. The custom TFT_eSPI library has been specifically configured for the Lilygo T-Display S3 to ensure proper operation.

## License

This usermod is provided as-is for use with WLED. WLED is licensed under the MIT License.
