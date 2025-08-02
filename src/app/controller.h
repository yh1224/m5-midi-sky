#if !defined(APP_CONTROLLER_H)
#define APP_CONTROLLER_H

#include "app/notes.h"

void updateController(const Notes15& notes15, int mapping);

void setupController(const char *deviceName, const char *deviceManufacturer);

#endif // !defined(APP_CONTROLLER_H)
