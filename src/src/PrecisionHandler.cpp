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
    DoubleResult originX_float_l = generateFloatFromString(lhs.m_originX);
    DoubleResult originX_float_r = generateFloatFromString(rhs.m_originX);
    DoubleResult originY_float_l = generateFloatFromString(lhs.m_originY);
    DoubleResult originY_float_r = generateFloatFromString(rhs.m_originY);
    DoubleResult curScale_float_l = generateFloatFromString(lhs.m_curScale);
    DoubleResult curScale_float_r = generateFloatFromString(rhs.m_curScale);
    DoubleResult pixmapScale_float_l = generateFloatFromString(lhs.m_pixmapScale);
    DoubleResult pixmapScale_float_r = generateFloatFromString(rhs.m_pixmapScale);
    return(lhs.m_size == rhs.m_size &&
        lhs.m_pos == rhs.m_pos &&
        originX_float_l.second && originX_float_r.second &&
        comparefloatingPointValues(originX_float_l.first, originX_float_r.first) &&
        originY_float_l.second && originY_float_r.second &&
        comparefloatingPointValues(originY_float_l.first, originY_float_r.first) &&
        curScale_float_l.second && curScale_float_r.second &&
        comparefloatingPointValues(curScale_float_l.first, curScale_float_r.first) &&
        pixmapScale_float_l.second && pixmapScale_float_r.second &&
        comparefloatingPointValues(pixmapScale_float_l.first, pixmapScale_float_r.first));
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



QString generatePreciseFloatingPointString(Float128 preciseValue)
{
    QMutexLocker lock(PrecisionHandler::getPreciseValuesHandler().getMutex());
    std::stringstream ss;
    ss.precision(std::numeric_limits<Float128>::max_digits10);
    ss.flags(std::ios_base::fmtflags(std::ios_base::scientific));
#if (PREFER_BOOST_MULTIPRECISION_FLOAT_128 != 1)
    //GCC __float128 doesn't seem to support this operation
    ss << static_cast<double>(preciseValue);
#else
    ss << preciseValue;
#endif //(PREFER_BOOST_MULTIPRECISION_FLOAT_128 != 1)

    return QString(ss.str().data());
}

#if !defined(__GNUC__)
PreciseFloatResult generateFloatFromPreciseString(QString floatString, std::streamsize precision)
{
    QMutexLocker lock(PrecisionHandler::getPreciseValuesHandler().getMutex());

    std::stringstream ss(floatString.toStdString());
    ss.precision(precision);
    //ss.flags(std::ios_base::fmtflags(std::ios_base::scientific));

    if (ss.bad())
    {
        return std::make_pair(Float128{}, false);
    }

    Float128 value;
    ss >> value;
    if (ss.fail())
    {
        return std::make_pair(Float128{}, false);
    }

    return std::make_pair(value, true);
}
#endif //!defined(__GNUC__)

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
