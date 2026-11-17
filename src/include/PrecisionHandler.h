#ifndef PRECISIONHANDLER_H
#define PRECISIONHANDLER_H

#include <QObject>
#include <QMutex>

#include "mandelbrotrenderer.h"

namespace MandelBrotRenderer
{
bool operator== (const RenderState& lhs, const RenderState& rhs);
bool operator!= (const RenderState& lhs, const RenderState& rhs);

bool comparefloatingPointValues(double thisValue, double otherValue);

QString generatePreciseFloatingPointString(Float128 preciseValue);

PreciseFloatResult generateFloatFromPreciseString(QString floatString, std::streamsize precision = std::numeric_limits<Float128>::max_digits10);
QString generateDoubleAsString(double value);

DoubleResult generateFloatFromString(const QString& floatString);

}

class PrecisionHandler : public QObject
{
    Q_OBJECT
public:
    static PrecisionHandler& getPreciseValuesHandler();
    QMutex* getMutex() { return &m_mutex; }

    PrecisionHandler(const PrecisionHandler&) = delete;
    PrecisionHandler(PrecisionHandler&&) = delete;
    PrecisionHandler& operator=(const PrecisionHandler&) = delete;
    PrecisionHandler& operator=(PrecisionHandler&&) = delete;

private:
    PrecisionHandler() = default;
    ~PrecisionHandler() override = default;

    QMutex m_mutex;
};

#endif // PRECISIONHANDLER_H
