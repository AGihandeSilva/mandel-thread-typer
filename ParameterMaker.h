#ifndef PARAMETERMAKER_H
#define PARAMETERMAKER_H

#include "computeddatasegment.h"

template <typename T>
struct ParameterMaker
{

    template <typename TT, typename U = ComputedDataSegment>
    ParameterMaker(typename std::enable_if<std::is_floating_point<TT>::value, U>::type dataSegment, TT limitValue) :
        scalingShift(0),
        scaling(1),
        minX(dataSegment.getMinX()),
        maxX(dataSegment.getMaxX()),
        centerX(static_cast<T>(dataSegment.getCenterX())),
        centerY(static_cast<T>(dataSegment.getCenterY())),
        scaleFactor(static_cast<T>(dataSegment.getScaleFactor())),
        limit(limitValue)
    {}



    template <typename TT, typename U = ComputedDataSegment>
    ParameterMaker(typename std::enable_if<std::is_integral<TT>::value, U>::type dataSegment, TT limitValue) :
    scalingShift(((sizeof(T) * CHAR_BIT) / 2) - (MAGNITUDE_BITS * 2)),
    scaling(static_cast<int64_t>(1LL << scalingShift)),
    minX(dataSegment.getMinX()),
    maxX( dataSegment.getMaxX()),
    centerX(static_cast<T>(scaling * dataSegment.getCenterX())),
    centerY(static_cast<T>(scaling * dataSegment.getCenterY())),
    scaleFactor(static_cast<T>(scaling * dataSegment.getScaleFactor())),
    limit(static_cast<T>(scaling * scaling * limitValue))
{}


#if (USE_BOOST_MULTIPRECISION == 1 && !defined(__GNUC__))

    const int doubleToIntShift = 56;
    const int64_t doubleToIntScaling =  1LL << doubleToIntShift;
    template <typename U = ComputedDataSegment>
    ParameterMaker(ComputedDataSegment dataSegment, Int128 limitValue) :
        scalingShift(doubleToIntShift),
        scaling((1LL << scalingShift)),
        minX(dataSegment.getMinX()),
        maxX(dataSegment.getMaxX()),
        //multiplication by floating point types is not supported for this boost type, convert scaling factor to a fixed point value
        centerX((scaling * static_cast<int64_t>(dataSegment.getCenterX() * doubleToIntScaling)) >> doubleToIntShift),
        centerY((scaling * static_cast<int64_t>(dataSegment.getCenterY() * doubleToIntScaling)) >> doubleToIntShift),
        scaleFactor((scaling * static_cast<int64_t>(dataSegment.getScaleFactor() * doubleToIntScaling)) >> doubleToIntShift),
        limit(scaling * scaling * limitValue)
    {}

    template <typename U = ComputedDataSegment>
    ParameterMaker(ComputedDataSegment dataSegment, Float80 limitValue) :
        scalingShift(0),
        scaling(1),
        minX(dataSegment.getMinX()),
        maxX(dataSegment.getMaxX()),
        centerX((dataSegment.getCenterX())),
        centerY((dataSegment.getCenterY())),
        scaleFactor((dataSegment.getScaleFactor())),
        limit(limitValue)
    {}

    template <typename U = ComputedDataSegment>
    ParameterMaker(ComputedDataSegment dataSegment, Float128 limitValue) :
        scalingShift(0),
        scaling(1),
        minX(dataSegment.getMinX()),
        maxX(dataSegment.getMaxX()),
        centerX((dataSegment.getCenterX())),
        centerY((dataSegment.getCenterY())),
        scaleFactor((dataSegment.getScaleFactor())),
        limit(limitValue)
    {}
#endif //(USE_BOOST_MULTIPRECISION == 1 && !defined(__GNUC__))


#if (USE_BOOST_MULTIPRECISION == 1)
    template <typename U = ComputedDataSegment>
    ParameterMaker(ComputedDataSegment dataSegment, Float20dd limitValue) :
        scalingShift(0),
        scaling(1),
        minX(dataSegment.getMinX()),
        maxX(dataSegment.getMaxX()),
        centerX((dataSegment.getCenterX())),
        centerY((dataSegment.getCenterY())),
        scaleFactor((dataSegment.getScaleFactor())),
        limit(limitValue)
    {}

    template <typename U = ComputedDataSegment>
    ParameterMaker(ComputedDataSegment dataSegment, Float30dd limitValue) :
        scalingShift(0),
        scaling(1),
        minX(dataSegment.getMinX()),
        maxX(dataSegment.getMaxX()),
        centerX((dataSegment.getCenterX())),
        centerY((dataSegment.getCenterY())),
        scaleFactor((dataSegment.getScaleFactor())),
        limit(limitValue)
    {}

    template <typename U = ComputedDataSegment>
    ParameterMaker(ComputedDataSegment dataSegment, Float50dd limitValue) :
        scalingShift(0),
        scaling(1),
        minX(dataSegment.getMinX()),
        maxX(dataSegment.getMaxX()),
        centerX((dataSegment.getCenterX())),
        centerY((dataSegment.getCenterY())),
        scaleFactor((dataSegment.getScaleFactor())),
        limit(limitValue)
    {}

    template <typename U = ComputedDataSegment>
    ParameterMaker(ComputedDataSegment dataSegment, CustomFloat limitValue) :
        scalingShift(0),
        scaling(1),
        minX(dataSegment.getMinX()),
        maxX(dataSegment.getMaxX()),
        centerX((dataSegment.getCenterX())),
        centerY((dataSegment.getCenterY())),
        scaleFactor((dataSegment.getScaleFactor())),
        limit(limitValue)
    {}
#endif //(USE_BOOST_MULTIPRECISION == 1)


    const int64_t scalingShift;
    const T scaling;
    const int minX;
    const int maxX;

    const T centerX;
    const T centerY;
    const T scaleFactor;
    const T limit;

    static constexpr int MAGNITUDE_BITS = 3;
};

#endif // PARAMETERMAKER_H
