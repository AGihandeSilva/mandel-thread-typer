#ifndef PRECISIONHANDLER_H
#define PRECISIONHANDLER_H

#include <QMutex>

#include "mandelbrotrenderer.h"

namespace MandelBrotRenderer
{
bool operator== (const RenderState& lhs, const RenderState& rhs);
bool operator!= (const RenderState& lhs, const RenderState& rhs);

bool comparefloatingPointValues(double thisValue, double otherValue);

#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
QString generatePreciseFloatingPointString(Float128 preciseValue);

PreciseFloatResult generateFloatFromPreciseString(QString floatString, std::streamsize precision = std::numeric_limits<Float128>::max_digits10);
#endif
QString generateDoubleAsString(double value);

DoubleResult generateFloatFromString(const QString& floatString);

}

class PrecisionHandler : public QObject
{
    Q_OBJECT
public:
    static PrecisionHandler& getPreciseValuesHandler();
    QMutex* getMutex() { return &mutex; }

private:
    PrecisionHandler() = default;
    ~PrecisionHandler() = default;
    PrecisionHandler(const PrecisionHandler&) = delete;
    PrecisionHandler(PrecisionHandler&&) = delete;
    PrecisionHandler& operator=(const PrecisionHandler&) = delete;
    PrecisionHandler& operator=(PrecisionHandler&&) = delete;
    QMutex mutex;
};

#endif // PRECISIONHANDLER_H
