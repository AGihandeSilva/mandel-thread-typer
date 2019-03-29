#ifndef SETTINGSUSER_H
#define SETTINGSUSER_H

#include <QSettings>

class SettingsUser
{
public:
    SettingsUser() = default;
    virtual ~SettingsUser() = default;
    SettingsUser(const SettingsUser&) = delete;
    SettingsUser(SettingsUser&&) = delete;
    SettingsUser& operator=(const SettingsUser&) = delete;
    SettingsUser& operator=(SettingsUser&&) = delete;

    virtual void processSettingUpdate(QSettings& settings) = 0 ;
};

#endif // SETTINGSUSER_H
