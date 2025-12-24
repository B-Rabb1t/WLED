/*
 * Christmas Tree Display Usermod for WLED
 * Displays an animated Christmas tree on ST7789 TFT display
 * Compatible with Lilygo T-Display S3 (320x170)
 * 
 * Features:
 * - Animated Christmas tree with twinkling lights
 * - Snow particle effects
 * - Multiple display modes (Full/Hybrid/Info)
 * - Configurable animation speed and color schemes
 * - Integration with WLED status information
 */

#include "wled.h"
#include <TFT_eSPI.h>
#include <SPI.h>

#ifndef USER_SETUP_LOADED
    #ifndef ST7789_DRIVER
        #error Please define ST7789_DRIVER
    #endif
    #ifndef TFT_WIDTH
        #error Please define TFT_WIDTH
    #endif
    #ifndef TFT_HEIGHT
        #error Please define TFT_HEIGHT
    #endif
    #ifndef TFT_DC
        #error Please define TFT_DC
    #endif
    #ifndef TFT_RST
        #error Please define TFT_RST
    #endif
    #ifndef TFT_CS
        #error Please define TFT_CS
    #endif
    #ifndef LOAD_GLCD
        #error Please define LOAD_GLCD
    #endif
#endif

#ifndef TFT_BL
#define TFT_BL -1
#endif

#define USERMOD_ID_CHRISTMAS_TREE_DISPLAY 98

// TFT_eSPI object - declare but don't initialize until needed
// This prevents TFT_eSPI constructor from running at boot and interfering with WiFi
static TFT_eSPI tft(TFT_WIDTH, TFT_HEIGHT);
static bool tftInitialized = false;

// Display modes
#define MODE_FULL_TREE 0
#define MODE_HYBRID 1
#define MODE_INFO 2

// Color schemes
#define COLOR_CLASSIC 0
#define COLOR_RAINBOW 1
#define COLOR_WLED_SYNC 2

// Extra char (+1) for null
#define LINE_BUFFER_SIZE 20

// How often we are redrawing screen
#define USER_LOOP_REFRESH_RATE_MS 100

// Tree configuration (portrait mode: 170x320)
#define TREE_CENTER_X (TFT_WIDTH / 2)
#define TREE_TOP_Y 30
#define TREE_TIER_HEIGHT 45
#define TREE_BASE_WIDTH 120

// Snow particles
#define MAX_SNOWFLAKES 30

extern int getSignalQuality(int rssi);

struct Snowflake {
    int x;
    int y;
    int speed;
    int size;
};

class ChristmasTreeDisplayUsermod : public Usermod {
  private:
    unsigned long lastTime = 0;
    bool enabled = false; // Disabled by default - enable via WLED web UI
    bool displayTurnedOff = false;
    long lastRedraw = 0;
    bool needRedraw = true;
    
    // Configuration
    uint8_t displayMode = MODE_HYBRID;
    uint8_t animationSpeed = 5;
    uint8_t colorScheme = COLOR_CLASSIC;
    bool showInDecemberOnly = false;
    uint16_t backlightTimeout = 300; // 5 minutes default
    
    // Display state
    String knownSsid = "";
    IPAddress knownIp;
    uint8_t knownBrightness = 0;
    uint8_t knownMode = 0;
    uint8_t knownPalette = 0;
    uint8_t knownEffectSpeed = 0;
    uint8_t knownEffectIntensity = 0;
    
    // Animation state
    unsigned long lastAnimationUpdate = 0;
    uint8_t animationFrame = 0;
    uint8_t rainbowHue = 0;
    Snowflake snowflakes[MAX_SNOWFLAKES];
    
    // Tree lights state
    struct TreeLight {
        int x;
        int y;
        bool on;
        uint8_t colorIndex;
    };
    TreeLight treeLights[50];
    int numLights = 0;
    
    const uint8_t tftcharwidth = 19;
    long lastUpdate = 0;
    
    void initSnowflakes() {
        for (int i = 0; i < MAX_SNOWFLAKES; i++) {
            snowflakes[i].x = random(TFT_WIDTH);
            snowflakes[i].y = random(TFT_HEIGHT);
            snowflakes[i].speed = random(1, 4);
            snowflakes[i].size = random(1, 3);
        }
    }
    
