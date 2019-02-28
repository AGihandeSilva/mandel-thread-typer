#ifndef RENDERHISTORY_H
#define RENDERHISTORY_H
#include "mandelbrotrenderer.h"

#include <QObject>
#include <list>

class MandelbrotWidget;

class RenderHistory : public QObject
{
    Q_OBJECT

public:
    explicit RenderHistory(MandelbrotWidget* const stateHolder);
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
