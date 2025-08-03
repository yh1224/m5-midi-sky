#include "../config.h"

#include <M5Unified.h>

#include "app/controller.h"
#include "app/midi.h"

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

void setup()
{
    Serial.begin(115200);

    const auto cfg = m5::M5Unified::config();
    M5.begin(cfg);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(0, 0);

    setupMIDI(MIDI_GPIO_RX, MIDI_GPIO_TX);
    setupController(DEVICE_NAME, DEVICE_MANUFACTURER);

    currentMapping = MAPPING_DEFAULT;
    currentTranspose = TRANSPOSE_DEFAULT;
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

    // Button B: Switch mapping
    if (btnPressedB) {
        currentMapping += 1;
        if (currentMapping > MAPPING_MAX) {
            currentMapping = MAPPING_MIN;
        }
    }
    if (currentMapping != prevMapping) {
        M5.Speaker.tone(2000, 100);
        M5.Display.setCursor(0, 48);
        M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Display.printf("Mapping: %d    \n", currentMapping);
    }

    // Button A/C: Change transpose
    if (btnPressedA && currentTranspose > TRANSPOSE_MIN) {
        currentTranspose--;
    }
    if (btnPressedC && currentTranspose < TRANSPOSE_MAX) {
        currentTranspose++;
    }
    if (currentTranspose != prevTranspose) {
        M5.Speaker.tone(1000, 100);
        M5.Display.setCursor(0, 128);
        M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
        M5.Display.printf("Transpose: %+d (%s)     \n", currentTranspose, getKey(currentTranspose));
        drawKeyboard(148, 320, 100, currentTranspose);
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
    if (currentMapping != prevMapping ||
        currentTranspose != prevTranspose ||
        notes15 != prevNotes15) {
        updateController(notes15, currentMapping);

        prevMapping = currentMapping;
        prevTranspose = currentTranspose;
        prevNotes15 = notes15;
    }

    delay(1);
}
