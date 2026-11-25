/****************************************************************************
**
****************************************************************************/

#include "renderworker.h"

#include <QtWidgets>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <future>
#include <functional>
#include <type_traits>

#ifdef __GNUC__
#include <quadmath.h>
#endif

#include "mandelbrotwidget.h"
#include "computeddatasegment.h"
#include "renderthreadmediator.h"
#include "ComputeTaskGenerator.h"
#if defined(DEBUG_RAW_DATA)
#include <boost/math/statistics/univariate_statistics.hpp>
#include <algorithm>
#endif

using namespace MandelBrotRenderer;

int RenderWorker::count  = 0;

RenderWorker::RenderWorker(RenderThread* parentThread,
                                       MandelbrotWidget* owner,
                                       uint currentPassValue,
                                       uint finalPassValue,
                                       colorMapStore& colormap,
                                       bool restart,
                                       const bool& abort,
                                       int threadIndex,
                                       ComputedDataSegment&& segment,
                                       MandelBrotRenderer::haltChecker abortChecker,
                                       QMutex&  pauseMutex)
    : parentThread(parentThread),
      internalData{owner, currentPassValue},
      pass(currentPassValue),
      finalPassValue(finalPassValue),
      MaxMaxIterations(calcMaxIterations(finalPassValue)),
      iterationColourScale(static_cast<double>(MaxMaxIterations) / static_cast<double>(colormap.size())),
      colormap(colormap),
      restart(restart),
      abort(abort),
      threadIndex(threadIndex),
      segment{std::move(segment)},
      currentYPosition(nonExistentPixelLinePosition),
      pauseMutex(pauseMutex),
      pointsDone(0),
      cleanedUp(false),
      computationCompleted(false),
      setToGenerate(setType::mandelbrot),
      m_ownerWidget(owner),
      abortChecker{std::move(abortChecker)}
{
    ++count;

    connect(this, SIGNAL(renderedSubImage(const QImage*,QString)), owner, SLOT(updatePixmap(const QImage*,QString)));

    connect(this, SIGNAL(finished()), parentThread, SLOT(cleanup()));
    connect(this, SIGNAL(computationDone(ComputedDataSegment&)), parentThread, SLOT(drawPreComputedData(ComputedDataSegment&)));
    connect(this, SIGNAL(taskDone()), parentThread, SLOT(markThreadProgressComplete()));

    connect(parentThread, SIGNAL(sendRestart(bool)), this, SLOT(setRestart(bool)));

    connect(this, SIGNAL(writeToLog(QString,bool)), m_ownerWidget, SLOT(outputToLog(QString,bool)));

    this->m_computeResult = std::async(std::launch::async, &RenderWorker::getComputeResult, this);
}

void RenderWorker::setRestart(bool restart)
{
        emit writeToLog("thread restart: " + QString::number(restart));
        this->restart = restart;
}

void RenderWorker::publishState(threadState state)
{
    parentThread->setThreadState(static_cast<uint>(threadIndex), state);
}

void RenderWorker::shareTask()
{
    if (!parentThread->getThreadMediator().getEnabled())
    {
       return;
    }
    Q_ASSERT(currentYPosition <= segment.getMaxY() - MIN_REALLOCATION_SIZE_IN_PIXELS);
    Q_ASSERT(parentThread->getThreadMediator().threadsAreWaiting());

    QMutexLocker locker(&mutex);
    ComputedDataSegment segmentForOtherThread(segment);
    RegionAttributes newAttributesForOtherThread = segment.getAttributes();
    newAttributesForOtherThread.adjustYValues(currentYPosition, true);
    segmentForOtherThread.ChangeRegionAttributes(newAttributesForOtherThread);
    segmentForOtherThread.clearRawData();

    parentThread->getThreadMediator().cacheComputeSegmentForSharing(segmentForOtherThread);

    RegionAttributes newAttributes = segment.getAttributes();
    newAttributes.adjustYValues(currentYPosition, false);

    segment.ChangeRegionAttributes(newAttributes);

    publishState(threadState::shared);
}


