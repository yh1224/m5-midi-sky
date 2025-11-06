#include <M5Unified.h>

#include "display.h"
#include "midi.h"
#include "settings.h"

// Settings constants
constexpr int MAPPING_MIN = 1;
constexpr int MAPPING_MAX = 2;
constexpr int MAPPING_DEFAULT = 1;
constexpr int BASENOTE_MIN = 24; // C1
constexpr int BASENOTE_MAX = 84; // C6
constexpr int BASENOTE_DEFAULT = 48; // C3
constexpr bool SUSTAIN_DEFAULT = false;

Settings::Settings()
    : _settingType(SettingType::NONE),
      _mapping(MAPPING_DEFAULT),
      _baseNote(BASENOTE_DEFAULT),
      _sustain(SUSTAIN_DEFAULT)
{
}

bool Settings::operator==(const Settings& other) const
{
    return _settingType == other._settingType &&
        _mapping == other._mapping &&
        _baseNote == other._baseNote &&
        _sustain == other._sustain;
}

bool Settings::operator!=(const Settings& other) const
{
    return !(*this == other);
}

bool Settings::processButtons(const bool btnPressedA, const bool btnPressedB, const bool btnPressedC)
{
    bool changed = false;

    // Button B: Switch setting
    if (btnPressedB) {
        _settingType = static_cast<SettingType>((static_cast<int>(_settingType) + 1) %
            static_cast<int>(SettingType::COUNT));
        if (_settingType == SettingType::NONE) {
            M5.Speaker.tone(1000, 500);
        } else {
            M5.Speaker.tone(1000, 100);
        }
        changed = true;
    }

    // Button A/C: Change current setting value (only if a setting is selected)
    if ((btnPressedA || btnPressedC) && _settingType != SettingType::NONE) {
        switch (_settingType) {
        case SettingType::MAPPING:
            M5.Speaker.tone(2000, 100);
            if (btnPressedA && _mapping > MAPPING_MIN) {
                _mapping--;
            }
            if (btnPressedC && _mapping < MAPPING_MAX) {
                _mapping++;
            }
            changed = true;
            break;
        case SettingType::BASENOTE:
            M5.Speaker.tone(2000, 100);
            if (btnPressedA && _baseNote > BASENOTE_MIN) {
                _baseNote--;
            }
            if (btnPressedC && _baseNote < BASENOTE_MAX) {
                _baseNote++;
            }
            changed = true;
            break;
        case SettingType::SUSTAIN:
            M5.Speaker.tone(2000, 100);
            if (btnPressedA || btnPressedC) {
                _sustain = !_sustain;
                setSustainEnabled(_sustain);
            }
            changed = true;
            break;
        default:
            break;
        }
    }

    return changed;
}

static const char* getBaseNote(const int baseNote)
{
    const char* KEYS[] = {
        "C",
        "C#",
        "D",
        "D#",
        "E",
        "F",
        "F#",
        "G",
        "G#",
        "A",
        "A#",
        "B",
    };
    return KEYS[baseNote % 12];
}

static const char* getKey(const int baseNote)
{
    const char* KEYS[] = {
        "C/Am",
        "Db/Bbm",
        "D/Bm",
        "Eb/Cm",
        "E/C#m",
        "F/Dm",
        "F#/D#m",
        "G/Em",
        "Ab/Fm",
        "A/F#m",
        "Bb/Gm",
        "B/G#m",
    };
    return KEYS[baseNote % 12];
}

void drawSettings(const Settings& settings)
{
    const auto settingType = settings.getSettingType();

    M5.Display.fillRect(0, 48, 320, static_cast<int>(SettingType::COUNT) * 16, TFT_BLACK);
    M5.Display.setCursor(0, 48);

    M5.Display.setTextColor(settingType == SettingType::MAPPING ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    M5.Display.printf("Mapping: %d\n", settings.getMapping());

    M5.Display.setTextColor(settingType == SettingType::BASENOTE ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    M5.Display.printf("Base note: %s%d (%s)\n", getBaseNote(settings.getBaseNote()),
                      settings.getBaseNote() / 12 - 1, getKey(settings.getBaseNote()));

    M5.Display.setTextColor(settingType == SettingType::SUSTAIN ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    M5.Display.printf("Sustain: %s\n", settings.getSustain() ? "ON" : "OFF");

    drawKeyboard(128, 320, 60, settings.getBaseNote());
}