    void initTreeLights() {
        numLights = 0;
        // Define light positions for each tier
        // Tier 1 (top)
        for (int i = 0; i < 8; i++) {
            int x = TREE_CENTER_X - 20 + i * 5;
            int y = TREE_TOP_Y + 15;
            treeLights[numLights].x = x;
            treeLights[numLights].y = y;
            treeLights[numLights].on = random(2);
            treeLights[numLights].colorIndex = i % 2;
            numLights++;
        }
        // Tier 2
        for (int i = 0; i < 12; i++) {
            int x = TREE_CENTER_X - 35 + i * 6;
            int y = TREE_TOP_Y + 35;
            treeLights[numLights].x = x;
            treeLights[numLights].y = y;
            treeLights[numLights].on = random(2);
            treeLights[numLights].colorIndex = i % 2;
            numLights++;
        }
        // Tier 3
        for (int i = 0; i < 16; i++) {
            int x = TREE_CENTER_X - 50 + i * 6;
            int y = TREE_TOP_Y + 55;
            treeLights[numLights].x = x;
            treeLights[numLights].y = y;
            treeLights[numLights].on = random(2);
            treeLights[numLights].colorIndex = i % 2;
            numLights++;
        }
        // Tier 4 (bottom)
        for (int i = 0; i < 14; i++) {
            int x = TREE_CENTER_X - 45 + i * 6;
            int y = TREE_TOP_Y + 75;
            treeLights[numLights].x = x;
            treeLights[numLights].y = y;
            treeLights[numLights].on = random(2);
            treeLights[numLights].colorIndex = i % 2;
            numLights++;
        }
    }
    
    uint16_t getLightColor(int colorIndex) {
        switch (colorScheme) {
            case COLOR_CLASSIC:
                return colorIndex == 0 ? TFT_RED : TFT_GREEN;
            case COLOR_RAINBOW:
                return tft.color565(
                    (rainbowHue + colorIndex * 30) % 256,
                    200,
                    200
                );
            case COLOR_WLED_SYNC:
                // Use current WLED palette color
                return strip.getMainSegment().colors[0];
            default:
                return TFT_RED;
        }
    }
    
    void drawTree() {
        int baseY = TREE_TOP_Y + 100;
        
        // Draw trunk
        tft.fillRect(TREE_CENTER_X - 10, baseY, 20, 20, TFT_BROWN);
        
        // Draw tree tiers (triangles)
        for (int tier = 0; tier < 4; tier++) {
            int tierTop = TREE_TOP_Y + tier * 25;
            int tierWidth = 30 + tier * 30;
            
            // Draw filled triangle
            for (int y = 0; y < 25; y++) {
                int widthAtY = tierWidth * (y + 1) / 25;
                int leftX = TREE_CENTER_X - widthAtY / 2;
                tft.drawLine(leftX, tierTop + y, leftX + widthAtY, tierTop + y, TFT_GREEN);
            }
        }
    }
    
    void drawLights() {
        for (int i = 0; i < numLights; i++) {
            if (treeLights[i].on) {
                uint16_t color = getLightColor(treeLights[i].colorIndex);
                tft.fillCircle(treeLights[i].x, treeLights[i].y, 3, color);
                // Add glow effect
                tft.drawCircle(treeLights[i].x, treeLights[i].y, 4, color);
            } else {
                // Draw dim light
                tft.fillCircle(treeLights[i].x, treeLights[i].y, 2, TFT_DARKGREEN);
            }
        }
    }
    
    void drawStar() {
        int starX = TREE_CENTER_X;
        int starY = TREE_TOP_Y - 5;
        
        // Pulsing star effect
        uint8_t pulse = (sin(animationFrame * 0.1) + 1) * 127;
        uint16_t starColor = tft.color565(255, 255, pulse);
        
        // Draw star shape
        tft.fillTriangle(starX, starY - 8, starX - 6, starY + 4, starX + 6, starY + 4, starColor);
        tft.fillTriangle(starX, starY + 4, starX - 6, starY - 2, starX + 6, starY - 2, starColor);
    }
    
