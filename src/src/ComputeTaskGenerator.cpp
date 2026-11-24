#include "ComputeTaskGenerator.h"
#include "ParameterMaker.h"
#include "renderworker.h"


/**********************************************
 *  Compute Task code
 *
 * The bulk of compute time should (hopefully)
 * be spent in code here
 **********************************************/

template<typename T>
concept UsesIntegerRepresentation = std::integral<T>
|| std::same_as<T, MandelBrotRenderer::Int128>
;

template<typename T>
      concept UsesFloatingPointRepresentation = std::floating_point<T>
    || std::same_as<T, MandelBrotRenderer::Float80>
    || std::same_as<T, MandelBrotRenderer::Float128>
    || std::same_as<T, MandelBrotRenderer::CustomFloat>
    || std::same_as<T, MandelBrotRenderer::Float20dd>
    || std::same_as<T, MandelBrotRenderer::Float30dd>
    || std::same_as<T, MandelBrotRenderer::Float50dd>
;

template <UsesIntegerRepresentation T, int64_t SCALING_SHIFT>
T normalize(T t) noexcept
{
    return (t >> SCALING_SHIFT);
}

template<UsesIntegerRepresentation T>
bool checkEndCondition(const T& value,
    const T& limit) noexcept {
    return value > limit || value < 0;
}

template<UsesFloatingPointRepresentation T, int64_t >
T normalize(const T t) noexcept { return t; }

template<UsesFloatingPointRepresentation T>
bool checkEndCondition(const T& value, const T& limit) noexcept {
    return value > limit;
}
/*
 * This templated class represents
 * the compute kernel task as executed with a given numeric type
 *
 * It works in conjunction with a templated ParameterMaker class
 * which prepares the correct parameter values for the kernel
 * to execute the task given the type used.
 *
 */

template <typename T, int64_t SCALE_SHIFT>
ComputeTaskGenerator<T, SCALE_SHIFT>::ComputeTaskGenerator(RenderWorker& workerOwner) : workerOwner(workerOwner){}

template <typename T, int64_t SCALE_SHIFT>
RenderWorker::computeFunction ComputeTaskGenerator<T, SCALE_SHIFT>::generateComputeTask()
{
        return ([this] (const ComputedDataSegment& segment, const bool& abort, int& currentPixelIndex, MandelBrotRenderer::ComputeTaskResults& resultData, int y)
                {
#if defined(DEBUG_RAW_DATA)
                    const bool isDebugRow = y == 0;
#endif

                    const uint pass = workerOwner.getPassValue();
                    const uint MaxIterations = RenderWorker::calcMaxIterations(pass);

                    const auto limit = static_cast<T>(4);

                    //generate parameters appropriate to the task and chosen numeric type
                    MandelBrotRenderer::Int128 fixedScaling{ 1 };
                    fixedScaling <<= SCALE_SHIFT;
                    const ParameterMaker<T> newParams(segment, limit, fixedScaling);
                    const T scaledYOrigin = newParams.m_originY + (static_cast<T>(y) * newParams.m_scaleFactor);

                    const MandelBrotRenderer::setType setToGenerate =  workerOwner.getSetToGenerate();
                    const T ay = static_cast<T>((setToGenerate == MandelBrotRenderer::setType::mandelbrot) ? scaledYOrigin : newParams.m_originY);

                    const MandelBrotRenderer::colorMapStore& colormap = workerOwner.getColormap();
                    const T iterationColourScale =  static_cast<T>(workerOwner.getIterationColourScale());

                for (int x = newParams.m_minX; x < newParams.m_maxX && !abort; ++x) {
                    const T ax = (setToGenerate == MandelBrotRenderer::setType::mandelbrot) ? newParams.m_originX + (static_cast<T>(x) * newParams.m_scaleFactor) :
                                                                   newParams.m_originX;

                    T a1 = (setToGenerate == MandelBrotRenderer::setType::mandelbrot) ? ax : newParams.m_originX + (static_cast<T>(x) * newParams.m_scaleFactor);

                    T b1 = static_cast<T>((setToGenerate == MandelBrotRenderer::setType::mandelbrot) ? ay : scaledYOrigin);
                    uint numIterations = 0;

//TODO: consider optimizations of the normalization scheme

                     /**************************************
                     *
                     * mandelbrot (julia) set calculations
                     *
                     **************************************/
                    do {
                        if (++numIterations >= MaxIterations)
                            break;

                        T a2 = (a1 * a1) - (b1 * b1);
                        a2 = normalize <T, SCALE_SHIFT >(a2);
                        a2 += ax;

                        T b2 = (2 * a1 * b1);
                        b2 = normalize <T, SCALE_SHIFT >(b2);
                        b2 += ay;

                        const T mod2Sq = (a2 * a2) + (b2 * b2);
                        if (checkEndCondition(mod2Sq, newParams.m_limit))
                            break;

                        if (++numIterations >= MaxIterations)
                            break;
                        a1 = (a2 * a2) - (b2 * b2);
                        a1 = normalize <T, SCALE_SHIFT >(a1);
                        a1 += ax;

                        b1 = (2 * a2 * b2);
                        b1 = normalize <T, SCALE_SHIFT >(b1);
                        b1 += ay;

                        const T mod1Sq = (a1 * a1) + (b1 * b1);
                        if (checkEndCondition(mod1Sq, newParams.m_limit))
                            break;

                    }
                    while (true);

                    if (numIterations < MaxIterations) {
                        (*(resultData.m_rawResultData))[currentPixelIndex] = colormap[static_cast<uint>(static_cast<T>(numIterations) * iterationColourScale)
                                                                       % colormap.size()];


                    }
#if defined(DEBUG_RAW_DATA)
                    //Maintain a raw numIterations result for debug and possible filter use (if filtering is enabled)
                    if (isDebugRow)
                        (*(resultData.m_rawIterationsData))[x - newParams.m_minX] = numIterations;
#endif
                    resultData.m_iterationSum += numIterations;
                    ++currentPixelIndex;
                }
        });
}
