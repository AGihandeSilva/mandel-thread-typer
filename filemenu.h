#ifndef FILEMENU_H
#define FILEMENU_H

#include <QMenu>
#include <QFileInfo>
#include <QFileDialog>
#include "settingsuser.h"

class QMenuBar;
class MandelbrotWidget;
class SettingsHandler;
class QAction;

struct FileMenuConfig
{
    QFileInfo* const currentFilePath;
    QFileInfo* const currentBitmapFilePath;
};


class FileMenu : public QMenu, public SettingsUser
{
    Q_OBJECT
public:
    explicit FileMenu(QMenuBar* parentMenu, MandelbrotWidget* plotHandler, SettingsHandler& applicationSettingsHandler);

    virtual void processSettingUpdate(QSettings& settings) override;

    FileMenuConfig getFileMenuConfig() const;

public slots:
    void quit();
    void openParams();
    void saveParams();
    void saveParamsAs();
    void savePicture();
    void savePictureAs();
    void writeSettings();

private:
    bool writeConfigFile(QFileInfo& configFile);
    bool readConfigFile(QFileInfo& configFile);
    bool writeBitmapFile(QFileInfo& bitmapFile);
    bool bitMapPathExists();
    bool paramsPathExists();
    void showSuccessMessage();
    void showCancelledMessage();
    void showFailureMessage(const QString& failureMessage = "File operation unsuccessful");
    //temporary flow
    void informNotImplementedYet(QFileInfo&);

    MandelbrotWidget * const plotHandler;
    SettingsHandler& applicationSettingsHandler;
    FileMenuConfig fileMenuConfig;
    QFileDialog dialog;

    QAction* saveParamsAction;
    QAction* savePictureAction;

};

#endif // FILEMENU_H