    void updateSnowflakes() {
        // Clear old snowflakes
        for (int i = 0; i < MAX_SNOWFLAKES; i++) {
            tft.fillRect(snowflakes[i].x, snowflakes[i].y, snowflakes[i].size, snowflakes[i].size, TFT_BLACK);
        }
        
        // Update and draw new positions
        for (int i = 0; i < MAX_SNOWFLAKES; i++) {
            snowflakes[i].y += snowflakes[i].speed;
            snowflakes[i].x += random(-1, 2);
            
            // Reset if off screen
            if (snowflakes[i].y > TFT_HEIGHT) {
                snowflakes[i].y = -5;
                snowflakes[i].x = random(TFT_WIDTH);
            }
            if (snowflakes[i].x < 0) snowflakes[i].x = TFT_WIDTH - 1;
            if (snowflakes[i].x >= TFT_WIDTH) snowflakes[i].x = 0;
            
            // Draw snowflake
            tft.fillCircle(snowflakes[i].x, snowflakes[i].y, snowflakes[i].size, TFT_WHITE);
        }
    }
    
    void animateLights() {
        // Randomly toggle some lights
        for (int i = 0; i < numLights; i++) {
            if (random(100) < (animationSpeed * 2)) {
                treeLights[i].on = !treeLights[i].on;
            }
        }
    }
    
    void drawWLEDInfo() {
        char buff[LINE_BUFFER_SIZE];
        int startY = 240;
        
        tft.setTextSize(1);
        
        // WiFi name
        tft.setTextColor(TFT_GREEN);
        tft.setCursor(0, startY);
        String line = knownSsid.substring(0, 25);
        if (knownSsid.length() > 25) line = line.substring(0, 22) + "...";
        tft.print(line);
        
        // IP address
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(0, startY + 12);
        tft.print("IP: ");
        tft.print(knownIp);
        
        // Brightness
        tft.setTextColor(TFT_CYAN);
        tft.setCursor(0, startY + 24);
        tft.print("Bri: ");
        tft.print(((int)bri * 100) / 255);
        tft.print("%");
        
        // Signal
        tft.setCursor(100, startY + 24);
        tft.print("Sig: ");
        if (getSignalQuality(WiFi.RSSI()) < 10) {
            tft.setTextColor(TFT_RED);
        } else if (getSignalQuality(WiFi.RSSI()) < 25) {
            tft.setTextColor(TFT_ORANGE);
        } else {
            tft.setTextColor(TFT_GREEN);
        }
        tft.print(getSignalQuality(WiFi.RSSI()));
        tft.print("%");
        
        // Effect name
        tft.setTextColor(TFT_YELLOW);
        tft.setCursor(0, startY + 36);
        char lineBuffer[LINE_BUFFER_SIZE];
        extractModeName(knownMode, JSON_mode_names, lineBuffer, 18);
        tft.print(lineBuffer);
    }
    
    void drawSmallTreeIcon() {
        int x = 150;
        int y = 5;
        
        // Small tree icon
        tft.fillTriangle(x, y, x - 8, y + 15, x + 8, y + 15, TFT_GREEN);
        tft.fillTriangle(x, y + 10, x - 6, y + 22, x + 6, y + 22, TFT_GREEN);
        tft.fillRect(x - 2, y + 22, 4, 4, TFT_BROWN);
        
        // Small star
        tft.fillCircle(x, y - 2, 2, TFT_YELLOW);
    }
    
    bool shouldDisplayTree() {
        if (!showInDecemberOnly) return true;
        
        // Check if current month is December
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        return (timeinfo->tm_mon == 11); // December is month 11 (0-indexed)
    }
    
    void center(String &line, uint8_t width) {
        int len = line.length();
        if (len < width) {
            for (byte i = (width - len) / 2; i > 0; i--) {
                line = ' ' + line;
            }
        }
        for (byte i = line.length(); i < width; i++) {
            line += ' ';
        }
    }

