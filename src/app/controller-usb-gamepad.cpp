#if defined(CONTROLLER_USB_GAMEPAD)

#include <M5Unified.h>
#include <USB.h>
#include <USBHIDGamepad.h>

#include "app/controller.h"

#define GAMEPAD_VID 0x046D    // Logitech
#define GAMEPAD_PID 0xc216    // Logitech F310 Gamepad

static USBHIDGamepad gamepad;

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

// Button constants
static constexpr int D_BUTTON_X = 1;
static constexpr int D_BUTTON_A = 2;
static constexpr int D_BUTTON_B = 3;
static constexpr int D_BUTTON_Y = 4;
static constexpr int D_BUTTON_L = 5;
static constexpr int D_BUTTON_R = 6;
static constexpr int D_BUTTON_ZL = 7;
static constexpr int D_BUTTON_ZR = 8;

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
    {ACTION_BUTTON, D_BUTTON_A}, // A
    {ACTION_DPAD, DIRECTION_LEFT}, // D-Pad ←
    {ACTION_BUTTON, D_BUTTON_X}, // X
    {ACTION_DPAD, DIRECTION_UP}, // D-Pad ↑
    {ACTION_BUTTON, D_BUTTON_Y}, // Y
    {ACTION_DPAD, DIRECTION_RIGHT}, // D-Pad →
    {ACTION_BUTTON, D_BUTTON_B}, // B
    {ACTION_BUTTON, D_BUTTON_L}, // L
    {ACTION_BUTTON, D_BUTTON_R}, // R
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
    {ACTION_BUTTON, D_BUTTON_L}, // L
    {ACTION_L_TRIGGER, 255}, // ZL
    {ACTION_R_STICK, DIRECTION_DOWN}, // R-Stick ↓
    {ACTION_R_STICK, DIRECTION_RIGHT}, // R-Stick →
    {ACTION_R_STICK, DIRECTION_UP}, // R-Stick ↑
    {ACTION_BUTTON, D_BUTTON_A}, // A
    {ACTION_BUTTON, D_BUTTON_B}, // B
    {ACTION_BUTTON, D_BUTTON_Y}, // Y
    {ACTION_BUTTON, D_BUTTON_R}, // R
    {ACTION_R_TRIGGER, 255}, // ZR
};

// Mapping table
const MappingEntry* mappings[] = {
    mapping1,
    mapping2,
};

static void applyMIDIToUSBGamepad(const uint16_t notes15, const int mapping)
{
    // Get mapping
    const MappingEntry* currentMapping = mappings[mapping - 1];

    // Variables for accumulating stick input (-127 to 127 range for USB HID)
    int8_t leftThumbX = 0, leftThumbY = 0;
    int8_t rightThumbX = 0, rightThumbY = 0;

    // DPad state
    bool dpadPressed[4] = {false}; // UP, DOWN, RIGHT, LEFT

    // Button state
    uint32_t buttons = 0;

    // Process 15-pitch array
    for (int i = 0; i < 15; i++) {
        if (notes15 & (1 << i)) {
            const MappingEntry& mappingEntry = currentMapping[i];
            switch (mappingEntry.type) {
            case ACTION_BUTTON:
                buttons |= (1UL << (mappingEntry.value - 1));
                break;
            case ACTION_DPAD:
                // Record DPad state
                if (mappingEntry.value == DIRECTION_UP) dpadPressed[0] = true;
                else if (mappingEntry.value == DIRECTION_DOWN) dpadPressed[1] = true;
                else if (mappingEntry.value == DIRECTION_RIGHT) dpadPressed[2] = true;
                else if (mappingEntry.value == DIRECTION_LEFT) dpadPressed[3] = true;
                break;
            case ACTION_L_TRIGGER:
                // In USB HID, treat triggers as buttons
                buttons |= (1UL << (D_BUTTON_ZL - 1));
                break;
            case ACTION_R_TRIGGER:
                // In USB HID, treat triggers as buttons
                buttons |= (1UL << (D_BUTTON_ZR - 1));
                break;
            case ACTION_L_STICK:
                if (mappingEntry.value == DIRECTION_LEFT) {
                    leftThumbX = -127; // L-Stick ←
                } else if (mappingEntry.value == DIRECTION_RIGHT) {
                    leftThumbX = 127; // L-Stick →
                } else if (mappingEntry.value == DIRECTION_UP) {
                    leftThumbY = 127; // L-Stick ↑
                } else if (mappingEntry.value == DIRECTION_DOWN) {
                    leftThumbY = -127; // L-Stick ↓
                }
                break;
            case ACTION_R_STICK:
                if (mappingEntry.value == DIRECTION_LEFT) {
                    rightThumbX = -127; // R-Stick ←
                } else if (mappingEntry.value == DIRECTION_RIGHT) {
                    rightThumbX = 127; // R-Stick →
                } else if (mappingEntry.value == DIRECTION_UP) {
                    rightThumbY = 127; // R-Stick ↑
                } else if (mappingEntry.value == DIRECTION_DOWN) {
                    rightThumbY = -127; // R-Stick ↓
                }
                break;
            default:
                break;
            }
        }
    }

    // Set DPad direction
    uint8_t hat = GAMEPAD_HAT_CENTERED;
    const bool north = dpadPressed[0];
    const bool south = dpadPressed[1];
    const bool east = dpadPressed[2];
    const bool west = dpadPressed[3];

    if (north && east) {
        hat = GAMEPAD_HAT_UP_RIGHT;
    } else if (north && west) {
        hat = GAMEPAD_HAT_UP_LEFT;
    } else if (south && east) {
        hat = GAMEPAD_HAT_DOWN_RIGHT;
    } else if (south && west) {
        hat = GAMEPAD_HAT_DOWN_LEFT;
    } else if (north) {
        hat = GAMEPAD_HAT_UP;
    } else if (south) {
        hat = GAMEPAD_HAT_DOWN;
    } else if (east) {
        hat = GAMEPAD_HAT_RIGHT;
    } else if (west) {
        hat = GAMEPAD_HAT_LEFT;
    }

    // Send input (x, y, rx, ry, z, rz, hat, buttons)
    gamepad.send(leftThumbX, leftThumbY, rightThumbX, rightThumbY, 0, 0, hat, buttons);
}

void updateController(uint16_t notes15, const int mapping)
{
    M5.Display.setCursor(0, 0);
    M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Display.println("USB Gamepad   ");

    // MIDI to gamepad processing
    applyMIDIToUSBGamepad(notes15, mapping);
}

void setupController(const char* deviceName, const char* deviceManufacturer)
{
    USB.VID(GAMEPAD_VID);
    USB.PID(GAMEPAD_PID);
    USB.productName(deviceName);
    USB.manufacturerName(deviceManufacturer);

    gamepad.begin();
    USB.begin();
    delay(3000);
}

#endif // defined(CONTROLLER_USB_GAMEPAD)
