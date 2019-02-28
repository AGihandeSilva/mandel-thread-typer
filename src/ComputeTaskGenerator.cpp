#include "ComputeTaskGenerator.h"
#include "ParameterMaker.h"
#include "renderworker.h"

template <typename T>
ComputeTaskGenerator<T>::ComputeTaskGenerator(RenderWorker& workerOwner) : workerOwner(workerOwner){}

template <typename T, enable_if_t<std::is_floating_point<T>::value>* = nullptr>
            bool checkEndCondition(const T& value, const T limit) noexcept { return value > limit; }

template <typename T, enable_if_t<std::is_integral<T>::value>* = nullptr>
            bool checkEndCondition(const T& value, const T limit) noexcept { return value > limit || value < 0; }

template <typename T, enable_if_t<std::is_floating_point<T>::value>* = nullptr>
                        void normalize(T&, int64_t) noexcept {}

template <typename T, enable_if_t<std::is_integral<T>::value>* = nullptr>
        void normalize(T& t, int64_t scalingShift) noexcept { t = static_cast<T>(t >> scalingShift); }

//TODO find some way to reduce the repetition for boost types here
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
bool checkEndCondition(const Int128& value,
    const Int128 limit) noexcept { return value > limit || value < 0; }

void normalize(Int128& t, int64_t scalingShift) noexcept
                        { t = static_cast<Int128>(t >> scalingShift); }

bool checkEndCondition(const Float128& value,
    const Float128 limit) noexcept { return value > limit; }

void normalize(Float128&, int64_t) noexcept {}

bool checkEndCondition(const Float80& value,
    const Float80 limit) noexcept { return value > limit; }

void normalize(Float80&, int64_t) noexcept {}
#endif //(USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)

#if (USE_BOOST_MULTIPRECISION == 1)
bool checkEndCondition(const CustomFloat& value,
    const CustomFloat limit) noexcept { return value > limit; }

void normalize(CustomFloat&, int64_t) noexcept {}


bool checkEndCondition(const Float20dd& value,
    const Float20dd limit) noexcept { return value > limit; }

void normalize(Float20dd&, int64_t) noexcept {}

bool checkEndCondition(const Float30dd& value,
    const Float30dd limit) noexcept { return value > limit; }

void normalize(Float30dd&, int64_t) noexcept {}

bool checkEndCondition(const Float50dd& value,
    const Float50dd limit) noexcept { return value > limit; }

void normalize(Float50dd&, int64_t) noexcept {}
#endif //(USE_BOOST_MULTIPRECISION == 1)

template <typename T>
RenderWorker::computeFunction ComputeTaskGenerator<T>::generateComputeTask()
{
        return ([&] (const ComputedDataSegment& segment, const bool& abort, int& currentPixelIndex, ComputeTaskResults& resultData, int y)
                {
                    const uint pass = workerOwner.getPassValue();
                    const uint MaxIterations = RenderWorker::calcMaxIterations(pass);

                    const T limit = 4;

                    const ParameterMaker<T> newParams(segment, limit);

                    const MandelBrotRenderer::setType setToGenerate =  workerOwner.getSetToGenerate();
                    const T ay = static_cast<T>((setToGenerate == MandelBrotRenderer::setType::mandelbrot) ? newParams.centerY + (y * newParams.scaleFactor) :
                                                                         newParams.centerY);

                    const colorMapStore& colormap = workerOwner.getColormap();
                    const T iterationColourScale =  static_cast<T>(workerOwner.getIterationColourScale());

                for (int x = newParams.minX; x < newParams.maxX && !abort; ++x) {
                    const T ax = static_cast<T>((setToGenerate == MandelBrotRenderer::setType::mandelbrot) ? newParams.centerX + (x * newParams.scaleFactor):
                                                                   newParams.centerX);
                    T a1 = static_cast<T>((setToGenerate == MandelBrotRenderer::setType::mandelbrot) ? ax : newParams.centerX + (x * newParams.scaleFactor));
                    T b1 = static_cast<T>((setToGenerate == MandelBrotRenderer::setType::mandelbrot) ? ay : newParams.centerY + (y * newParams.scaleFactor));
                    uint numIterations = 0;

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
                        //                if (currentPixelIndex > rawResultData.size())
                        //                {
                        //                    throw std::out_of_range("pixel index out of range!");
                        //                }
                        //rawResultData[currentPixelIndex] = colormap.at(numIterations % ColormapSize);

                        (*(resultData.rawResultData))[currentPixelIndex] = colormap[static_cast<uint>(static_cast<T>(numIterations) * iterationColourScale)
                                                                       % colormap.size()];

                        //TODO : maintain a raw numIterations result for filter use (if filtering is enabled)
                    }
                    resultData.iterationSum += numIterations;
                    ++currentPixelIndex;
                }
        });
}
