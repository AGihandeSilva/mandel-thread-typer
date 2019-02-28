#ifndef SETTINGSUSER_H
#define SETTINGSUSER_H

#include <QSettings>

class SettingsUser
{
public:
    virtual ~SettingsUser() = default;
    virtual void processSettingUpdate(QSettings& settings) = 0 ;
};

#endif // SETTINGSUSER_H
