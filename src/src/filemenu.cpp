#include <QMenuBar>
#include <QApplication>
#include <QPixmap>
#include <QMessageBox>

#include "filemenu.h"
#include "mandelbrotwidget.h"

#include <iostream>

FileMenu::FileMenu(QMenuBar* parent, MandelbrotWidget* plotHandler, SettingsHandler& applicationSettingsHandler)
: QMenu(parent), plotHandler(plotHandler), applicationSettingsHandler(applicationSettingsHandler),
    fileMenuConfig {new QFileInfo(), new QFileInfo()},
    dialog(this)

{
    setTitle("File");
    addSeparator();
    addAction("Load Parameters", this, SLOT(openParams()));
    addSeparator();
    saveParamsAction = addAction("Save Parameters", this, SLOT(saveParams()), tr("Ctrl S"));
    addAction("Save Parameters As...", this, SLOT(saveParamsAs()));
    savePictureAction = addAction("Save Bitmap Image", this, SLOT(savePicture()));
    addAction("Save Bitmap Image As...", this, SLOT(savePictureAs()));
    addSeparator();
    addSeparator();
    addAction("Exit", this, SLOT(quit()), tr("Ctrl+Q"));

    saveParamsAction->setEnabled(false);
    savePictureAction->setEnabled(false);

    applicationSettingsHandler.registerSettingsUser(this);
}

void FileMenu::quit()
{
    std::cout << static_cast<const char*>(__FUNCTION__) << std::endl;
    plotHandler->initiateQuit();

    //TODO make sure all windows are closed
    //TODO make sure no memory is leaked: (how?)
}

void FileMenu::openParams()
{
    dialog.setFileMode(QFileDialog::ExistingFile);

    QFileInfo fileName = QFileDialog::getOpenFileName(this,
        tr("Open Settings File"), fileMenuConfig.currentFilePath->filePath(), tr("Renderer Config Files (*.cfg)"));

    if (readConfigFile(fileName))
    {
        *fileMenuConfig.currentFilePath = fileName;
        saveParamsAction->setEnabled(true);
    }
    else {
        //TODO make sure failed case is handled correctly
    }
}

void FileMenu::processSettingUpdate(QSettings & settings)
{
    settings.beginGroup("FileSettings");

    fileMenuConfig.currentFilePath->setFile(settings.value("currentFilePath", "").toString());
    fileMenuConfig.currentBitmapFilePath->setFile(settings.value("currentBitmapFilePath", "").toString());

    settings.endGroup();

    saveParamsAction->setEnabled(paramsPathExists());
    savePictureAction->setEnabled(bitMapPathExists());
}

bool FileMenu::paramsPathExists()
{
    return (fileMenuConfig.currentFilePath->isWritable() &&
            (!fileMenuConfig.currentFilePath->filePath().isEmpty()));
}

void FileMenu::showSuccessMessage()
{
    plotHandler->writeTransientStatusMessage("File operation successful", false);
}

void FileMenu::showCancelledMessage()
{
    plotHandler->writeStatusMessage("File operation cancelled", false);
}

void FileMenu::showFailureMessage(const QString& failureMessage)
{
    plotHandler->writeStatusMessage(failureMessage, true);
}

void FileMenu::saveParams()
{
    Q_ASSERT(paramsPathExists());
    writeConfigFile(*fileMenuConfig.currentFilePath);
}

void FileMenu::saveParamsAs()
{
    dialog.setFileMode(QFileDialog::AnyFile);
     QString filename = QFileDialog::getSaveFileName(this, tr("Save Config File"),
                               fileMenuConfig.currentFilePath->filePath(),
                               tr("Config files (*.cfg)"));

    if (filename.isEmpty())
    {
        showCancelledMessage();
        return;
    }

    QFileInfo pendingParamsFile(filename);
    if (writeConfigFile(pendingParamsFile)) {
        *fileMenuConfig.currentFilePath = pendingParamsFile;
        saveParamsAction->setEnabled(true);
    }
}

bool FileMenu::bitMapPathExists()
{
   return (fileMenuConfig.currentBitmapFilePath->isWritable() &&
            (!fileMenuConfig.currentBitmapFilePath->filePath().isEmpty()));
}

void FileMenu::savePicture()
{
    Q_ASSERT(bitMapPathExists());
    writeBitmapFile(*fileMenuConfig.currentBitmapFilePath);
}

void FileMenu::savePictureAs()
{
    dialog.setFileMode(QFileDialog::AnyFile);

    QString filename = QFileDialog::getSaveFileName(this, tr("Save Bitmap File"),
                               fileMenuConfig.currentBitmapFilePath->filePath(),
                               tr("Images (*.png *.jpg)"));

    if (filename.isEmpty())
    {
        showCancelledMessage();
        return;
    }

    QFileInfo pendingBitMapFile(filename);

    if (writeBitmapFile(pendingBitMapFile)) {
        *fileMenuConfig.currentBitmapFilePath = pendingBitMapFile;
        savePictureAction->setEnabled(true);
    }
}

void FileMenu::writeSettings()
{
    applicationSettingsHandler.getSettings().beginGroup("FileSettings");
    applicationSettingsHandler.getSettings().setValue("currentFilePath", fileMenuConfig.currentFilePath->filePath());
    applicationSettingsHandler.getSettings().setValue("currentBitmapFilePath", fileMenuConfig.currentBitmapFilePath->filePath());
    applicationSettingsHandler.getSettings().endGroup();
}

bool FileMenu::writeConfigFile(QFileInfo& configFile)
{

    RendererConfig config = plotHandler->generateConfigData();

    if (!config.checkLegality())
    {
        showFailureMessage("Error occurred during config generation");
        return false;
    }

    bool saveCompleted = config.write(configFile);

    if (saveCompleted) {
        showSuccessMessage();
        plotHandler->setUnsavedChangesExist(false);
    } else {
        showFailureMessage();
    }

    return saveCompleted;
}

bool FileMenu::readConfigFile(QFileInfo& configFile)
{
    bool readSuccessful = false;

    std::pair<bool, RendererConfig> readResult = RendererConfig::createFromFile(configFile);

    if (readResult.first
            && readResult.second.checkLegality())
    {
        plotHandler->enforceConfigData(readResult.second);

        //double check
        auto configuration = plotHandler->generateConfigData();
        Q_ASSERT(configuration.checkLegality());

        showSuccessMessage();
        plotHandler->setUnsavedChangesExist(false);
    }
    else
    {
        readSuccessful = false;
        showFailureMessage();
    }

    return readSuccessful;
}

bool FileMenu::writeBitmapFile(QFileInfo& bitmapFile)
{
    bool result = plotHandler->getPixmap().save(bitmapFile.filePath());

    if (result) {
        showSuccessMessage();
    } else {
        showFailureMessage();
    }
    return result;
}

void FileMenu::informNotImplementedYet(QFileInfo&)
{
    auto NoDuplicateWarning = new QMessageBox();
    NoDuplicateWarning->setText("This Feature is not implemented yet");
    NoDuplicateWarning->exec();
}

FileMenuConfig FileMenu::getFileMenuConfig() const
{
    return fileMenuConfig;
}
