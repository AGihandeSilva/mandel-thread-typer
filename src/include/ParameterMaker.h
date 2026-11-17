#ifndef PARAMETERMAKER_H
#define PARAMETERMAKER_H

#include "computeddatasegment.h"
#include "PrecisionHandler.h"
#include "MandelParams.h"

/*
 * Parameter helper class and related operations
 * as required by the ComputeTask class
 *
 * scaling for fixed-point arithmetic flows is carried out here
 */

/*
 * Constructor for float types
 */
template <typename T>
struct ParameterMaker
{
#ifdef MANDELBROT_USE_SFINAE
    template <typename TT, typename U = ComputedDataSegment>
    explicit ParameterMaker(typename std::enable_if<!MandelParams::needs_scale_shift<TT>::value &&
                                        MandelParams::multiply_by_float_supported<TT>::value, U>::type dataSegment, TT limitValue, TT) :
#else
    template <typename TT, typename U = ComputedDataSegment> requires IsFloatLikeType<TT>
    explicit ParameterMaker(U dataSegment, TT limitValue, TT) :
#endif
        m_scaling(1),
        m_minX(dataSegment.getMinX()),
        m_maxX(dataSegment.getMaxX()),
        m_originX(MandelBrotRenderer::generateFloatFromPreciseString(dataSegment.getPreciseOriginX()).first),
        m_originY(MandelBrotRenderer::generateFloatFromPreciseString(dataSegment.getPreciseOriginY()).first),
        m_scaleFactor(static_cast<T>(dataSegment.getScaleFactor())),
        m_limit(limitValue)
    {}


/*
 * Constructor for 'Normal' integer types
 */
    
#ifdef MANDELBROT_USE_SFINAE
    template <typename TT, typename U = ComputedDataSegment>
    explicit ParameterMaker(typename std::enable_if<MandelParams::needs_scale_shift<TT>::value &&
                                            MandelParams::multiply_by_float_supported<TT>::value, U>::type dataSegment, TT limitValue, TT scaling) :
#else
    template <typename TT, typename U = ComputedDataSegment> requires IsIntegerLikeType<TT>
    explicit ParameterMaker(U dataSegment, TT limitValue, TT scaling) :
#endif
    m_scaling(static_cast<int64_t>(scaling)),
    m_minX(dataSegment.getMinX()),
    m_maxX( dataSegment.getMaxX()),
    m_originX(static_cast<T>(m_scaling * dataSegment.getOriginX())),
    m_originY(static_cast<T>(m_scaling * dataSegment.getOriginY())),
    m_scaleFactor(static_cast<T>(m_scaling * dataSegment.getScaleFactor())),
    m_limit(static_cast<T>(m_scaling * m_scaling * limitValue))
{}


/*
 * Constructor for Boost multiprecision integer types
 */
    template <typename U = ComputedDataSegment>
    explicit ParameterMaker(ComputedDataSegment dataSegment, MandelBrotRenderer::Int128 limitValue, MandelBrotRenderer::Int128 scaling) :
        m_scaling(scaling),
        m_minX(dataSegment.getMinX()),
        m_maxX(dataSegment.getMaxX()),
        //multiplication by floating point types is not supported for this boost type, convert scaling factor to a fixed point value
        m_originX((m_scaling * static_cast<int64_t>(dataSegment.getOriginX() * doubleToIntScaling)) >> doubleToIntShift),
        m_originY((m_scaling * static_cast<int64_t>(dataSegment.getOriginY() * doubleToIntScaling)) >> doubleToIntShift),
        m_scaleFactor((m_scaling * static_cast<int64_t>(dataSegment.getScaleFactor() * doubleToIntScaling)) >> doubleToIntShift),
        m_limit(m_scaling * m_scaling * limitValue)
    {}


    const int doubleToIntShift = 56;
    const int64_t doubleToIntScaling = 1LL << doubleToIntShift;

    const T m_scaling;
    const int m_minX;
    const int m_maxX;

    const T m_originX;
    const T m_originY;
    const T m_scaleFactor;
    const T m_limit;              // the end value at which kernel iterations stop

    static constexpr int MAGNITUDE_BITS = 3;
};

#endif // PARAMETERMAKER_H
