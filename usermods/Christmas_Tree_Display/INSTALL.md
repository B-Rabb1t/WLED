# Christmas Tree Display - Installation Guide

This guide will walk you through installing the Christmas Tree Display usermod on your Lilygo T-Display S3 running WLED.

## Prerequisites

- Lilygo T-Display S3 board
- WLED firmware source code
- PlatformIO (VS Code extension or command line)
- USB-C cable for flashing
- Basic understanding of WLED and Arduino

## Step-by-Step Installation

### Step 1: Download WLED Source Code

If you haven't already, clone the WLED repository:

```bash
git clone https://github.com/wled-dev/WLED.git
cd WLED
```

Or download and extract the latest release from: https://github.com/wled-dev/WLED/releases

### Step 2: Copy Usermod Files

Copy the Christmas Tree Display usermod files to your WLED project:

```bash
# Copy the entire usermod directory
cp -r usermods/Christmas_Tree_Display /path/to/WLED/usermods/
```

The usermod files should now be at:
- `usermods/Christmas_Tree_Display/Christmas_Tree_Display.cpp`
- `usermods/Christmas_Tree_Display/library.json`
- `usermods/Christmas_Tree_Display/README.md`
- `usermods/Christmas_Tree_Display/User_Setup.h`
- `usermods/Christmas_Tree_Display/platformio_override.ini.example`

### Step 3: Configure PlatformIO

You have two options for configuration:

#### Option A: Use the Example Configuration (Recommended)

1. Copy the example configuration to your WLED root:

```bash
cp usermods/Christmas_Tree_Display/platformio_override.ini.example platformio_override.ini
```

2. Open `platformio_override.ini` and verify the environment name matches your board:

```ini
[env:lilygo_t7_s3]  # Make sure this matches your board
```

#### Option B: Manual Configuration

1. Create or edit `platformio_override.ini` in your WLED root directory
2. Add the following configuration:

```ini
[env:lilygo_t7_s3]
board = lilygo-t7-s3
platform = ${esp32s3.platform}
platform_packages = ${esp32s3.platform_packages}
upload_speed = 921600
build_unflags = ${common.build_unflags}
build_flags = ${common.build_flags} ${esp32s3.build_flags}
  -D WLED_RELEASE_NAME=\"Lilygo-T7-S3\"
  -D WLED_WATCHDOG_TIMEOUT=0
  -D ARDUINO_USB_CDC_ON_BOOT=1
  -D ARDUINO_USB_MODE=1
  -DBOARD_HAS_PSRAM
  ; TFT_eSPI configuration
  -D USER_SETUP_LOADED=1
  -D ST7789_DRIVER=1
  -D TFT_WIDTH=170
  -D TFT_HEIGHT=320
  -D TFT_MISO=37
  -D TFT_MOSI=35
  -D TFT_SCLK=36
  -D TFT_CS=6
  -D TFT_DC=4
  -D TFT_RST=5
  -D TFT_BL=7
  -D LOAD_GLCD=1
  -D LOAD_FONT2=1
  -D LOAD_FONT4=1
  -D LOAD_FONT6=1
  -D LOAD_FONT7=1
  -D LOAD_FONT8=1
  -D LOAD_GFXFF=1
  -D SMOOTH_FONT=1
  -D SPI_FREQUENCY=40000000
  -D SPI_READ_FREQUENCY=20000000

lib_deps = ${esp32s3.lib_deps}
  TFT_eSPI

custom_usermods = Christmas_Tree_Display

board_build.partitions = ${esp32.extreme_partitions}
board_upload.flash_size = 16MB
board_upload.maximum_size = 16777216
board_build.f_flash = 80000000L
board_build.flash_mode = qio
monitor_filters = esp32_exception_decoder
```

### Step 4: Install Dependencies

Open your WLED project in VS Code with PlatformIO extension, or run:

```bash
pio lib install
```

This will install the TFT_eSPI library automatically.

### Step 5: Build the Firmware

#### Using VS Code:

1. Open the WLED project in VS Code
2. Click the PlatformIO icon in the left sidebar
3. Select your environment (e.g., `lilygo_t7_s3`)
4. Click the Build button (✓) or press `Ctrl+Alt+B`

#### Using Command Line:

```bash
pio run -e lilygo_t7_s3
```

The build process will take a few minutes. Watch for any errors.

### Step 6: Flash the Firmware

#### Using VS Code:

1. Connect your Lilygo T-Display S3 via USB-C
2. Click the Upload button (→) in PlatformIO or press `Ctrl+Alt+U`