  public:
    void setup() override {
        // Only initialize display if enabled
        if (!enabled) return;
        
        // Allocate 8-bit parallel interface control pins for Lilygo T-Display S3
        // Control pins: DC=7, RST=5, CS=6, WR=8, RD=9, BL=38
        // Data pins (39-48) are shared with PSRAM and NOT allocated here
        // PSRAM manages these pins automatically
        PinManagerPinType controlPins[] = { { 5, true }, { 6, true }, { 7, true }, { 8, true }, { 9, true }, { 38, true } };
        if (!PinManager::allocateMultiplePins(controlPins, 6, PinOwner::UM_FourLineDisplay)) {
            enabled = false;
            return;
        }
        
        // Delay display initialization to allow WiFi to start first
        delay(1000);
        
        // Initialize display
        tft.init();
        tft.setRotation(0); // Portrait mode for 170x320
        tft.fillScreen(TFT_BLACK);
        tftInitialized = true;
        
        // Turn on backlight (GPIO38)
        pinMode(TFT_BL, OUTPUT);
        digitalWrite(TFT_BL, HIGH); // Turn backlight ON
        
        initSnowflakes();
        initTreeLights();
        
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.setCursor(45, 150);
        tft.print("Loading...");
    }

    void connected() override {
    }

    void loop() override {
        // Don't use display until WiFi is connected to prevent initialization issues
        if (!enabled || !WLED_CONNECTED || !tftInitialized) return;
        
        // Check if we should display tree
        if (!shouldDisplayTree()) {
            // Show normal WLED info instead
            if (millis() - lastUpdate < USER_LOOP_REFRESH_RATE_MS) return;
            lastUpdate = millis();
            
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_WHITE);
            tft.setTextSize(2);
            tft.setCursor(45, 150);
            tft.print("WLED Ready");
            return;
        }
        
        // Turn on backlight when first accessing display
        if (displayTurnedOff) {
            if (TFT_BL >= 0) digitalWrite(TFT_BL, HIGH);
            displayTurnedOff = false;
        }
        
        // Animation update
        unsigned long now = millis();
        unsigned long animationInterval = 1000 / (animationSpeed * 2);
        
        if (now - lastAnimationUpdate > animationInterval) {
            lastAnimationUpdate = now;
            animationFrame++;
            rainbowHue = (rainbowHue + 5) % 256;
            
            animateLights();
            
            needRedraw = true;
        }
        
        // Full redraw interval
        if (millis() - lastUpdate < USER_LOOP_REFRESH_RATE_MS) return;
        lastUpdate = millis();
        
        // Backlight timeout
        if (!displayTurnedOff && backlightTimeout > 0 && millis() - lastRedraw > backlightTimeout * 1000) {
            if (TFT_BL >= 0) digitalWrite(TFT_BL, LOW);
            displayTurnedOff = true;
        }
        
        // Check for changes
        String currentSsid = (apActive) ? String(apSSID) : WiFi.SSID();
        IPAddress currentIp = (apActive ? IPAddress(4,3,2,1) : Network.localIP());
        
        if (currentSsid != knownSsid ||
            currentIp != knownIp ||
            knownBrightness != bri ||
            knownEffectSpeed != strip.getMainSegment().speed ||
            knownEffectIntensity != strip.getMainSegment().intensity ||
            knownMode != strip.getMainSegment().mode ||
            knownPalette != strip.getMainSegment().palette)
        {
            needRedraw = true;
        }
        
        if (!needRedraw) return;
        needRedraw = false;
        
        // Turn on backlight if it was off
        if (displayTurnedOff) {
            if (TFT_BL >= 0) digitalWrite(TFT_BL, HIGH);
            displayTurnedOff = false;
        }
        
        lastRedraw = millis();
        
        // Update known values
        knownSsid = WiFi.SSID();
        knownIp = apActive ? IPAddress(4,3,2,1) : Network.localIP();
        knownBrightness = bri;
        knownMode = strip.getMainSegment().mode;
        knownPalette = strip.getMainSegment().palette;
        knownEffectSpeed = strip.getMainSegment().speed;
        knownEffectIntensity = strip.getMainSegment().intensity;
        
        // Draw based on display mode
        tft.fillScreen(TFT_BLACK);
        
