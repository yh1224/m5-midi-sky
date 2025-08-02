#if defined(CONTROLLER_USB_NSWITCH)

#include <M5Unified.h>
#include <switch_ESP32.h>

#include "app/controller.h"

static NSGamepad gamepad;

// Gamepad action type constants
static constexpr int ACTION_BUTTON = 1;
static constexpr int ACTION_DPAD = 2;
static constexpr int ACTION_L_TRIGGER = 3;
static constexpr int ACTION_R_TRIGGER = 4;
static constexpr int ACTION_L_STICK = 5;
static constexpr int ACTION_R_STICK = 6;

// Direction constants (common for stick and DPAD)
static constexpr int DIRECTION_LEFT = 1;
static constexpr int DIRECTION_RIGHT = 2;
static constexpr int DIRECTION_UP = 3;
static constexpr int DIRECTION_DOWN = 4;

// Mapping entry structure
struct MappingEntry
{
    int type;
    int value;
};

// Mapping 1
const MappingEntry mapping1[15] = {
    {ACTION_L_TRIGGER, 255}, // ZL
    {ACTION_R_TRIGGER, 255}, // ZR
    {ACTION_DPAD, DIRECTION_DOWN}, // D-Pad ↓
    {ACTION_BUTTON, NSButton_B}, // B
    {ACTION_DPAD, DIRECTION_LEFT}, // D-Pad ←
    {ACTION_BUTTON, NSButton_Y}, // Y
    {ACTION_DPAD, DIRECTION_UP}, // D-Pad ↑
    {ACTION_BUTTON, NSButton_X}, // X
    {ACTION_DPAD, DIRECTION_RIGHT}, // D-Pad →
    {ACTION_BUTTON, NSButton_A}, // A
    {ACTION_BUTTON, NSButton_LeftTrigger}, // L
    {ACTION_BUTTON, NSButton_RightTrigger}, // R
    {ACTION_L_STICK, DIRECTION_LEFT}, // L-Stick ←
    {ACTION_R_STICK, DIRECTION_LEFT}, // R-Stick ←
    {ACTION_L_STICK, DIRECTION_RIGHT}, // L-Stick →
};

// Mapping 2
const MappingEntry mapping2[15] = {
    {ACTION_DPAD, DIRECTION_DOWN}, // D-Pad ↓
    {ACTION_DPAD, DIRECTION_LEFT}, // D-Pad ←
    {ACTION_DPAD, DIRECTION_UP}, // D-Pad ↑
    {ACTION_L_STICK, DIRECTION_DOWN}, // L-Stick ↓
    {ACTION_L_STICK, DIRECTION_LEFT}, // L-Stick ←
    {ACTION_BUTTON, NSButton_LeftTrigger}, // L
    {ACTION_L_TRIGGER, 255}, // ZL
    {ACTION_R_STICK, DIRECTION_DOWN}, // R-Stick ↓
    {ACTION_R_STICK, DIRECTION_RIGHT}, // R-Stick →
    {ACTION_R_STICK, DIRECTION_UP}, // R-Stick ↑
    {ACTION_BUTTON, NSButton_B}, // B
    {ACTION_BUTTON, NSButton_A}, // A
    {ACTION_BUTTON, NSButton_X}, // X
    {ACTION_BUTTON, NSButton_RightTrigger}, // R
    {ACTION_R_TRIGGER, 255}, // ZR
};

// Mapping table
const MappingEntry* mappings[] = {
    mapping1,
    mapping2,
};

