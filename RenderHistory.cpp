#include "RenderHistory.h"
#include "mandelbrotwidget.h"

RenderHistory::RenderHistory(MandelbrotWidget* const stateHolder) : stateHolder(stateHolder), currentPos(historyLog.begin())
{
    connect(this, SIGNAL(undoIsAvailable(bool)), stateHolder, SLOT(setUndoEnabled(bool)));
    connect(this, SIGNAL(redoIsAvailable(bool)), stateHolder, SLOT(setRedoEnabled(bool)));
}

RenderHistory::stateListIter RenderHistory::getLastEntryPos()
{
    Q_ASSERT(!historyLog.empty());
    auto result = historyLog.end();
    result--;

    return result;
}

bool RenderHistory::saveState()
{
    bool novelStateGenerated = false;
    if (!historyLog.empty() && currentPos != getLastEntryPos())
    {
        removeTailEntries();
    }
    const MandelBrotRenderer::RenderState newState = stateHolder->generateConfigData().getState();
    if (currentPos == historyLog.end() || newState != *currentPos) {
        historyLog.push_back(newState);
        novelStateGenerated = true;
    }
    
    currentPos = getLastEntryPos();
    checkAvailability();

    return novelStateGenerated;
}

void RenderHistory::removeTailEntries()
{
    Q_ASSERT(currentPos != getLastEntryPos());
    auto firstToGo = currentPos;
    firstToGo++;
    historyLog.erase(firstToGo, historyLog.end());
}

void RenderHistory::undo()
{
    Q_ASSERT(undoIsPossible());
    setNewState(Direction::backwards);
}

void RenderHistory::redo()
{
    Q_ASSERT(redoIsPossible());
    setNewState(Direction::forwards);
}

bool RenderHistory::undoIsPossible() const
{
    return (currentPos != historyLog.end() &&
        currentPos != historyLog.begin());
}

bool RenderHistory::redoIsPossible() const
{
    if (currentPos == historyLog.end()) {
        return false;
    }
    stateListIter next(currentPos);
    ++next;
    return (next != historyLog.end());
}

void RenderHistory::setNewState(Direction moveDirection)
{
    if (moveDirection == Direction::backwards) {
        Q_ASSERT(moveDirection == Direction::backwards &&
                  currentPos != historyLog.begin());
        --currentPos;
    } else if (moveDirection == Direction::forwards) {
        Q_ASSERT(moveDirection == Direction::forwards &&
                                currentPos != historyLog.end());
        ++currentPos;
    }
    checkAvailability();
    RendererConfig newConfig(*currentPos);
    stateHolder->setUsingUndoRedo();
    stateHolder->enforceConfigData(newConfig);
}

void RenderHistory::checkAvailability()
{
    bool undoAvailable = undoIsPossible();
    emit undoIsAvailable(undoAvailable);

    bool redoAvailable = redoIsPossible();
    emit redoIsAvailable(redoAvailable);
}