void RenderWorker::handleSegmentDone()
{
    QMutexLocker locker(&mutex);

    if (abort) {
       // std::cout << "pass : " << pass << " thread : " << threadIndex <<
       //              " finished after abort " << std::endl;
    }
    computationCompleted = true;
    emit computationDone(segment);
    emit writeToLog("computation done, thread: " + QString::number(threadIndex));
}

/*
 *
 * Given a task object using the specified type, execute
 * kernel computations, interacting with a mediator object to
 * dynamically adjust the task parameters if required
 */

bool RenderWorker::execute(const computeFunction& computeTask)
{
    bool result = false;

    int pauseLoopCount = 0;
    constexpr int pauseInterval = 5;

    emit writeToLog("worker running, thread index: " + QString::number(threadIndex));

    if (segment.hasBeenUsed())
    {
        //std::cout << "data already used!" << std::endl;
    }

    bool newTaskReceived = true;

    publishState(threadState::busy);
    while (newTaskReceived) {

        ComputeTaskResults& fullResultData = segment.getFullResultData();
        int currentPixelIndex = 0;
        parentThread->getThreadMediator().incrementBusyThreadCount();

        for (int y = segment.getMinY(); y < segment.getMaxY(); ++y) {
            if (restart || abort) {
                handleSegmentDone();
                publishState(threadState::idle);
                parentThread->getThreadMediator().decrementBusyThreadCount();
                return result;
            }

            if (++pauseLoopCount > pauseInterval)
            {
                QMutexLocker locker(&mutex);
                pauseMutex.lock();
                pauseLoopCount = 0;
                pauseMutex.unlock();
            }

            if (segment.getMaxY() - y <= MIN_REALLOCATION_SIZE_IN_PIXELS ) {
                publishState(threadState::finishing);
            }
            else {
                currentYPosition = y;
                //negotiate with the thread mediator to see if this task should split so it can be shared
                parentThread->getThreadMediator().performDynamicThreadOperation(this, dynamicThreadAction::splitTask);
            }
            /***************************************
             * calculate the fractal pixel values! *
             ***************************************/
            computeTask(segment, abort, currentPixelIndex, fullResultData, y);

#if defined(DEBUG_RAW_DATA)
            //statistics for debugging purposes
            if (y == 0 && fullResultData.m_rawIterationsData)
            {
                std::vector<uint> rawValues((fullResultData.m_rawIterationsData)->size());
                rawValues.assign(fullResultData.m_rawIterationsData->begin(), fullResultData.m_rawIterationsData->end());


                const double mean = boost::math::statistics::mean(rawValues.cbegin(), rawValues.cend());
                const double sigma_sq = boost::math::statistics::variance(std::execution::seq, rawValues.cbegin(), rawValues.cend());
                const auto minIter = std::min_element(rawValues.cbegin(), rawValues.cend());
                const uint minValue = minIter != rawValues.cend() ? *minIter : 0;
                const auto maxIter = std::max_element(rawValues.cbegin(), rawValues.cend());
                const uint maxValue = maxIter != rawValues.cend() ? *maxIter : 0;
                std::cout << "row 0 size: " << rawValues.size() << " mean:" << mean << " : " << "var : " << sigma_sq << " min: " << minValue << \
                " max: " << maxValue << " : currentPixel : " << currentPixelIndex << std::endl;

                //for (auto i : rawValues)
                //    std::cout << i << std::endl;
            }
#endif
        }
        handleSegmentDone();

        publishState(threadState::idle);
        parentThread->getThreadMediator().decrementBusyThreadCount();

        //as this task is idle, contact the thread mediator to request a new compute task
        //this call will block this thread if a new task looks like it can be found
        newTaskReceived = parentThread->getThreadMediator().performDynamicThreadOperation(this, dynamicThreadAction::requestTask);
        if (newTaskReceived) {
            publishState(threadState::restarted);
        }
    }
    emit taskDone();
    result = true;
    publishState(threadState::idle);
    parentThread->getThreadMediator().setThreadDone(static_cast<uint>(threadIndex));
    return result;
}

/*
 *
 * Generate a compute task using the specified type
 * and execute the compute flow (with assistance from a mediator object)
 *
 */

#ifdef MANDELBROT_USE_SFINAE
template <typename T>
typename std::enable_if <!std::is_same<T, MandelBrotRenderer::Int128>::value &&
                    MandelParams::needs_scale_shift<T>::value, bool>::type
