#include <M5Unified.h>
#include <MIDI.h>

#include "app/midi.h"

// Total number of MIDI notes (0-127)
static constexpr int MAX_NOTES = 128;

// Duration to temporarily turn off key during repress
static constexpr unsigned long REPRESS_KEY_OFF_DURATION_MS = 50;

// Global flag to enable/disable sustain pedal processing
static bool sustainEnabled = false;

// Key states - timestamp when each note was last pressed (0 = not pressed)
static unsigned long notes[MAX_NOTES] = {0};

// Physical key press state (true = physically pressed)
static bool physicallyPressed[MAX_NOTES] = {false};

// Timestamps for repressed keys (milliseconds)
static unsigned long repressedTime[MAX_NOTES] = {};

// Sustain pedal state
static bool sustainPedal = false;

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

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
                        physicallyPressed[noteNum] = true;
                        notes[noteNum] = millis();
                        if (sustainEnabled && sustainPedal && notes[noteNum] != 0) {
                            // Re-press while pedal is down and note is sustained
                            repressedTime[noteNum] = millis();
                        } else {
                            repressedTime[noteNum] = 0;
                        }
                    }
                    break;
                }
            case midi::NoteOff:
                {
                    const int noteNum = MIDI.getData1();
                    if (0 <= noteNum && noteNum < MAX_NOTES) {
                        physicallyPressed[noteNum] = false;
                        if (sustainEnabled && sustainPedal && notes[noteNum] != 0) {
                            // Keep note sustained while pedal is down
                        } else {
                            notes[noteNum] = 0;
                        }
                        repressedTime[noteNum] = 0;
                    }
                    break;
                }
            case midi::ControlChange:
                {
                    const int ccNum = MIDI.getData1();
                    const int ccValue = MIDI.getData2();

                    // Sustain pedal (CC64)
                    if (ccNum == midi::MidiControlChangeNumber::Sustain && sustainEnabled) {
                        sustainPedal = ccValue >= 64;
                        // If sustain pedal is released, turn off sustained notes except physically pressed ones
                        if (!sustainPedal) {
                            for (int i = 0; i < MAX_NOTES; i++) {
                                if (notes[i] != 0 && !physicallyPressed[i]) {
                                    notes[i] = 0;
                                }
                            }
                        }
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
    Serial2.begin(31250, SERIAL_8N1, rxPin, txPin);

    memset(notes, 0, sizeof(unsigned long) * MAX_NOTES);
    memset(physicallyPressed, false, sizeof(bool) * MAX_NOTES);
    memset(repressedTime, 0, sizeof(unsigned long) * MAX_NOTES);
    sustainPedal = false;

    MIDI.turnThruOn();

    // Start MIDI receive task
    xTaskCreatePinnedToCore(
        midiTask,
        "midiTask",
        8192,
        nullptr,
        1,
        nullptr,
        0
    );
}

void setSustainEnabled(const bool enabled)
{
    sustainEnabled = enabled;

    // If sustain is disabled, immediately turn off all sustained notes
    if (!enabled && sustainPedal) {
        for (int i = 0; i < MAX_NOTES; i++) {
            if (notes[i] != 0 && !physicallyPressed[i]) {
                notes[i] = 0;
            }
        }
    }
}

Notes15 getNotes15(const int transpose)
{
    // 15 pitches from C3 to C5 (48-72)
    const static int noteMapping[15] = {
        48, 50, 52, 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72,
    };

    // Initialize output array to 0 (not pressed)
    unsigned long timestamps[15] = {0};

    const unsigned long currentTime = millis();
    for (int midiNote = 0; midiNote < MAX_NOTES; midiNote++) {
        if (notes[midiNote] == 0) {
            continue;
        }

        // Handle re-pressed state: return as off for first REPRESS_KEY_OFF_DURATION_MS, then continue normally
        if (repressedTime[midiNote] > 0) {
            if (currentTime - repressedTime[midiNote] >= REPRESS_KEY_OFF_DURATION_MS) {
                repressedTime[midiNote] = 0;
            } else {
                continue;
            }
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
                // Keep the latest timestamp for each position
                if (timestamps[i] == 0 || notes[midiNote] > timestamps[i]) {
                    timestamps[i] = notes[midiNote];
                }
                break;
            }
        }
    }

    return Notes15(timestamps);
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

void drawKeyboard(const int startY, const int width, const int height, const int transpose)
{
    const int whiteKeyHeight = height * 3 / 5;
    const int blackKeyHeight = height * 2 / 5;

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

    const int whiteKeyWidth = width / numWhiteKeys;
    const int blackKeyWidth = whiteKeyWidth * 2 / 3;

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
