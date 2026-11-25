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
    PreciseFloatResult originX_float_l = generateFloatFromPreciseString(lhs.m_originX);
    PreciseFloatResult  originX_float_r = generateFloatFromPreciseString(rhs.m_originX);
    PreciseFloatResult  originY_float_l = generateFloatFromPreciseString(lhs.m_originY);
    PreciseFloatResult  originY_float_r = generateFloatFromPreciseString(rhs.m_originY);
    PreciseFloatResult  curScale_float_l = generateFloatFromPreciseString(lhs.m_curScale);
    PreciseFloatResult  curScale_float_r = generateFloatFromPreciseString(rhs.m_curScale);
    PreciseFloatResult  pixmapScale_float_l = generateFloatFromPreciseString(lhs.m_pixmapScale);
    PreciseFloatResult  pixmapScale_float_r = generateFloatFromPreciseString(rhs.m_pixmapScale);

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

bool comparefloatingPointValues(PreciseFloatStandardType thisValue, PreciseFloatStandardType otherValue)
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



QString generatePreciseFloatingPointString(PreciseFloatStandardType preciseValue)
{
    QMutexLocker lock(PrecisionHandler::getPreciseValuesHandler().getMutex());
    std::stringstream ss;
    ss.precision(PreciseFloatStandardNumDigits);
    ss.flags(std::ios_base::fmtflags(std::ios_base::scientific));
    ss << preciseValue;

    return QString(ss.str().data());
}

PreciseFloatResult generateFloatFromPreciseString(QString floatString, std::streamsize precision)
{
    QMutexLocker lock(PrecisionHandler::getPreciseValuesHandler().getMutex());

    std::stringstream ss(floatString.toStdString());
    ss.precision(precision);
    ss.flags(std::ios_base::fmtflags(std::ios_base::scientific));

    if (ss.bad())
    {
        return std::make_pair(PreciseFloatStandardType{}, false);
    }

    PreciseFloatStandardType value;
    ss >> value;
    if (ss.fail())
    {
        return std::make_pair(PreciseFloatStandardType{}, false);
    }

    return std::make_pair(value, true);
}
}
