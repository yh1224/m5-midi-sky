#if !defined(APP_DISPLAY_H)
#define APP_DISPLAY_H

#include "app/notes.h"

void resetNotes();

void drawNotes(const Notes15& notes15, int startY, int width, int height, int spacing);

void drawButtons(int startY, int width, int height, bool withSelect);

#endif // !defined(APP_DISPLAY_H)
