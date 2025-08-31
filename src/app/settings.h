#if !defined(APP_SETTINGS_H)
#define APP_SETTINGS_H

enum class SettingType
{
    NONE = 0,
    MAPPING = 1,
    TRANSPOSE = 2,
    SUSTAIN = 3,
    COUNT = 4,
};

class Settings
{
public:
    Settings();

    SettingType getSettingType() const { return _settingType; }
    bool isSettingsMode() const { return _settingType != SettingType::NONE; }
    int getMapping() const { return _mapping; }
    int getTranspose() const { return _transpose; }
    bool getSustain() const { return _sustain; }

    bool processButtons(bool btnPressedA, bool btnPressedB, bool btnPressedC);

    bool operator==(const Settings& other) const;
    bool operator!=(const Settings& other) const;

private:
    SettingType _settingType;
    int _mapping;
    int _transpose;
    bool _sustain;
};

void drawSettings(const Settings& settings);

#endif // !defined(APP_SETTINGS_H)
