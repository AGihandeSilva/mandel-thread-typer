#ifndef THREADICONMAP_H
#define THREADICONMAP_H

#include "mandelbrotrenderer.h"

#include <QPixmap>

#include <array>
#include <map>


using ThreadBitmapList      = std::array<QPixmap, MandelBrotRenderer::numStates>;
using ThreadStateNameList   = std::array<QString, MandelBrotRenderer::numStates>;
using StateNameMap          = std::map<MandelBrotRenderer::threadState, QString>;

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
    const QString& getThreadStateName(MandelBrotRenderer::threadState state);

    static constexpr int bitmapPixelSize = 15;
    static constexpr int colMinWidth = 80;

    ThreadBitmapList threadStateBitmaps {};
    ThreadStateNameList threadStateNames {};
    StateNameMap StateNames {};
};

#endif // THREADICONMAP_H
