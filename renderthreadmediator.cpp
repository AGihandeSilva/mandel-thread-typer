#include "renderthreadmediator.h"
#include "renderworker.h"

RenderThreadMediator::RenderThreadMediator(MandelBrotRenderer::RendererData& rendererData) :
    rendererData(rendererData),
    allocationUnderway(false), requestUnderway(false),
    readyThreadIndex(nonExistentThreadIndex), busyThreadCount(0),  waitingThreadCount(0), sharedSegmentCount(0)
{
    waitingThreads.fill(nonExistentThreadIndex);
    doneThreads.fill(false);
}


bool RenderThreadMediator::getEnabled() const
{
    return rendererData.threadMediatorEnabled;
}

void RenderThreadMediator::setEnabled(bool value)
{
    rendererData.threadMediatorEnabled = value;
}

void RenderThreadMediator::addThreadToWaitingQueue(uint threadIndex)
{
    readyThreadIndex.store(static_cast<int>(threadIndex));
    waitingThreads.at(threadIndex) = static_cast<int>(threadIndex);

}

void RenderThreadMediator::removeThreadFromWaitingQueue(uint threadIndex)
{
    waitingThreads.at(threadIndex) = nonExistentThreadIndex;
}


bool RenderThreadMediator::threadIsInWaitingQueue(uint threadIndex) const
{
    return (waitingThreads.at(threadIndex) == static_cast<int>(threadIndex));
}

bool RenderThreadMediator::threadsAreWaiting() const
{
    bool found = (waitingThreadCount.load() > 0);

    return found;
}

void RenderThreadMediator::resetThreadMediator()
{

    for (auto& i : waitingThreads) {
        i = nonExistentThreadIndex;
    }

    for (auto& i : doneThreads) {
        i = false;
    }

    readyThreadIndex.store(nonExistentThreadIndex);

    clearNewTaskSegment();

    waitingThreadCount.store(0);

    setRequestUnderway(false);
    setAllocationUnderway(false);
}

void RenderThreadMediator::processThreadGoesIdle(int threadIndex)
{
    if (threadIndex == readyThreadIndex.load())
    {
        readyThreadIndex.store(nonExistentThreadIndex);
    }
}

bool RenderThreadMediator::performDynamicThreadOperation(RenderWorker* workerThread, dynamicThreadAction taskType)
{
    if (taskType == dynamicThreadAction::requestTask)
    {
        workerThread->publishState(threadState::waiting);
        return requestNewTask(workerThread);
    }

    if (threadsAreWaiting())
    {
        Q_ASSERT(taskType == dynamicThreadAction::splitTask);
        return shareTask(workerThread);
    }

    return false;
}

bool RenderThreadMediator::requestNewTask(RenderWorker* workerThread)
{
    std::atomic<bool> newTaskReceived(false);

    newTaskReceived.store(false);

    if (!getEnabled())
    {
        workerThread->publishState(threadState::idle);
       return false;
    }
    while(
          (BusyThreadsExist() ||
           computeSegmentsAreReadyForSharing()) &&
          !newTaskReceived.load())
    {
        {
            QMutexLocker locker(&mutex);
            int workerThreadIndex = workerThread->getThreadIndex();

            if (readyThreadIndex.load() == nonExistentThreadIndex ||
                    (doneThreads.at(static_cast<size_t>(readyThreadIndex.load())) &&
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
                    readyThreadIndex.store(nonExistentThreadIndex);
                    setAllocationUnderway(false);
                }
            }

            locker.unlock();
        }
    }

    if (!newTaskReceived.load())
    {
        workerThread->publishState(threadState::idle);
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

            //TODO: check for the case when this is the only thread left (or maybe there are very few left?)
            // and don't share then.
            workerThread->shareTask();
        }
    }

    return unused;
}

void RenderThreadMediator::cacheComputeSegmentForSharing(const ComputedDataSegment& newTaskSegment)
{
    Q_ASSERT(segmentsForSharing.getSegmentIndex() == nonExistentThreadIndex);

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
    Q_ASSERT(segmentsForSharing.getSegmentIndex() != nonExistentThreadIndex);

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
    Q_ASSERT(threadIndex < MAX_NUM_WORKER_THREADS);
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
