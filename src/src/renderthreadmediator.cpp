#include "renderthreadmediator.h"
#include "renderworker.h"


/************************************************
 *  Dynamic Task algorithm helper class
 *
 *  This class acts as a mediator between
 *  compute task (worker)threads to simplify
 *  busy/idle thread interactions and help
 *  eliminate shared data problems.
 *
 ************************************************/

using MandelBrotRenderer::nonExistentSegmentIndex;

RenderThreadMediator::RenderThreadMediator(MandelBrotRenderer::RendererData& rendererData) :
    rendererData(rendererData),
    allocationUnderway(false), requestUnderway(false),
    readyThreadIndex{}, busyThreadCount(0), waitingThreadCount(0), sharedSegmentCount(0)
{
    waitingThreads.fill({});
    doneThreads.fill(false);
}


bool RenderThreadMediator::getEnabled() const
{
    return rendererData.m_threadMediatorEnabled;
}

void RenderThreadMediator::setEnabled(bool value)
{
    rendererData.m_threadMediatorEnabled = value;
}

void RenderThreadMediator::addThreadToWaitingQueue(uint threadIndex)
{
    readyThreadIndex.store(static_cast<int>(threadIndex));
    waitingThreads.at(threadIndex) = static_cast<int>(threadIndex);

}

void RenderThreadMediator::removeThreadFromWaitingQueue(uint threadIndex)
{
    waitingThreads.at(threadIndex).reset();
}


bool RenderThreadMediator::threadIsInWaitingQueue(uint threadIndex) const
{
    if (!waitingThreads.at(threadIndex).has_value())
    {
        return false;
    }
    return (waitingThreads.at(threadIndex) == static_cast<int>(threadIndex));
}

bool RenderThreadMediator::threadsAreWaiting() const
{
    bool found = (waitingThreadCount.load() > 0);

    return found;
}

void RenderThreadMediator::resetThreadMediator()
{
    waitingThreads.fill({});

    for (auto& i : doneThreads) {
        i = false;
    }

    readyThreadIndex.store({});

    clearNewTaskSegment();

    waitingThreadCount.store(0);

    setRequestUnderway(false);
    setAllocationUnderway(false);
}

void RenderThreadMediator::processThreadGoesIdle(int threadIndex)
{
    if (threadIndex == readyThreadIndex.load())
    {
        readyThreadIndex.store({});
    }
}

bool RenderThreadMediator::performDynamicThreadOperation(RenderWorker* workerThread, MandelBrotRenderer::dynamicThreadAction taskType)
{
    if (taskType == MandelBrotRenderer::dynamicThreadAction::requestTask)
    {
        workerThread->publishState(MandelBrotRenderer::threadState::waiting);
        return requestNewTask(workerThread);
    }

    if (threadsAreWaiting())
    {
        Q_ASSERT(taskType == MandelBrotRenderer::dynamicThreadAction::splitTask);
        return shareTask(workerThread);
    }

    return false;
}

/*
 *
 * Carry out the negotiations to find a new task
 * for an idle thread
 *
 *
 * returns true if a new task was found for the passed-in thread,
 * otherwise false (if it's deemed there is no chance of
 * finding one)
 */

bool RenderThreadMediator::requestNewTask(RenderWorker* workerThread)
{
    std::atomic<bool> newTaskReceived(false);

    newTaskReceived.store(false);

    if (!getEnabled())
    {
        /* the dynamic tasks algorithm is disabled, finish now */
        workerThread->publishState(MandelBrotRenderer::threadState::idle);
       return false;
    }
    // find a shared task, keep trying if it's not yet available (but expected)
    while(
          (BusyThreadsExist() ||
           computeSegmentsAreReadyForSharing()) &&
          !newTaskReceived.load())
    {
        {
            QMutexLocker locker(&mutex);
            int workerThreadIndex = workerThread->getThreadIndex();

            if (!readyThreadIndex.load().has_value() ||
                    (doneThreads.at(static_cast<size_t>(readyThreadIndex.load().value())) &&
                     !doneThreads.at(static_cast<size_t>(workerThreadIndex))))
            {
                addThreadToWaitingQueue(static_cast<uint>(workerThread->getThreadIndex()));
                readyThreadIndex.store(workerThreadIndex);
                waitingThreadCount.store(1);
            }
            else
            {
                int workerThreadIndex = workerThread->getThreadIndex();
                bool threadIsReady =  (readyThreadIndex.load() == workerThreadIndex);

                if (computeSegmentsAreReadyForSharing() &&
                        threadIsReady) {

                    workerThread->setSegment(getNewTaskSegment());
                    clearNewTaskSegment();
                    newTaskReceived.store(true);
                    removeThreadFromWaitingQueue(static_cast<uint>(workerThread->getThreadIndex()));
                    waitingThreadCount.store(0);
                    readyThreadIndex.store({});
                    setAllocationUnderway(false);
                }
            }

            locker.unlock();
        }
    }

    if (!newTaskReceived.load())
    {
        workerThread->publishState(MandelBrotRenderer::threadState::idle);
    }

    return newTaskReceived.load();
}

bool RenderThreadMediator::shareTask(RenderWorker *workerThread)
{
    QMutexLocker locker(&mutex);
    bool unused = false;
    if (!getAllocationUnderway()) {
        if (waitingThreadCount.load() != 0) {
            setAllocationUnderway(true);
            workerThread->shareTask();
        }
    }

    return unused;
}

void RenderThreadMediator::cacheComputeSegmentForSharing(const ComputedDataSegment& newTaskSegment)
{
    Q_ASSERT(segmentsForSharing.getSegmentIndex() == nonExistentSegmentIndex);

    segmentsForSharing = newTaskSegment;
    sharedSegmentCount.store(1);
}

bool RenderThreadMediator::computeSegmentsAreReadyForSharing() const
{
    return (sharedSegmentCount.load() > 0);
}

ComputedDataSegment &RenderThreadMediator::getNewTaskSegment()
{
    Q_ASSERT(computeSegmentsAreReadyForSharing());
    Q_ASSERT(segmentsForSharing.getSegmentIndex() != nonExistentSegmentIndex);

    return segmentsForSharing;
}

void RenderThreadMediator::clearNewTaskSegment()
{
    segmentsForSharing.clearSegmentIndex();
    sharedSegmentCount.store(0);
}

void RenderThreadMediator::setEnabledByState(int state)
{
    if (state == Qt::Checked)
    {
        setEnabled(false);
    }
    else if (state == Qt::Unchecked)
    {
        setEnabled(true);
    }
}

bool RenderThreadMediator::getRequestUnderway() const
{
    return requestUnderway.load();
}

void RenderThreadMediator::setRequestUnderway(bool value)
{
    requestUnderway = value;
}

void RenderThreadMediator::setThreadDone(uint threadIndex)
{
    Q_ASSERT(threadIndex < MandelBrotRenderer::MAX_NUM_WORKER_THREADS);
    doneThreads.at(threadIndex) = true;
}

bool RenderThreadMediator::getAllocationUnderway() const
{
    return allocationUnderway.load();
}

void RenderThreadMediator::setAllocationUnderway(bool value)
{
    allocationUnderway = value;
}
