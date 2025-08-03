#if defined(CONTROLLER_USB_KEYBOARD)

#include <M5Unified.h>
#include <USB.h>
#include <USBHIDKeyboard.h>

#include "app/controller.h"

// Maximum simultaneous notes
static constexpr int MAX_SIMULTANEOUS_NOTES = 5;

// Keyboard mapping entry structure
struct MappingEntry
{
    char key;
};

// Mapping 1
const MappingEntry mapping1[15] = {
    {'y'},
    {'u'},
    {'i'},
    {'o'},
    {'p'},
    {'h'},
    {'j'},
    {'k'},
    {'l'},
    {';'},
    {'n'},
    {'m'},
    {','},
    {'.'},
    {'/'},
};

// Mapping table
const MappingEntry* mappings[] = {
    mapping1,
    mapping1,
};

// keyboard device instance
static USBHIDKeyboard keyboard;

// Filter to prevent old notes from reappearing
static Notes15Filter noteFilter;

// Previous state
static Notes15 prevNotes15;

static void applyMIDIToUSBKeyboard(const Notes15& notes15, const int mapping)
{
    // Get mapping
    const MappingEntry* currentMapping = mappings[mapping - 1];

    // Limit to latest keys for USB keyboard
    const Notes15 latestNotes15 = noteFilter.latest(notes15, MAX_SIMULTANEOUS_NOTES);

    // Process 15-pitch array
    for (int i = 0; i < 15; i++) {
        const bool currentState = latestNotes15.get(i) != 0;
        const bool prevState = prevNotes15.get(i) != 0;

        // Send key event only when state changes
        if (currentState && !prevState) {
            // Key pressed
            keyboard.press(currentMapping[i].key);
        } else if (!currentState && prevState) {
            // Key released
            keyboard.release(currentMapping[i].key);
        }
    }

    // Update previous state
    prevNotes15 = notes15;
}

void updateController(const Notes15& notes15, const int mapping)
{
    M5.Display.setCursor(0, 0);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.println("USB Keyboard");

    // MIDI to keyboard processing
    applyMIDIToUSBKeyboard(notes15, mapping);
}

void setupController(const char* deviceName, const char* deviceManufacturer)
{
    USB.productName(deviceName);
    USB.manufacturerName(deviceManufacturer);

    // Initialize USB keyboard
    keyboard.begin();
    USB.begin();
    delay(3000);
}

#endif // defined(CONTROLLER_USB_KEYBOARD)
