#ifndef PARAMETERMAKER_H
#define PARAMETERMAKER_H

#include "computeddatasegment.h"
#include "PrecisionHandler.h"

/*
 * Parameter helper class and related operations
 * as required by the ComputeTask class
 *
 * scaling for fixed-point arithmetic flows is carried out here
 */

namespace MandelParams
{
    template <typename T>
    struct needs_scale_shift {
        static const bool value = false;
    };

    template <typename T>
    struct multiply_by_float_supported {
        static const bool value = true;
    };

    #if (USE_BOOST_MULTIPRECISION == 1 || defined(__GNUC__))
    template <>
    struct needs_scale_shift<MandelBrotRenderer::Int128> {
        static const bool value = true;
    };

    //boost multiprecision integer types don't support multiply by floating point values
    #if !defined(__GNUC__)
    template <>
    struct multiply_by_float_supported<MandelBrotRenderer::Int128> {
        static const bool value = false;
    };
    #endif //!__GNUC__
    #endif //USE_BOOST_MULTIPRECISION == 1 || defined(__GNUC__)

    template <>
    struct needs_scale_shift<int32_t> {
        static const bool value = true;
    };

    template <>
    struct needs_scale_shift<int64_t> {
        static const bool value = true;
    };
}

/*
 * Constructor for float types
 */
template <typename T>
struct ParameterMaker
{

    template <typename TT, typename U = ComputedDataSegment>
    explicit ParameterMaker(typename std::enable_if<!MandelParams::needs_scale_shift<TT>::value &&
                                        MandelParams::multiply_by_float_supported<TT>::value, U>::type dataSegment, TT limitValue) :
        scalingShift(0),
        scaling(1),
        minX(dataSegment.getMinX()),
        maxX(dataSegment.getMaxX()),
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
        centerX(MandelBrotRenderer::generateFloatFromPreciseString(dataSegment.getPreciseCenterX()).first),
        centerY(MandelBrotRenderer::generateFloatFromPreciseString(dataSegment.getPreciseCenterY()).first),
#else
        centerX(static_cast<T>(dataSegment.getCenterX())),
        centerY(static_cast<T>(dataSegment.getCenterY())),
#endif
        scaleFactor(static_cast<T>(dataSegment.getScaleFactor())),
        limit(limitValue)
    {}


/*
 * Constructor for 'Normal' integer types
 */
    template <typename TT, typename U = ComputedDataSegment>
    explicit ParameterMaker(typename std::enable_if<MandelParams::needs_scale_shift<TT>::value &&
                                            MandelParams::multiply_by_float_supported<TT>::value, U>::type dataSegment, TT limitValue) :
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
/*
 * Constructor for Boost multiprecision integer types
 */
    template <typename U = ComputedDataSegment>
    explicit ParameterMaker(ComputedDataSegment dataSegment, MandelBrotRenderer::Int128 limitValue) :
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


    const int doubleToIntShift = 56;
    const int64_t doubleToIntScaling = 1LL << doubleToIntShift;


#endif //(USE_BOOST_MULTIPRECISION == 1 && !defined(__GNUC__))

    const int64_t scalingShift; //shift for fixed-point arithmetic
    const T scaling;
    const int minX;
    const int maxX;

    const T centerX;
    const T centerY;
    const T scaleFactor;
    const T limit;              // the end value at which kernel iterations stop

    static constexpr int MAGNITUDE_BITS = 3;
};

#endif // PARAMETERMAKER_H
