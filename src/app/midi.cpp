#include <M5Unified.h>
#include <MIDI.h>

#include "app/midi.h"

// Total number of MIDI notes (0-127)
static constexpr int MAX_MIDI_NOTES = 128;

// Duration to temporarily turn off key during repress
static constexpr unsigned long REPRESS_KEY_OFF_DURATION_MS = 50;

// Global flag to enable/disable sustain pedal processing
static bool sustainEnabled = false;

// Key states - timestamp when each note was last pressed (0 = not pressed)
static unsigned long notes[MAX_MIDI_NOTES] = {0};

// Physical key press state (true = physically pressed)
static bool physicallyPressed[MAX_MIDI_NOTES] = {false};

// Timestamps for repressed keys (milliseconds)
static unsigned long repressedTime[MAX_MIDI_NOTES] = {};

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
                    if (0 <= noteNum && noteNum < MAX_MIDI_NOTES) {
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
                    if (0 <= noteNum && noteNum < MAX_MIDI_NOTES) {
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
                            for (int i = 0; i < MAX_MIDI_NOTES; i++) {
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

    memset(notes, 0, sizeof(unsigned long) * MAX_MIDI_NOTES);
    memset(physicallyPressed, false, sizeof(bool) * MAX_MIDI_NOTES);
    memset(repressedTime, 0, sizeof(unsigned long) * MAX_MIDI_NOTES);
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
        for (int i = 0; i < MAX_MIDI_NOTES; i++) {
            if (notes[i] != 0 && !physicallyPressed[i]) {
                notes[i] = 0;
            }
        }
    }
}

Notes getNotes(const int numNotes, const int noteMapping[], const int baseNote, const bool expand)
{
    // Initialize output array to 0 (not pressed)
    auto timestamps = std::make_unique<unsigned long[]>(numNotes);
    for (int i = 0; i < numNotes; i++) {
        timestamps[i] = 0;
    }

    int maxNote = 0;
    for (int i = 0; i < numNotes; i++) {
        if (maxNote < noteMapping[i]) {
            maxNote = noteMapping[i];
        }
    }

    const unsigned long currentTime = millis();
    for (int midiNote = 0; midiNote < MAX_MIDI_NOTES; midiNote++) {
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
        int targetNote = midiNote - baseNote;

        // Expand
        if (expand) {
            // map all notes
            while (targetNote < 0) {
                targetNote += 12; // +1 octave
            }
            while (targetNote > maxNote) {
                targetNote -= 12; // -1 octave
            }
        } else if (targetNote < 0 || targetNote > maxNote) {
            // ignore outside
            continue;
        }

        // Find corresponding index in pitch array
        for (int i = 0; i < numNotes; i++) {
            if (noteMapping[i] == targetNote) {
                // Keep the latest timestamp for each position
                if (timestamps[i] == 0 || notes[midiNote] > timestamps[i]) {
                    timestamps[i] = notes[midiNote];
                }
                break;
            }
        }
    }

    return Notes(numNotes, timestamps.get());
}