static void applyMIDIToNSwitchGamepad(const Notes15& notes15, const int mapping)
{
    // Get mapping
    const MappingEntry* currentMapping = mappings[mapping - 1];

    // Variables for accumulating stick input (Nintendo Switch typically uses 0-255 range)
    uint8_t leftStickX = 128, leftStickY = 128; // Center position
    uint8_t rightStickX = 128, rightStickY = 128; // Center position

    // DPad state
    bool dpadPressed[4] = {false}; // UP, DOWN, RIGHT, LEFT

    // Clear button state
    gamepad.releaseAll();

    // Process 15-pitch array
    for (int i = 0; i < 15; i++) {
        if (notes15.get(i) != 0) {
            const MappingEntry& mappingEntry = currentMapping[i];
            switch (mappingEntry.type) {
            case ACTION_BUTTON:
                gamepad.press(mappingEntry.value);
                break;
            case ACTION_DPAD:
                // Record DPad state
                if (mappingEntry.value == DIRECTION_UP) dpadPressed[0] = true;
                else if (mappingEntry.value == DIRECTION_DOWN) dpadPressed[1] = true;
                else if (mappingEntry.value == DIRECTION_RIGHT) dpadPressed[2] = true;
                else if (mappingEntry.value == DIRECTION_LEFT) dpadPressed[3] = true;
                break;
            case ACTION_L_TRIGGER:
                gamepad.press(NSButton_LeftThrottle);
                break;
            case ACTION_R_TRIGGER:
                gamepad.press(NSButton_RightThrottle);
                break;
            case ACTION_L_STICK:
                if (mappingEntry.value == DIRECTION_LEFT) {
                    leftStickX = 0; // L-Stick ←
                } else if (mappingEntry.value == DIRECTION_RIGHT) {
                    leftStickX = 255; // L-Stick →
                } else if (mappingEntry.value == DIRECTION_UP) {
                    leftStickY = 0; // L-Stick ↑
                } else if (mappingEntry.value == DIRECTION_DOWN) {
                    leftStickY = 255; // L-Stick ↓
                }
                break;
            case ACTION_R_STICK:
                if (mappingEntry.value == DIRECTION_LEFT) {
                    rightStickX = 0; // R-Stick ←
                } else if (mappingEntry.value == DIRECTION_RIGHT) {
                    rightStickX = 255; // R-Stick →
                } else if (mappingEntry.value == DIRECTION_UP) {
                    rightStickY = 0; // R-Stick ↑
                } else if (mappingEntry.value == DIRECTION_DOWN) {
                    rightStickY = 255; // R-Stick ↓
                }
                break;
            default:
                break;
            }
        }
    }

    // Set DPad direction
    const bool north = dpadPressed[0];
    const bool south = dpadPressed[1];
    const bool east = dpadPressed[2];
    const bool west = dpadPressed[3];

    if (north && east) {
        gamepad.dPad(NSGAMEPAD_DPAD_UP_RIGHT);
    } else if (north && west) {
        gamepad.dPad(NSGAMEPAD_DPAD_UP_LEFT);
    } else if (south && east) {
        gamepad.dPad(NSGAMEPAD_DPAD_DOWN_RIGHT);
    } else if (south && west) {
        gamepad.dPad(NSGAMEPAD_DPAD_DOWN_LEFT);
    } else if (north) {
        gamepad.dPad(NSGAMEPAD_DPAD_UP);
    } else if (south) {
        gamepad.dPad(NSGAMEPAD_DPAD_DOWN);
    } else if (east) {
        gamepad.dPad(NSGAMEPAD_DPAD_RIGHT);
    } else if (west) {
        gamepad.dPad(NSGAMEPAD_DPAD_LEFT);
    } else {
        gamepad.dPad(NSGAMEPAD_DPAD_CENTERED);
    }

    // Set stick positions
    gamepad.leftXAxis(leftStickX);
    gamepad.leftYAxis(leftStickY);
    gamepad.rightXAxis(rightStickX);
    gamepad.rightYAxis(rightStickY);

    // Send report
    gamepad.loop();
}

void updateController(const Notes15& notes15, const int mapping)
{
    M5.Display.setCursor(0, 0);
    M5.Display.setTextColor(TFT_PURPLE, TFT_BLACK);
    M5.Display.println("USB Nintendo Switch");

    // MIDI to gamepad processing
    applyMIDIToNSwitchGamepad(notes15, mapping);
}

void setupController(const char* deviceName, const char* deviceManufacturer)
{
    // Initialize Nintendo Switch controller
    gamepad.begin();
    USB.begin();
    delay(3000);
}

#endif // defined(CONTROLLER_USB_NSWITCH)
