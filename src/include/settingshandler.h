#ifndef SETTINGS_HANDLER_H
#define SETTINGS_HANDLER_H

#include <QSettings>

#include <QVector>

class SettingsUser;
class MandelbrotWidget;

class SettingsHandler
{
public:
    explicit SettingsHandler(MandelbrotWidget* owner);

    QSettings &getSettings();

    void registerSettingsUser(SettingsUser* settingsReceiver);

    void notifyUsersSettingsUpdatedByOwner(SettingsUser* ownerReceiver);

    void reset();

    void getXYSettingsData(QString& Xvalue, QString& Yvalue);

private:
    void notifyUsersSettingsUpdated();
    static constexpr enum QSettings::Format settingsFormat = QSettings::IniFormat;

	QVector<SettingsUser*> settingsReceivers;

    QSettings settings;
    SettingsUser* topSharer;

};

#endif // SETTINGS_HANDLER_H
