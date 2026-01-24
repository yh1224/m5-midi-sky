#if defined(CONTROLLER_USB_KEYBOARD)

#include <M5Unified.h>
#include <USB.h>
#include <USBHIDKeyboard.h>

#include <map>

#include "app/controller.h"

// Maximum simultaneous notes
static constexpr int MAX_SIMULTANEOUS_NOTES = 5;

// Mapping table
static const std::map<int, char> mappings[] = {
    // Mapping 1
    {
        {0, 'y'},
        {2, 'u'},
        {4, 'i'},
        {5, 'o'},
        {7, 'p'},
        {9, 'h'},
        {11, 'j'},
        {12, 'k'},
        {14, 'l'},
        {16, ';'},
        {17, 'n'},
        {19, 'm'},
        {21, ','},
        {23, '.'},
        {24, '/'},
    },
    // Mapping 2
    {
        {0, 'y'},
        {2, 'u'},
        {4, 'i'},
        {5, 'o'},
        {7, 'p'},
        {9, 'h'},
        {11, 'j'},
        {12, 'k'},
        {14, 'l'},
        {16, ';'},
        {17, 'n'},
        {19, 'm'},
        {21, ','},
        {23, '.'},
        {24, '/'},
    },
};

// keyboard device instance
static USBHIDKeyboard keyboard;

// Filter to prevent old notes from reappearing
static NotesFilter noteFilter;

// Previous state
static Notes prevNotes{25};

static void applyMIDIToUSBKeyboard(const Notes& notes, const int mapping)
{
    // Get mapping
    const std::map<int, char>& currentMapping = mappings[mapping - 1];

    // Limit to latest keys for USB keyboard
    const Notes latestNotes = noteFilter.latest(notes, MAX_SIMULTANEOUS_NOTES);

    for (const auto& [noteIndex, key] : currentMapping) {
        const bool currentState = latestNotes.get(noteIndex) != 0;
        const bool prevState = prevNotes.get(noteIndex) != 0;

        // Send key event only when state changes
        if (currentState && !prevState) {
            // Key pressed
            keyboard.press(key);
        } else if (!currentState && prevState) {
            // Key released
            keyboard.release(key);
        }
    }

    // Update previous state
    prevNotes = notes;
}

void updateController(const Notes& notes, const int mapping)
{
    M5.Display.setCursor(0, 0);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.println("USB Keyboard");

    // MIDI to keyboard processing
    applyMIDIToUSBKeyboard(notes, mapping);
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
