#include "threadiconmap.h"

#include <QPixmap>

ThreadIconMap::ThreadIconMap()
{
    prepareStateNames();
    prepareBitmaps();
}

void ThreadIconMap::prepareStateNames()
{
    StateNames.insert(std::make_pair(threadState::disabled, "Disabled"));
    StateNames.insert(std::make_pair(threadState::starting, "Initializing"));
    StateNames.insert(std::make_pair(threadState::idle, "Idle"));
    StateNames.insert(std::make_pair(threadState::waiting, "Waiting"));
    StateNames.insert(std::make_pair(threadState::busy, "Running original task"));
    StateNames.insert(std::make_pair(threadState::shared, "Running split task"));
    StateNames.insert(std::make_pair(threadState::restarted, "Running received task"));
    StateNames.insert(std::make_pair(threadState::finishing, "Near Completion"));
}

const QString& ThreadIconMap::getThreadStateName(threadState state)
{
    static const QString unknownStateName("-");
    auto iter = StateNames.find(state);

    if (iter == StateNames.end())
    {
        return unknownStateName;
    }

    return ((*iter).second);
}

StateNameMap ThreadIconMap::getStateNames() const
{
    return StateNames;
}

ThreadStateNameList ThreadIconMap::getThreadStateNames() const
{
    return threadStateNames;
}


void ThreadIconMap::prepareBitmaps()
{
    QPixmap disabledBitmap(bitmapPixelSize, bitmapPixelSize);
    disabledBitmap.fill(Qt::white);
    std::size_t index = toUnderlyingType(threadState::disabled);
    threadStateBitmaps.at(index) = disabledBitmap;
    threadStateNames.at(index) = getThreadStateName(threadState::disabled);

    QPixmap startingBitmap(bitmapPixelSize, bitmapPixelSize);
    startingBitmap.fill(Qt::yellow);
    index = toUnderlyingType(threadState::starting);
    threadStateBitmaps.at(index) = startingBitmap;
    threadStateNames.at(index) = getThreadStateName(threadState::starting);

    QPixmap idleBitmap(bitmapPixelSize, bitmapPixelSize);
    idleBitmap.fill(Qt::lightGray);
    index = toUnderlyingType(threadState::idle);
    threadStateBitmaps.at(toUnderlyingType(threadState::idle)) = idleBitmap;
    threadStateNames.at(index) = getThreadStateName(threadState::idle);

    QPixmap waitingBitmap(bitmapPixelSize, bitmapPixelSize);
    waitingBitmap.fill(Qt::black);
    index = toUnderlyingType(threadState::waiting);
    threadStateBitmaps.at(toUnderlyingType(threadState::waiting)) = waitingBitmap;
    threadStateNames.at(index) = getThreadStateName(threadState::waiting);

    QPixmap busyBitmap(bitmapPixelSize, bitmapPixelSize);
    busyBitmap.fill(Qt::green);
    index = toUnderlyingType(threadState::busy);
    threadStateBitmaps.at(toUnderlyingType(threadState::busy)) = busyBitmap;
    threadStateNames.at(index) = getThreadStateName(threadState::busy);

    QPixmap sharedBitmap(bitmapPixelSize, bitmapPixelSize);
    sharedBitmap.fill(Qt::magenta);
    index = toUnderlyingType(threadState::shared);
    threadStateBitmaps.at(toUnderlyingType(threadState::shared)) = sharedBitmap;
    threadStateNames.at(index) = getThreadStateName(threadState::shared);

    QPixmap restartedBitmap(bitmapPixelSize, bitmapPixelSize);
    restartedBitmap.fill(Qt::darkCyan);
    index = toUnderlyingType(threadState::restarted);
    threadStateBitmaps.at(toUnderlyingType(threadState::restarted)) = restartedBitmap;
    threadStateNames.at(index) = getThreadStateName(threadState::restarted);

    QPixmap finishingBitmap(bitmapPixelSize, bitmapPixelSize);
    finishingBitmap.fill(Qt::darkRed);
    index = toUnderlyingType(threadState::finishing);
    threadStateBitmaps.at(toUnderlyingType(threadState::finishing)) = finishingBitmap;
    threadStateNames.at(index) = getThreadStateName(threadState::finishing);
}
