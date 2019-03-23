#include "RenderMenu.h"
#include "mandelbrotwidget.h"

#include <iostream>


RenderMenu::RenderMenu(MandelbrotWidget* plotHandler) : plotHandler(plotHandler)
{
    setTitle("Render");
    addAction("Execute Render", this, SLOT(refresh()), Qt::Key_F5);
    addAction("Halt computations", this, SLOT(halt()), tr("Ctrl+H"));
}

void RenderMenu::halt()
{
    std::cout << static_cast<const char*>(__FUNCTION__) << std::endl;
    plotHandler->initiateHalt();
}

void RenderMenu::refresh()
{
    plotHandler->initiateRefresh();
}
