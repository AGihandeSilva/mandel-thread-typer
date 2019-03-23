#include "ParametersMenu.h"
#include "RenderParametersWidget.h"
#include "renderthread.h"
#include "settingshandler.h"

#include <QMenuBar>

ParametersMenu::ParametersMenu(QMenuBar *parent, MandelbrotWidget *mainWidget,
                               RenderThread *masterThread, SettingsHandler& settingsHandler)
    : QMenu(parent), parametersRenderParameters(new RenderParametersWidget(masterThread, *mainWidget, settingsHandler)),
    mainWidget(mainWidget), masterThread(masterThread)
{
    setTitle("Parameters");
    addAction("Render Parameters", this, SLOT(setParameters()), tr("Ctrl+P"));

    connect(masterThread, SIGNAL(renderStarting()), this, SLOT(disableRenderParameters()));
    connect(masterThread, SIGNAL(allDone()), this, SLOT(enableRenderParameters()));
}

void ParametersMenu::setParameters()
{
    parametersRenderParameters->updateAndShow();
}

void ParametersMenu::refreshParametersGUI()
{
    parametersRenderParameters->refresh();
}

void ParametersMenu::disableRenderParameters()
{
    parametersRenderParameters->setEnabled(false);
}

void ParametersMenu::enableRenderParameters()
{
    parametersRenderParameters->setEnabled(true);
}
