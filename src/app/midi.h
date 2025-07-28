#if !defined(APP_MIDI_H)
#define APP_MIDI_H

void drawKeyboard(int transpose);

void setupMIDI(int8_t rxPin, int8_t txPin);

uint16_t getNotes15(int transpose);

const char* getKey(int transpose);

#endif // !defined(APP_MIDI_H)
