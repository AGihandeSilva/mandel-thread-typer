#ifndef THREADICONMAP_H
#define THREADICONMAP_H

#include "mandelbrotrenderer.h"

#include <QPixmap>

#include <array>
#include <map>



using namespace MandelBrotRenderer;

using ThreadBitmapList      = std::array<QPixmap, numStates>;
using ThreadStateNameList   = std::array<QString, numStates>;
using StateNameMap          = std::map<threadState, QString>;

class ThreadIconMap
{
public:
    ThreadIconMap();

    const ThreadBitmapList&  getThreadBitmaps() const { return threadStateBitmaps; }


    ThreadStateNameList getThreadStateNames() const;

    StateNameMap getStateNames() const;

private:
    void prepareStateNames();
    void prepareBitmaps();
    const QString& getThreadStateName(threadState state);

    static constexpr int bitmapPixelSize = 15;
    static constexpr int colMinWidth = 80;

    ThreadBitmapList threadStateBitmaps {};
    ThreadStateNameList threadStateNames {};
    StateNameMap StateNames {};
};

#endif // THREADICONMAP_H
