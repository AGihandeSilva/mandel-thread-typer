#include "EditMenu.h"
#include "mandelbrotwidget.h"

EditMenu::EditMenu(MandelbrotWidget* historyOwner) : historyOwner(historyOwner)
{
    setTitle("Edit");
    undo = addAction("Undo", historyOwner, SLOT(undo()), tr("Ctrl+Z"));
    redo = addAction("Redo", historyOwner, SLOT(redo()), tr("Ctrl+Y"));
}

void EditMenu::setUndoEnabled(bool enabled)
{
    undo->setEnabled(enabled);
}

void EditMenu::setRedoEnabled(bool enabled)
{
    redo->setEnabled(enabled);
}
