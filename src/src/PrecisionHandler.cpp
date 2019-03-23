#include "PrecisionHandler.h"

PrecisionHandler& PrecisionHandler::getPreciseValuesHandler()
{
    static PrecisionHandler handler;
    return handler;
}


namespace MandelBrotRenderer
{
bool operator==(const RenderState &lhs, const RenderState &rhs)
{
    DoubleResult centerX_float_l = generateFloatFromString(lhs.centerX);
    DoubleResult centerX_float_r = generateFloatFromString(rhs.centerX);
    DoubleResult centerY_float_l = generateFloatFromString(lhs.centerY);
    DoubleResult centerY_float_r = generateFloatFromString(rhs.centerY);
    return(lhs.size == rhs.size &&
        lhs.pos == rhs.pos &&
        centerX_float_l.second && centerX_float_r.second &&
        comparefloatingPointValues(centerX_float_l.first, centerX_float_r.first) &&
        centerY_float_l.second && centerY_float_r.second &&
        comparefloatingPointValues(centerY_float_l.first, centerY_float_r.first) &&
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

DoubleResult generateFloatFromString(const QString& floatString)
{
    bool ok = true;
    double result = floatString.toDouble(&ok);
    result = ok ? result : 0.0;
    return std::make_pair(result, ok);
}



#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
QString generatePreciseFloatingPointString(Float128 preciseValue)
{
    QMutexLocker lock(PrecisionHandler::getPreciseValuesHandler().getMutex());
    std::stringstream ss;
    ss.precision(std::numeric_limits<Float128>::max_digits10);
    ss.flags(std::ios_base::fmtflags(std::ios_base::scientific));
#if (USE_BOOST_MULTIPRECISION != 1)
    //GCC __float128 doesn't seem to support this operation
    ss << static_cast<double>(preciseValue);
#else
    ss << preciseValue;
#endif //(USE_BOOST_MULTIPRECISION != 1)

    return QString(ss.str().data());
}

#if !defined(__GNUC__)
PreciseFloatResult generateFloatFromPreciseString(QString floatString, std::streamsize precision)
{
    QMutexLocker lock(PrecisionHandler::getPreciseValuesHandler().getMutex());

    std::stringstream ss(floatString.toStdString());
    ss.precision(precision);
    //ss.flags(std::ios_base::fmtflags(std::ios_base::scientific));

    Q_ASSERT(!ss.bad());

    Float128 value;
    ss >> value;
    Q_ASSERT (!ss.fail());

    return std::make_pair(value, !ss.fail());
}
#endif //!defined(__GNUC__)
#endif //(USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
//TODO use templated functions to eliminate this code duplication
QString generateDoubleAsString(double value)
{
    QMutexLocker lock(PrecisionHandler::getPreciseValuesHandler().getMutex());
    std::stringstream ss;
    ss.precision(std::numeric_limits<double>::max_digits10);
    ss.flags(std::ios_base::fmtflags(std::ios_base::scientific));
    ss << value;

    return QString(ss.str().data());
}

#if defined(__GNUC__)
PreciseFloatResult generateFloatFromPreciseString(QString floatString, std::streamsize)
{
return std::make_pair((strtoflt128(floatString.toStdString().data(), nullptr)), true);
}
#endif
}
