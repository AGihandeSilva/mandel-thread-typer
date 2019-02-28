#ifndef WINDOWTHREADINFOKEY_H
#define WINDOWTHREADINFOKEY_H

#include "mandelbrotrenderer.h"

#include <QGroupBox>

#include <array>

class QLabel;
class QGridLayout;
class ThreadIconMap;

using namespace MandelBrotRenderer;

class WindowThreadInfoKey : public QGroupBox
{
public:
    explicit WindowThreadInfoKey(const ThreadIconMap* threadIcons);

    public slots:

private:
    QGridLayout* threadStateKeyLayout;
    const ThreadIconMap* threadIcons;

    static constexpr int width = 210;
    static constexpr int height = 260;
};

#endif // WINDOWTHREADINFOKEY_H
