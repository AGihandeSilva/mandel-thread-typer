#ifndef PARAMETERSMENU_H
#define PARAMETERSMENU_H

#include <QMenu>

class QMenuBar;
class MandelbrotWidget;
class RenderParametersWidget;
class RenderThread;
class SettingsHandler;

class ParametersMenu : public QMenu
{
    Q_OBJECT
public:
    explicit ParametersMenu(QMenuBar* parent, MandelbrotWidget* mainWidget,
                            RenderThread *masterThread, SettingsHandler& settingsHandler);

public slots:
    void setParameters();
    void refreshParametersGUI();

private:
    RenderParametersWidget* parametersRenderParameters;
    MandelbrotWidget* mainWidget;
    RenderThread *masterThread;

private slots:
    void disableRenderParameters();
    void enableRenderParameters();
};

#endif // PARAMETERSMENU_H
