#include "RenderMenu.h"
#include "mandelbrotwidget.h"

#include <iostream>


RenderMenu::RenderMenu(MandelbrotWidget* plotHandler) : plotHandler(plotHandler)
{
    setTitle("Render");
    addAction("Execute Render", Qt::Key_F5, this, SLOT(refresh()));
    addAction("Halt computations", tr("Ctrl+H"), this, SLOT(halt()));
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
