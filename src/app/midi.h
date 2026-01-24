#if !defined(APP_MIDI_H)
#define APP_MIDI_H

#include "app/notes.h"

void setupMIDI(int8_t rxPin, int8_t txPin);

void setSustainEnabled(bool enabled);

Notes getNotes(int baseNote, int numNotes, bool expand);

#endif // !defined(APP_MIDI_H)
