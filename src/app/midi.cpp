#include <M5Unified.h>
#include <MIDI.h>

#include "app/midi.h"

// Constants
static constexpr int MAX_NOTES = 128;

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

// Key states
static bool notes[MAX_NOTES] = {false};

/** MIDI receive task */
[[noreturn]] void midiTask(void*)
{
    while (true) {
        if (MIDI.read()) {
            switch (MIDI.getType()) {
            case midi::NoteOn:
                {
                    const int noteNum = MIDI.getData1();
                    if (0 <= noteNum && noteNum < MAX_NOTES) {
                        notes[noteNum] = true;
                    }
                    break;
                }
            case midi::NoteOff:
                {
                    const int noteNum = MIDI.getData1();
                    if (0 <= noteNum && noteNum < MAX_NOTES) {
                        notes[noteNum] = false;
                    }
                    break;
                }
            default:
                break;
            }
        }
        vTaskDelay(1);
    }
}

void setupMIDI(const int8_t rxPin, const int8_t txPin)
{
    memset(notes, false, sizeof(bool) * MAX_NOTES);

    Serial2.begin(31250, SERIAL_8N1, rxPin, txPin);

    MIDI.turnThruOn();

    // Start MIDI receive task
    xTaskCreatePinnedToCore(
        midiTask,
        "performanceTask",
        8192,
        nullptr,
        1,
        nullptr,
        0
    );
}

uint16_t getNotes15(const int transpose)
{
    // 15 pitches from C3 to C5 (48-72)
    const static int noteMapping[15] = {
        48, 50, 52, 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72,
    };

    uint16_t notes15 = 0;

    for (int midiNote = 0; midiNote < MAX_NOTES; midiNote++) {
        if (!notes[midiNote]) {
            continue;
        }

        // Apply transpose
        const int transposedNote = midiNote + transpose;
        if (transposedNote < 0 || transposedNote >= MAX_NOTES) {
            continue;
        }

        // Map all notes to 48-72 range (C3-C5)
        int targetNote = transposedNote;
        if (transposedNote < 48) {
            targetNote += ((4 - transposedNote / 12) * 12); // Add 12, 24, 36, or 48
        } else if (transposedNote > 72) {
            targetNote -= (((transposedNote - 61) / 12) * 12); // Subtract 12, 24, 36, or 48
        }

        // Find corresponding index in 15-pitch array
        for (int i = 0; i < 15; i++) {
            if (noteMapping[i] == targetNote) {
                notes15 |= (1 << i);
                break;
            }
        }
    }

    return notes15;
}

const char* getKey(const int transpose)
{
    const char* KEYS[] = {
        "C/Am",
        "Db/Bbm",
        "D/Bm",
        "Eb/Cm",
        "E/C#m",
        "F/Dm",
        "F#/D#m",
        "G/Em",
        "Ab/Fm",
        "A/F#m",
        "Bb/Gm",
        "B/G#m",
    };
    return KEYS[(-transpose + 24) % 12];
}

void drawKeyboard(const int transpose)
{
    constexpr int startY = 148;
    constexpr int width = 320;
    constexpr int whiteKeyHeight = 60;
    constexpr int blackKeyHeight = 40;

    // White keys (C3-C5)
    const int whiteKeyNotes[] = {
        36, 38, 40, 41, 43, 45, 47, 48, 50, 52, 53, 55, 57, 59,
        60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81, 83, 84,
    };
    constexpr int numWhiteKeys = sizeof(whiteKeyNotes) / sizeof(whiteKeyNotes[0]);

    // Black keys (C#3-A#5)
    const int blackKeyNotes[] = {
        37, 39, 42, 44, 46, 49, 51, 54, 56, 58,
        61, 63, 66, 68, 70, 73, 75, 78, 80, 82,
    };
    // Black key positions relative to white keys
    const int blackKeyPositions[] = {
        0, 1, 3, 4, 5, 7, 8, 10, 11, 12,
        14, 15, 17, 18, 19, 21, 22, 24, 25, 26,
    };
    constexpr int numBlackKeys = sizeof(blackKeyNotes) / sizeof(blackKeyNotes[0]);

    // Valid keys (C3 to C5)
    const int validKeyNotes[] = {
        48, 50, 52, 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72,
    };

    constexpr int whiteKeyWidth = width / numWhiteKeys;
    constexpr int blackKeyWidth = whiteKeyWidth * 2 / 3;

    bool activeNotes[MAX_NOTES] = {false};
    for (const int validKeyNote : validKeyNotes) {
        activeNotes[validKeyNote - transpose] = true;
    }

    for (int i = 0; i < numWhiteKeys; i++) {
        const int note = whiteKeyNotes[i];
        const int x = i * whiteKeyWidth;
        uint16_t color = activeNotes[note] ? TFT_CYAN : TFT_WHITE;
        M5.Display.fillRect(x, startY, whiteKeyWidth - 1, whiteKeyHeight, color);
        M5.Display.drawRect(x, startY, whiteKeyWidth - 1, whiteKeyHeight, TFT_BLACK);
    }
    for (int i = 0; i < numBlackKeys; i++) {
        const int pos = blackKeyPositions[i];
        const int note = blackKeyNotes[i];
        const int x = pos * whiteKeyWidth + whiteKeyWidth - blackKeyWidth / 2;
        uint16_t color = activeNotes[note] ? TFT_CYAN : TFT_BLACK;
        M5.Display.fillRect(x, startY, blackKeyWidth, blackKeyHeight, color);
        M5.Display.drawRect(x, startY, blackKeyWidth, blackKeyHeight, TFT_BLACK);
    }
}
