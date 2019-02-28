#include <QColor>
#include <cmath>

#include "mandelbrotrenderer.h"

namespace MandelBrotRenderer
{
uint rgbFromWaveLength(double wave)
{
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    if (wave >= 380.0 && wave <= 440.0) {
        r = -1.0 * (wave - 440.0) / (440.0 - 380.0);
        b = 1.0;
    } else if (wave >= 440.0 && wave <= 490.0) {
        g = (wave - 440.0) / (490.0 - 440.0);
        b = 1.0;
    } else if (wave >= 490.0 && wave <= 510.0) {
        g = 1.0;
        b = -1.0 * (wave - 510.0) / (510.0 - 490.0);
    } else if (wave >= 510.0 && wave <= 580.0) {
        r = (wave - 510.0) / (580.0 - 510.0);
        g = 1.0;
    } else if (wave >= 580.0 && wave <= 645.0) {
        r = 1.0;
        g = -1.0 * (wave - 645.0) / (645.0 - 580.0);
    } else if (wave >= 645.0 && wave <= 780.0) {
        r = 1.0;
    }

    double s = 1.0;
    if (wave > 700.0)
        s = 0.3 + 0.7 * (780.0 - wave) / (780.0 - 700.0);
    else if (wave <  420.0)
        s = 0.3 + 0.7 * (wave - 380.0) / (420.0 - 380.0);

    r = std::pow(r * s, 0.8);
    g = std::pow(g * s, 0.8);
    b = std::pow(b * s, 0.8);
    return qRgb(int(r * 255), int(g * 255), int(b * 255));
}

const QString & getBoolValueAsString(bool value, boolDescriptionMode mode)
{
    static const QString onString("On");
    static const QString offString("Off");
    static const QString trueString("True");
    static const QString falseString("False");
    return ((mode == boolDescriptionMode::on_off) ? (value ? onString : offString) :
        (value ? trueString : falseString));
}

QDataStream &operator <<(QDataStream &outputStream, const MandelBrotRenderer::RenderState& state)
{
    outputStream << state.detailedDisplayEnabled << static_cast<qint32>(state.size.width());
    outputStream << static_cast<qint32>(state.size.height());
    outputStream << static_cast<qint32>(state.pos.x()) << static_cast<qint32>(state.pos.y());
    outputStream << state.centerX << state.centerY << state.curScale << state.pixmapScale << state.threadMediatorEnabled;

    return outputStream;
}

QDataStream &operator >>(QDataStream &inputStream, MandelBrotRenderer::RenderState &state)
{
    inputStream >> state.detailedDisplayEnabled;

    qint32 sizeWidth;
    qint32 sizeHeight;
    inputStream >> sizeWidth;
    inputStream >> sizeHeight;
    state.size = QSize(sizeWidth, sizeHeight);

    qint32 x;
    qint32 y;
    inputStream >> x;
    inputStream >> y;
    state.pos = QPoint(x, y);

    inputStream >> state.centerX;
    inputStream >> state.centerY;
    inputStream >> state.curScale;
    inputStream >> state.pixmapScale;
    inputStream >> state.threadMediatorEnabled;

    return inputStream;
}

bool operator==(const RenderState &lhs, const RenderState &rhs)
{
    return(lhs.size == rhs.size &&
        lhs.pos == rhs.pos &&
        comparefloatingPointValues(lhs.centerX, rhs.centerX) &&
        comparefloatingPointValues(lhs.centerY, rhs.centerY) &&
        comparefloatingPointValues(lhs.curScale, rhs.curScale) &&
        comparefloatingPointValues(lhs.pixmapScale, rhs.pixmapScale));
}

bool operator!=(const RenderState &lhs, const RenderState &rhs)
{
    return (!(lhs == rhs));
}

bool comparefloatingPointValues(double thisValue, double otherValue)
{
    static constexpr double equalityThreshold = 1.0001;
    return (otherValue != 0.0 &&
            (thisValue/otherValue > (1.0 - equalityThreshold)
             && thisValue/otherValue < (1.0 + equalityThreshold)));
}

}
