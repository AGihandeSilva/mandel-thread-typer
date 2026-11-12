#include "EditMenu.h"
#include "mandelbrotwidget.h"

EditMenu::EditMenu(MandelbrotWidget* historyOwner) : historyOwner(historyOwner)
{
    setTitle("Edit");
    undo = addAction("Undo", tr("Ctrl+Z"), historyOwner, SLOT(undo()));
    redo = addAction("Redo", tr("Ctrl+Y"), historyOwner, SLOT(redo()));
}

void EditMenu::setUndoEnabled(bool enabled)
{
    undo->setEnabled(enabled);
}

void EditMenu::setRedoEnabled(bool enabled)
{
    redo->setEnabled(enabled);
}
