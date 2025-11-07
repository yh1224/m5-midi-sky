#if !defined(APP_MIDI_H)
#define APP_MIDI_H

#include "app/notes.h"

void setupMIDI(int8_t rxPin, int8_t txPin);

void setSustainEnabled(bool enabled);

Notes15 getNotes15(int baseNote, bool expand);

void drawKeyboard(int startY, int width, int height, int baseNote);

#endif // !defined(APP_MIDI_H)
