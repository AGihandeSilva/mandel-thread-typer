#ifndef TOOLSMENU_H
#define TOOLSMENU_H

#include <memory>

#include <QMenu>
#include <QAction>

class QMenuBar;
class ToolsOptionsWidget;
class RenderThread;
class MandelbrotWidget;
class SettingsHandler;


class ToolsMenu : public QMenu
{
    Q_OBJECT
public:
    explicit ToolsMenu(QMenuBar* parent, MandelbrotWidget* mainWidget, 
        RenderThread* masterThread, SettingsHandler& settingsHandler);

public slots:
    void setOptions();
    void refreshOptionsGUI();

private:
    ToolsOptionsWidget* toolsOptions;
    MandelbrotWidget* mainWidget;
    RenderThread* masterThread;

private slots:
    void disableOptions();
    void enableOptions();
};

#endif // TOOLSMENU_H