#### Using Command Line:

```bash
pio run -e lilygo_t7_s3 -t upload
```

### Step 7: Configure the Usermod

1. After flashing, connect to your WLED device's WiFi
2. Open the WLED web interface (usually `http://4.3.2.1` in AP mode or your device's IP)
3. Navigate to **Config** → **Usermods**
4. Find the **Christmas Tree** section
5. Configure your preferences:
   - **Enabled**: Check to enable the display
   - **Display Mode**: Choose Full Tree, Hybrid, or Info
   - **Animation Speed**: Adjust 1-10
   - **Color Scheme**: Classic, Rainbow, or WLED Sync
   - **Show in December Only**: Enable for seasonal display
   - **Backlight Timeout**: Set to 0 for always-on, or specify seconds

6. Click **Save** at the bottom of the page

### Step 8: Verify Installation

1. Your Lilygo T-Display S3 should now show the Christmas tree
2. If in Hybrid mode, you should see WLED status information
3. Try changing WLED effects to see the display update
4. Adjust settings in the web interface to customize the display

## Troubleshooting

### Build Errors

**Error: `wled.h file not found`**
- Ensure you're building from the WLED root directory
- Check that the usermod is in `usermods/Christmas_Tree_Display/`

**Error: `TFT_eSPI library not found`**
- Verify `TFT_eSPI` is in `lib_deps`
- Run `pio lib install` to install dependencies
- Check internet connection

**Error: Pin allocation failed**
- The display pins may conflict with other usermods
- Disable other display-related usermods
- Check pin definitions in `platformio_override.ini`

### Display Issues

**Display is blank/black**
- Check if the usermod is enabled in WLED config
- Verify backlight timeout isn't set too low
- Try increasing backlight timeout or setting to 0
- Check TFT_BL pin configuration

**Display shows garbage/corrupted graphics**
- Verify TFT_eSPI User_Setup.h has correct pin definitions
- Check SPI frequency (try lowering to 20000000)
- Ensure display is properly connected

**Tree not showing**
- Verify "Enabled" is checked in WLED config
- Check if "Show in December Only" is enabled and current month
- Try different display modes (Full/Hybrid/Info)

**Animation too fast/slow**
- Adjust "Animation Speed" in WLED config (1-10)
- Lower values = slower, higher = faster

### Performance Issues

**Firmware too large**
- Disable unused usermods
- Reduce font loading in TFT_eSPI config
- Use smaller partition table

**Display flickering**
- Lower SPI frequency in build flags
- Check for power supply issues
- Reduce animation speed

## Advanced Configuration

### Adding Other Usermods

To use multiple usermods together, add them to `custom_usermods`:

```ini
custom_usermods = audioreactive,Christmas_Tree_Display,Battery
```

### Customizing TFT_eSPI

Edit the TFT_eSPI configuration flags in `platformio_override.ini`:

```ini
-D SPI_FREQUENCY=40000000  # Lower if display issues
-D SMOOTH_FONT=0          # Disable for faster rendering
-D LOAD_FONT2=1          # Only load needed fonts
```

### Debug Mode

Enable debug output for troubleshooting:

```ini
build_flags = ${common.build_flags} ${esp32s3.build_flags} -D WLED_DEBUG
```

Then monitor serial output:

```bash
pio device monitor -e lilygo_t7_s3
```

## Uninstallation

To remove the Christmas Tree Display usermod:

1. Delete the usermod directory:

```bash
rm -rf usermods/Christmas_Tree_Display
```

2. Remove from `platformio_override.ini`:

```ini
; Remove this line:
custom_usermods = Christmas_Tree_Display
```

3. Rebuild and flash:

```bash
pio run -e lilygo_t7_s3 -t upload
```

## Getting Help

If you encounter issues:

1. Check the [README.md](README.md) for detailed documentation
2. Search [WLED GitHub Issues](https://github.com/wled-dev/WLED/issues)
3. Join the [WLED Discord](https://discord.gg/WLED)
4. Visit the [WLED Forum](https://wled.discourse.group)

When asking for help, please include:
- Your board model (Lilygo T-Display S3)
- WLED version
- Error messages or screenshots
- Your `platformio_override.ini` configuration

## Next Steps

After successful installation:

1. Explore different display modes
2. Try the WLED Sync color scheme
3. Customize animation speed
4. Enable December-only mode for seasonal display
5. Combine with other WLED features (effects, schedules, etc.)

Enjoy your festive Christmas tree display! 🎄✨