RenderWorker::executeTask()
#else
template <typename T> requires NeedsScaleShift<T>
bool RenderWorker::executeTask()
#endif
{
    //shift required for fixed-point arithmetic
    constexpr auto scalingShift = static_cast<int>((((sizeof(T) * CHAR_BIT) / 2) - (MAGNITUDE_BITS * 2)));

    ComputeTaskGenerator<T, scalingShift> taskGenerator(*this );
    RenderWorker::computeFunction computeTask = taskGenerator.generateComputeTask();
    return (execute(computeTask));
}

#ifdef MANDELBROT_USE_SFINAE
template <typename T, int Scaling>
typename std::enable_if<!MandelParams::needs_scale_shift<T>::value&&
    MandelParams::multiply_by_float_supported<T>::value, bool>::type
    RenderWorker::executeTask()
#else
template <typename T, int Scaling> requires IsFloatLikeType<T>
bool RenderWorker::executeTask()
#endif
{
    ComputeTaskGenerator<T, Scaling> taskGenerator(*this);
    RenderWorker::computeFunction computeTask = taskGenerator.generateComputeTask();
    return (execute(computeTask));
}

#ifdef MANDELBROT_USE_SFINAE
template <typename T>
typename std::enable_if<std::is_same<T, MandelBrotRenderer::Int128>::value, bool>::type
RenderWorker::executeTask()
#else
template <>
bool RenderWorker::executeTask<MandelBrotRenderer::Int128>()
#endif
{
    constexpr int scalingShift = 56;
    ComputeTaskGenerator<MandelBrotRenderer::Int128, scalingShift> taskGenerator(*this);
    RenderWorker::computeFunction computeTask = taskGenerator.generateComputeTask();
    return (execute(computeTask));
}

/*
 *
 * Entry point for the compute flow, currently called by the constructor
 * with std::launch::async (force a new thread)
 *
 */
bool RenderWorker::getComputeResult()
{
    static MandelBrotRenderer::setType setTypeSetting = setType::mandelbrot;
    setToGenerate = setTypeSetting;

    bool result;

    /*
     * Generate a task object encapsulating the computations to be done with the
     * specified type
     */
    if (parentThread->getRendererData().m_numericType == internalDataType::singlePrecisionFloat)
    {
        result = executeTask<float>();
    } else if (parentThread->getRendererData().m_numericType == internalDataType::int32) {
        result = executeTask<int32_t>();
    } else if (parentThread->getRendererData().m_numericType == internalDataType::int64) {
        result = executeTask<int64_t>();
    }else if (parentThread->getRendererData().m_numericType == internalDataType::customFloat20) {
        result = executeTask<CustomFloat>();
    }else if (parentThread->getRendererData().m_numericType == internalDataType::float20dd) {
        result = executeTask<Float20dd>();
    }else if (parentThread->getRendererData().m_numericType == internalDataType::float30dd) {
        result = executeTask<Float30dd>();
    }else if (parentThread->getRendererData().m_numericType == internalDataType::float50dd) {
        result = executeTask<Float50dd>();
    } else if (parentThread->getRendererData().m_numericType == internalDataType::float80) {
        result = executeTask<Float80>();
    } else if (parentThread->getRendererData().m_numericType == internalDataType::float128) {
        result = executeTask<Float128>();
    } else if (parentThread->getRendererData().m_numericType == internalDataType::float256) {
        result = executeTask<Float256>();
    } else if (parentThread->getRendererData().m_numericType == internalDataType::int128) {
        result = executeTask<Int128>();
    } else  {
        result = executeTask<double>();
    }

    emit finished();
    return result;
}



double RenderWorker::getIterationColourScale() const
{
    return iterationColourScale;
}

colorMapStore &RenderWorker::getColormap() const
{
    return colormap;
}

MandelBrotRenderer::setType RenderWorker::getSetToGenerate() const
{
    return setToGenerate;
}

void RenderWorker::setSegment(const ComputedDataSegment &value)
{
    segment = value;
}

int RenderWorker::getThreadIndex() const
{
    return threadIndex;
}


ComputedDataSegment& RenderWorker::getComputedData()
{
    return segment;
}
