#pragma once

#include "computeddatasegment.h"
#include "PrecisionHandler.h"

/*
 * Definitions for detailed handling of different types
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

    template <>
    struct needs_scale_shift<int32_t> {
        static const bool value = true;
    };

    template <>
    struct needs_scale_shift<int64_t> {
        static const bool value = true;
    };
}

#ifndef MANDELBROT_USE_SFINAE

template <typename T>
concept IsFloatLikeType = !MandelParams::needs_scale_shift<T>::value &&
MandelParams::multiply_by_float_supported<T>::value;

template <typename T>
concept IsIntegerLikeType = MandelParams::needs_scale_shift<T>::value &&
MandelParams::multiply_by_float_supported<T>::value;


template <typename T>
concept NeedsScaleShift = MandelParams::needs_scale_shift<T>::value;

#endif
