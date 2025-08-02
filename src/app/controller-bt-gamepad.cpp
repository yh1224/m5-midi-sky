#if defined(CONTROLLER_BT_GAMEPAD)

#include <M5Unified.h>
#include <BleCompositeHID.h>
#include <XboxGamepadDevice.h>
#include <XboxGamepadConfiguration.h>

#include "app/controller.h"

static BleCompositeHID* bleHID;
static XboxGamepadDevice* gamepad;

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
    // ACTION_NONE, ACTION_BUTTON, ACTION_DPAD, ACTION_L_TRIGGER, ACTION_R_TRIGGER, ACTION_L_STICK, ACTION_R_STICK
    int value;
};

// Mapping 1
const MappingEntry mapping1[15] = {
    {ACTION_L_TRIGGER, 1023}, // LT
    {ACTION_R_TRIGGER, 1023}, // RT
    {ACTION_DPAD, DIRECTION_DOWN}, // D-Pad ↓
    {ACTION_BUTTON, XBOX_BUTTON_A}, // A
    {ACTION_DPAD, DIRECTION_LEFT}, // D-Pad ←
    {ACTION_BUTTON, XBOX_BUTTON_X}, // X
    {ACTION_DPAD, DIRECTION_UP}, // D-Pad ↑
    {ACTION_BUTTON, XBOX_BUTTON_Y}, // Y
    {ACTION_DPAD, DIRECTION_RIGHT}, // D-Pad →
    {ACTION_BUTTON, XBOX_BUTTON_B}, // B
    {ACTION_BUTTON, XBOX_BUTTON_LB}, // LB
    {ACTION_BUTTON, XBOX_BUTTON_RB}, // RB
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
    {ACTION_BUTTON, XBOX_BUTTON_LB}, // LB
    {ACTION_L_TRIGGER, 1023}, // LT
    {ACTION_R_STICK, DIRECTION_DOWN}, // R-Stick ↓
    {ACTION_R_STICK, DIRECTION_RIGHT}, // R-Stick →
    {ACTION_R_STICK, DIRECTION_UP}, // R-Stick ↑
    {ACTION_BUTTON, XBOX_BUTTON_A}, // A
    {ACTION_BUTTON, XBOX_BUTTON_B}, // B
    {ACTION_BUTTON, XBOX_BUTTON_Y}, // Y
    {ACTION_BUTTON, XBOX_BUTTON_RB}, // RB
    {ACTION_R_TRIGGER, 1023}, // RT
};

// Mapping table
const MappingEntry* mappings[] = {
    mapping1,
    mapping2,
};

static void applyMIDIToGamepad(const Notes15& notes15, const int mapping)
{
    // Get mapping
    const MappingEntry* currentMapping = mappings[mapping - 1];

    gamepad->resetInputs();

    // Variables for accumulating stick input
    int16_t leftThumbX = 0, leftThumbY = 0;
    int16_t rightThumbX = 0, rightThumbY = 0;

    // DPad state
    bool dpadPressed[4] = {false}; // NORTH, SOUTH, EAST, WEST

    for (int i = 0; i < 15; i++) {
        if (notes15.get(i) != 0) {
            const MappingEntry& mappingEntry = currentMapping[i];
            switch (mappingEntry.type) {
            case ACTION_BUTTON:
                gamepad->press(mappingEntry.value);
                break;
            case ACTION_DPAD:
                // Record DPad state
                if (mappingEntry.value == DIRECTION_UP) dpadPressed[0] = true;
                else if (mappingEntry.value == DIRECTION_DOWN) dpadPressed[1] = true;
                else if (mappingEntry.value == DIRECTION_RIGHT) dpadPressed[2] = true;
                else if (mappingEntry.value == DIRECTION_LEFT) dpadPressed[3] = true;
                break;
            case ACTION_L_TRIGGER:
                gamepad->setLeftTrigger(mappingEntry.value);
                break;
            case ACTION_R_TRIGGER:
                gamepad->setRightTrigger(mappingEntry.value);
                break;
            case ACTION_L_STICK:
                if (mappingEntry.value == DIRECTION_LEFT) {
                    leftThumbX = -32768; // L-Stick ←
                } else if (mappingEntry.value == DIRECTION_RIGHT) {
                    leftThumbX = 32767; // L-Stick →
                } else if (mappingEntry.value == DIRECTION_UP) {
                    leftThumbY = 32767; // L-Stick ↑
                } else if (mappingEntry.value == DIRECTION_DOWN) {
                    leftThumbY = -32768; // L-Stick ↓
                }
                break;
            case ACTION_R_STICK:
                if (mappingEntry.value == DIRECTION_LEFT) {
                    rightThumbX = -32768; // R-Stick ←
                } else if (mappingEntry.value == DIRECTION_RIGHT) {
                    rightThumbX = 32767; // R-Stick →
                } else if (mappingEntry.value == DIRECTION_UP) {
                    rightThumbY = 32767; // R-Stick ↑
                } else if (mappingEntry.value == DIRECTION_DOWN) {
                    rightThumbY = -32768; // R-Stick ↓
                }
                break;
            default:
                break;
            }
        }
    }

    // Set DPad direction (diagonal directions supported)
    const bool north = dpadPressed[0];
    const bool south = dpadPressed[1];
    const bool east = dpadPressed[2];
    const bool west = dpadPressed[3];

    if (north && east) {
        gamepad->pressDPadDirection(XBOX_BUTTON_DPAD_NORTHEAST);
    } else if (north && west) {
        gamepad->pressDPadDirection(XBOX_BUTTON_DPAD_NORTHWEST);
    } else if (south && east) {
        gamepad->pressDPadDirection(XBOX_BUTTON_DPAD_SOUTHEAST);
    } else if (south && west) {
        gamepad->pressDPadDirection(XBOX_BUTTON_DPAD_SOUTHWEST);
    } else if (north) {
        gamepad->pressDPadDirection(XBOX_BUTTON_DPAD_NORTH);
    } else if (south) {
        gamepad->pressDPadDirection(XBOX_BUTTON_DPAD_SOUTH);
    } else if (east) {
        gamepad->pressDPadDirection(XBOX_BUTTON_DPAD_EAST);
    } else if (west) {
        gamepad->pressDPadDirection(XBOX_BUTTON_DPAD_WEST);
    }

    // Set stick input
    gamepad->setLeftThumb(leftThumbX, leftThumbY);
    gamepad->setRightThumb(rightThumbX, rightThumbY);

    gamepad->sendGamepadReport();
}

void updateController(const Notes15& notes15, const int mapping)
{
    M5.Display.setCursor(0, 0);
    M5.Display.setTextColor(TFT_BLUE, TFT_BLACK);
    M5.Display.println("Bluetooth Gamepad");

    // Check BLE connection status
    if (bleHID->isConnected()) {
        M5.Display.println("Connected       ");

        applyMIDIToGamepad(notes15, mapping);
    } else {
        M5.Display.println("                ");
    }
}

void setupController(const char* deviceName, const char* deviceManufacturer)
{
    bleHID = new BleCompositeHID(deviceName, deviceManufacturer, 100);

    // Use Xbox One S controller settings
    const auto deviceConfig = new XboxOneSControllerDeviceConfiguration();
    gamepad = new XboxGamepadDevice(deviceConfig);
    const auto hostConfig = deviceConfig->getIdealHostConfiguration();
    bleHID->addDevice(gamepad);
    bleHID->begin(hostConfig);
}

#endif // defined(CONTROLLER_BT_GAMEPAD)
