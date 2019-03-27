#ifndef RENDERHISTORY_H
#define RENDERHISTORY_H
#include "mandelbrotrenderer.h"

#include <QObject>
#include <list>

class MandelbrotWidget;

/*
 *
 * This class is responsible for managing the undo/redo flow.
 * It also assists in deciding whether to ask the user to
 * consider saving the state to prevent its loss on quitting
 * the program.
 *
 */

class RenderHistory : public QObject
{
    Q_OBJECT

public:
    explicit RenderHistory(MandelbrotWidget* stateHolder);
    bool saveState();
    void saveChangedStateClearRedo() {} // TODO
    bool undoIsPossible() const;
    bool redoIsPossible() const;

public slots:
    void undo();
    void redo();

signals:
    void undoIsAvailable(bool available);
    void redoIsAvailable(bool available);

private:
    using stateListIter = std::list<MandelBrotRenderer::RenderState>::iterator;
    enum class Direction { forwards, backwards };

    void setNewState(Direction moveDirection);
    void checkAvailability();
    stateListIter getLastEntryPos();
    void removeTailEntries();

    MandelbrotWidget* const stateHolder;
    std::list<MandelBrotRenderer::RenderState> historyLog;
    stateListIter currentPos;

};

#endif // RENDERHISTORY_H