        if (displayMode == MODE_FULL_TREE) {
            drawTree();
            drawLights();
            drawStar();
            updateSnowflakes();
        } else if (displayMode == MODE_HYBRID) {
            drawTree();
            drawLights();
            drawStar();
            updateSnowflakes();
            drawWLEDInfo();
        } else if (displayMode == MODE_INFO) {
            drawWLEDInfo();
            drawSmallTreeIcon();
        }
    }

    void addToJsonInfo(JsonObject& root) override {
        JsonObject user = root["u"];
        if (user.isNull()) user = root.createNestedObject("u");
        
        JsonArray treeArr = user.createNestedArray("Christmas Tree");
        treeArr.add(enabled ? F("enabled") : F("disabled"));
        
        if (enabled) {
            JsonArray modeArr = user.createNestedArray("Tree Mode");
            String modeStr;
            if (displayMode == MODE_FULL_TREE) modeStr = "Full";
            else if (displayMode == MODE_HYBRID) modeStr = "Hybrid";
            else modeStr = "Info";
            modeArr.add(modeStr);
        }
    }

    void addToConfig(JsonObject& root) override {
        JsonObject top = root.createNestedObject("ChristmasTree");
        top["enabled"] = enabled;
        top["displayMode"] = displayMode;
        top["animationSpeed"] = animationSpeed;
        top["colorScheme"] = colorScheme;
        top["showInDecemberOnly"] = showInDecemberOnly;
        top["backlightTimeout"] = backlightTimeout;
        
        JsonArray pins = top.createNestedArray("pin");
        pins.add(TFT_CS);
        pins.add(TFT_DC);
        pins.add(TFT_RST);
        pins.add(TFT_BL);
    }

    void appendConfigData() override {
        oappend(F("dd=addDropdown('ChristmasTree','displayMode');"));
        oappend(F("addOption(dd,'Full Tree',0);"));
        oappend(F("addOption(dd,'Hybrid',1);"));
        oappend(F("addOption(dd,'Info',2);"));
        
        oappend(F("addInfo('ChristmasTree:displayMode',0,'Display mode');"));
        
        oappend(F("addSlider('ChristmasTree','animationSpeed',1,10,5);"));
        oappend(F("addInfo('ChristmasTree:animationSpeed',1,'Animation speed');"));
        
        oappend(F("dd=addDropdown('ChristmasTree','colorScheme');"));
        oappend(F("addOption(dd,'Classic',0);"));
        oappend(F("addOption(dd,'Rainbow',1);"));
        oappend(F("addOption(dd,'WLED Sync',2);"));
        oappend(F("addInfo('ChristmasTree:colorScheme',0,'Color scheme');"));
        
        oappend(F("addCheckbox('ChristmasTree','showInDecemberOnly',false);"));
        oappend(F("addInfo('ChristmasTree:showInDecemberOnly',0,'Only show in December');"));
        
        oappend(F("addNumber('ChristmasTree','backlightTimeout',0,3600,300);"));
        oappend(F("addInfo('ChristmasTree:backlightTimeout',1,'Backlight timeout (seconds, 0=never)');"));
        
        oappend(F("addInfo('ChristmasTree:pin[]',0,','SPI CS');"));
        oappend(F("addInfo('ChristmasTree:pin[]',1,','SPI DC');"));
        oappend(F("addInfo('ChristmasTree:pin[]',2,','SPI RST');"));
        oappend(F("addInfo('ChristmasTree:pin[]',3,','SPI BL');"));
    }

    bool readFromConfig(JsonObject& root) override {
        JsonObject top = root["ChristmasTree"];
        if (top.isNull()) return true;
        
        bool configComplete = true;
        
        configComplete &= getJsonValue(top["enabled"], enabled);
        configComplete &= getJsonValue(top["displayMode"], displayMode);
        configComplete &= getJsonValue(top["animationSpeed"], animationSpeed);
        configComplete &= getJsonValue(top["colorScheme"], colorScheme);
        configComplete &= getJsonValue(top["showInDecemberOnly"], showInDecemberOnly);
        configComplete &= getJsonValue(top["backlightTimeout"], backlightTimeout);
        
        // Clamp values
        if (displayMode > 2) displayMode = MODE_HYBRID;
        if (animationSpeed < 1) animationSpeed = 1;
        if (animationSpeed > 10) animationSpeed = 10;
        if (colorScheme > 2) colorScheme = COLOR_CLASSIC;
        
        return configComplete;
    }

    uint16_t getId() override {
        return USERMOD_ID_CHRISTMAS_TREE_DISPLAY;
    }
};

static ChristmasTreeDisplayUsermod christmas_tree_display;
REGISTER_USERMOD(christmas_tree_display);
