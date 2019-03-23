#include <QMenuBar>
#include "toolsmenu.h"
#include "toolsoptionswidget.h"
#include "mandelbrotwidget.h"

#include <iostream>

ToolsMenu::ToolsMenu(QMenuBar* parent, MandelbrotWidget* mainWidget, 
    RenderThread *masterThread, SettingsHandler& settingsHandler)
    : QMenu(parent), mainWidget(mainWidget), masterThread(masterThread)
{
    setTitle("Tools");
    addAction("Options", this, SLOT(setOptions()), tr("Ctrl+O"));
    toolsOptions = new ToolsOptionsWidget(masterThread, mainWidget, settingsHandler);

    addAction("Restore Defaults", mainWidget, SLOT(restoreDefaults()));

    connect(masterThread, SIGNAL(renderStarting()), this, SLOT(disableOptions()));
    connect(masterThread, SIGNAL(allDone()), this, SLOT(enableOptions()));
}

void ToolsMenu::setOptions()
{
    toolsOptions->updateAndShow();
}

void ToolsMenu::refreshOptionsGUI()
{
    toolsOptions->refresh();
}

void ToolsMenu::disableOptions()
{
    toolsOptions->setEnabled(false);
}

void ToolsMenu::enableOptions()
{
    toolsOptions->setEnabled(true);
}

