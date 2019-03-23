#include "ComputeTaskGenerator.h"
#include "ParameterMaker.h"
#include "renderworker.h"


/**********************************************
 *  Compute Task code
 *
 * The bulk of compute time should (hopefully)
 * be spent in code here
 **********************************************/

/*
 * Miscellaneous functions associated with the compute task kernel
 *
 */

template <typename T, MandelBrotRenderer::enable_if_t<std::is_floating_point<T>::value>* = nullptr>
            bool checkEndCondition(const T& value, const T& limit) noexcept { return value > limit; }

template <typename T, MandelBrotRenderer::enable_if_t<std::is_integral<T>::value>* = nullptr>
            bool checkEndCondition(const T& value, const T& limit) noexcept { return value > limit || value < 0; }

template <typename T, MandelBrotRenderer::enable_if_t<std::is_floating_point<T>::value>* = nullptr>
                        void normalize(T&, const int64_t&) noexcept {}

template <typename T, MandelBrotRenderer::enable_if_t<std::is_integral<T>::value>* = nullptr>
        void normalize(T& t, const int64_t& scalingShift) noexcept { t = static_cast<T>(t >> scalingShift); }

#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
bool checkEndCondition(const MandelBrotRenderer::Int128& value,
    const MandelBrotRenderer::Int128& limit) noexcept { return value > limit || value < 0; }

void normalize(MandelBrotRenderer::Int128& t, const int64_t& scalingShift) noexcept
                        { t = static_cast<MandelBrotRenderer::Int128>(t >> scalingShift); }

bool checkEndCondition(const MandelBrotRenderer::Float128& value,
    const MandelBrotRenderer::Float128& limit) noexcept { return value > limit; }

void normalize(const MandelBrotRenderer::Float128&, const int64_t&) noexcept {}

bool checkEndCondition(const MandelBrotRenderer::Float80& value,
    const MandelBrotRenderer::Float80& limit) noexcept { return value > limit; }

void normalize(const MandelBrotRenderer::Float80&, const int64_t&) noexcept {}
#endif //(USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)

#if (USE_BOOST_MULTIPRECISION == 1)
bool checkEndCondition(const MandelBrotRenderer::CustomFloat& value,
    const MandelBrotRenderer::CustomFloat& limit) noexcept { return value > limit; }

void normalize(const MandelBrotRenderer::CustomFloat&, const int64_t&) noexcept {}


bool checkEndCondition(const MandelBrotRenderer::Float20dd& value,
    const MandelBrotRenderer::Float20dd& limit) noexcept { return value > limit; }

void normalize(const MandelBrotRenderer::Float20dd&, const int64_t&) noexcept {}

bool checkEndCondition(const MandelBrotRenderer::Float30dd& value,
    const MandelBrotRenderer::Float30dd& limit) noexcept { return value > limit; }

void normalize(const MandelBrotRenderer::Float30dd&, const int64_t&) noexcept {}

bool checkEndCondition(const MandelBrotRenderer::Float50dd& value,
    const MandelBrotRenderer::Float50dd& limit) noexcept { return value > limit; }

void normalize(const MandelBrotRenderer::Float50dd&, const int64_t&) noexcept {}
#endif //(USE_BOOST_MULTIPRECISION == 1)

/*
 * This templated class represents
 * the compute kernel task as executed with a given numeric type
 *
 * It works in conjunction with a templated ParameterMaker class
 * which prepares the correct parameter values for the kernel
 * to execute the task given the type used.
 *
 */

template <typename T>
ComputeTaskGenerator<T>::ComputeTaskGenerator(RenderWorker& workerOwner) : workerOwner(workerOwner){}

template <typename T>
RenderWorker::computeFunction ComputeTaskGenerator<T>::generateComputeTask()
{
        return ([&] (const ComputedDataSegment& segment, const bool& abort, int& currentPixelIndex, MandelBrotRenderer::ComputeTaskResults& resultData, int y)
                {
                    const uint pass = workerOwner.getPassValue();
                    const uint MaxIterations = RenderWorker::calcMaxIterations(pass);

                    const T limit = 4;

                    //generate parameters appropriate to the task and chosen numeric type
                    const ParameterMaker<T> newParams(segment, limit);

                    const MandelBrotRenderer::setType setToGenerate =  workerOwner.getSetToGenerate();
                    const T ay = static_cast<T>((setToGenerate == MandelBrotRenderer::setType::mandelbrot) ? newParams.centerY + (y * newParams.scaleFactor) :
                                                                         newParams.centerY);

                    const MandelBrotRenderer::colorMapStore& colormap = workerOwner.getColormap();
                    const T iterationColourScale =  static_cast<T>(workerOwner.getIterationColourScale());

                for (int x = newParams.minX; x < newParams.maxX && !abort; ++x) {
                    const T ax = static_cast<T>((setToGenerate == MandelBrotRenderer::setType::mandelbrot) ? newParams.centerX + (x * newParams.scaleFactor):
                                                                   newParams.centerX);
                    T a1 = static_cast<T>((setToGenerate == MandelBrotRenderer::setType::mandelbrot) ? ax : newParams.centerX + (x * newParams.scaleFactor));
                    T b1 = static_cast<T>((setToGenerate == MandelBrotRenderer::setType::mandelbrot) ? ay : newParams.centerY + (y * newParams.scaleFactor));
                    uint numIterations = 0;

//TODO: consider optimizations of the normalization scheme

                    /**************************************
                     *
                     * mandelbrot (julia) set calculations
                     *
                     **************************************/
                    do {
                        ++numIterations;
                        T a2 = (a1 * a1) - (b1 * b1);
                        normalize(a2, newParams.scalingShift);
                        a2 += ax;

                        T b2 = (2 * a1 * b1);
                        normalize(b2, newParams.scalingShift);
                        b2 += ay;

                        const T mod2Sq = (a2 * a2) + (b2 * b2);
                        if (checkEndCondition(mod2Sq, newParams.limit))
                            break;

                        ++numIterations;
                        a1 = (a2 * a2) - (b2 * b2);
                        normalize(a1, newParams.scalingShift);
                        a1 += ax;

                        b1 = (2 * a2 * b2);
                        normalize(b1, newParams.scalingShift);
                        b1 += ay;

                        const T mod1Sq = (a1 * a1) + (b1 * b1);
                        if (checkEndCondition(mod1Sq, newParams.limit))
                            break;

                    } while (numIterations < MaxIterations);

                    if (numIterations < MaxIterations) {
                        (*(resultData.rawResultData))[currentPixelIndex] = colormap[static_cast<uint>(static_cast<T>(numIterations) * iterationColourScale)
                                                                       % colormap.size()];

                        //TODO : maintain a raw numIterations result for filter use (if filtering is enabled)
                    }
                    resultData.iterationSum += numIterations;
                    ++currentPixelIndex;
                }
        });
}
