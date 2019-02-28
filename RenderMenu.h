#ifndef RENDERMENU_H
#define RENDERMENU_H

#include <QMenu>

class MandelbrotWidget;

class RenderMenu : public QMenu
{
    Q_OBJECT

public:
    RenderMenu(MandelbrotWidget* plotHandler);

public slots:
    void halt();
    void refresh();

private:
    MandelbrotWidget* plotHandler;
};

#endif // RENDERMENU_H
