#include "settingshandler.h"
#include "settingsuser.h"
#include "mandelbrotwidget.h"

#include <iostream>

SettingsHandler::SettingsHandler(MandelbrotWidget* owner) :
    settings(settingsFormat,
              QSettings::UserScope,
              "ShinkuSoft",
              "mandelbrot",
              dynamic_cast<QObject*>(owner)),
    topSharer(dynamic_cast<SettingsUser*>(owner))
{
    settingsReceivers.clear();
}


QSettings& SettingsHandler::getSettings()
{
    return settings;
}

void SettingsHandler::registerSettingsUser(SettingsUser *settingsReceiver)
{
    settingsReceivers.push_back(settingsReceiver);

    if (settingsReceiver == topSharer)
    {
        //the last object to register must be the owner of this handler in this scheme
        notifyUsersSettingsUpdated();
    }
}

void SettingsHandler::notifyUsersSettingsUpdatedByOwner(SettingsUser *ownerReceiver)
{
    if (ownerReceiver == topSharer) {
       notifyUsersSettingsUpdated();
    } else {
        std::cout << "WARNING: settings update triggered by non-owner -> trigger ignored" << std::endl;
    }
}

void SettingsHandler::reset()
{
    settingsReceivers.clear();
	topSharer = nullptr;
}

void SettingsHandler::notifyUsersSettingsUpdated()
{
    settings.sync();
    for (auto& r : settingsReceivers) {
        r->processSettingUpdate(settings);
    }
}
