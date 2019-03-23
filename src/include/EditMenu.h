#ifndef EDITMENU_H
#define EDITMENU_H

#include <QMenu>

class MandelbrotWidget;
class QAction;

class EditMenu : public QMenu
{
    Q_OBJECT

public:
    EditMenu(MandelbrotWidget* historyOwner);
    void setUndoEnabled(bool enabled);
    void setRedoEnabled(bool enabled);

private:
    MandelbrotWidget* historyOwner;
    QAction* undo;
    QAction* redo;
};

#endif // EDITMENU_H
