#include "../config.h"

#include <M5Unified.h>

#include "app/controller.h"
#include "app/midi.h"

// Settings menu system
enum class SettingType
{
    NONE = 0,
    MAPPING = 1,
    TRANSPOSE = 2,
    SUSTAIN = 3,
    COUNT = 4,
};

static auto currentSetting = SettingType::NONE;
static auto prevSetting = SettingType::COUNT;

// Mapping settings
constexpr int MAPPING_MIN = 1;
constexpr int MAPPING_MAX = 2;
constexpr int MAPPING_DEFAULT = 1;
static int currentMapping = 0;
static int prevMapping = INT_MIN;

// Transpose settings
constexpr int TRANSPOSE_MIN = -12;
constexpr int TRANSPOSE_MAX = 12;
constexpr int TRANSPOSE_DEFAULT = 0;
static int currentTranspose = 0;
static int prevTranspose = INT_MIN;

// Sustain settings
constexpr bool SUSTAIN_DEFAULT = false;
static bool currentSustain = SUSTAIN_DEFAULT;
static bool prevSustain = !SUSTAIN_DEFAULT;

void setup()
{
    Serial.begin(115200);

    const auto cfg = m5::M5Unified::config();
    M5.begin(cfg);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(0, 0);

    currentMapping = MAPPING_DEFAULT;
    currentTranspose = TRANSPOSE_DEFAULT;
    currentSustain = SUSTAIN_DEFAULT;

    setupMIDI(MIDI_GPIO_RX, MIDI_GPIO_TX, currentSustain);
    setupController(DEVICE_NAME, DEVICE_MANUFACTURER);

    // Initialize display
    M5.Display.fillScreen(TFT_BLACK);
}


void loop()
{
    // Previous notes state
    static Notes15 prevNotes15;

    // Touch state tracking for one-touch detection
    static bool wasTouchPressed = false;

    bool btnPressedA = false;
    bool btnPressedB = false;
    bool btnPressedC = false;

    M5.update();

    // Touch detection with one-touch logic
    const bool isTouchPressed = M5.Touch.getCount() > 0;
    const bool touchJustPressed = isTouchPressed && !wasTouchPressed;
    wasTouchPressed = isTouchPressed;
    if (touchJustPressed) {
        auto t = M5.Touch.getDetail();
        if (200 <= t.y && t.y < 240) {
            if (0 <= t.x && t.x < 106) {
                btnPressedA = true;
            } else if (t.x < 214) {
                btnPressedB = true;
            } else {
                btnPressedC = true;
            }
        }
    }
    if (M5.BtnA.wasPressed()) {
        btnPressedA = true;
    }
    if (M5.BtnB.wasPressed()) {
        btnPressedB = true;
    }
    if (M5.BtnC.wasPressed()) {
        btnPressedC = true;
    }

    // Button B: Switch setting
    if (btnPressedB) {
        currentSetting = static_cast<SettingType>((static_cast<int>(currentSetting) + 1) % static_cast<int>(
            SettingType::COUNT));
        if (currentSetting == SettingType::NONE) {
            M5.Speaker.tone(1000, 500);
        } else {
            M5.Speaker.tone(1000, 100);
        }
    }

    // Button A/C: Change current setting value (only if a setting is selected)
    if ((btnPressedA || btnPressedC) && currentSetting != SettingType::NONE) {
        switch (currentSetting) {
        case SettingType::MAPPING:
            M5.Speaker.tone(2000, 100);
            if (btnPressedA && currentMapping > MAPPING_MIN) {
                currentMapping--;
            }
            if (btnPressedC && currentMapping < MAPPING_MAX) {
                currentMapping++;
            }
            break;
        case SettingType::TRANSPOSE:
            M5.Speaker.tone(2000, 100);
            if (btnPressedA && currentTranspose > TRANSPOSE_MIN) {
                currentTranspose--;
            }
            if (btnPressedC && currentTranspose < TRANSPOSE_MAX) {
                currentTranspose++;
            }
            break;
        case SettingType::SUSTAIN:
            M5.Speaker.tone(2000, 100);
            if (btnPressedA || btnPressedC) {
                currentSustain = !currentSustain;
                setSustainEnabled(currentSustain);
            }
            break;
        default:
            break;
        }
    }

    if (currentSetting != prevSetting ||
        currentMapping != prevMapping ||
        currentTranspose != prevTranspose ||
        currentSustain != prevSustain) {
        M5.Display.fillRect(0, 48, 320, static_cast<int>(SettingType::COUNT) * 16, TFT_BLACK);
        M5.Display.setCursor(0, 48);

        M5.Display.setTextColor(currentSetting == SettingType::MAPPING ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
        M5.Display.printf("Mapping: %d\n", currentMapping);

        M5.Display.setTextColor(currentSetting == SettingType::TRANSPOSE ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
        M5.Display.printf("Transpose: %+d (%s)\n", currentTranspose, getKey(currentTranspose));

        M5.Display.setTextColor(currentSetting == SettingType::SUSTAIN ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
        M5.Display.printf("Sustain: %s\n", currentSustain ? "ON" : "OFF");

        drawKeyboard(128, 320, 100, currentTranspose);
    }

#if defined(MODE_TEST)
    // test mode
    static int testIndex = 14;
    static unsigned long testPrevTs = 0;
    const unsigned long ts = millis();
    if (ts - testPrevTs > 500) {
        testPrevTs = ts;
        testIndex = ++testIndex % 15;
    }
    unsigned long testTimestamps[15] = {0};
    testTimestamps[testIndex] = ts;
    Notes15 notes15(testTimestamps);
#else
    Notes15 notes15 = getNotes15(currentTranspose);
#endif
    // Update controller if there are changes
    if (currentSetting != prevSetting ||
        currentMapping != prevMapping ||
        currentTranspose != prevTranspose ||
        currentSustain != prevSustain ||
        notes15 != prevNotes15) {
        updateController(notes15, currentMapping);

        prevSetting = currentSetting;
        prevMapping = currentMapping;
        prevTranspose = currentTranspose;
        prevSustain = currentSustain;
        prevNotes15 = notes15;
    }

    delay(1);
}
