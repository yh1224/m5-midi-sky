#include "../config.h"

#include <M5Unified.h>

#include "app/controller.h"
#include "app/display.h"
#include "app/midi.h"
#include "app/settings.h"

void setup()
{
    Serial.begin(115200);

    const auto cfg = m5::M5Unified::config();
    M5.begin(cfg);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(0, 0);
    M5.Speaker.setVolume(20);

    setupMIDI(MIDI_GPIO_RX, MIDI_GPIO_TX);
    setupController(DEVICE_NAME, DEVICE_MANUFACTURER);

    // Initialize display
    resetDisplay(false);
}


void loop()
{
    static bool firstDraw = true;

    // Settings
    static Settings settings;
    static bool previousSettingsMode = false;

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
        const auto t = M5.Touch.getDetail();
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
    if (btnPressedA || btnPressedB || btnPressedC) {
        // Process setting button presses
        if (settings.processButtons(btnPressedA, btnPressedB, btnPressedC)) {
            const auto isSettingsMode = settings.isSettingsMode();
            if (isSettingsMode != previousSettingsMode) {
                resetDisplay(isSettingsMode);
                firstDraw = true;
            }
            if (isSettingsMode) {
                drawSettings(settings);
            }
            previousSettingsMode = isSettingsMode;
        }
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
    const Notes15 notes15{testTimestamps};
#else
    const Notes15 notes15 = getNotes15(settings.getBaseNote(), false);
#endif
    // Update controller if there are changes
    if (firstDraw || notes15 != prevNotes15) {
        updateController(notes15, settings.getMapping());

        // Display notes when not in settings mode
        if (!settings.isSettingsMode()) {
            drawNotes(notes15, 32, 320, 160, 16, firstDraw);
        }

        prevNotes15 = notes15;
        firstDraw = false;
    }

    delay(1);
}
