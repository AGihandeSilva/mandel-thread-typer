#ifndef RENDERTHREADMEDIATOR_H
#define RENDERTHREADMEDIATOR_H

#include <QMutex>
#include <QObject>

#include "computeddatasegment.h"
#include "mandelbrotrenderer.h"

#include <iostream>
#include <array>

class RenderWorker;

/************************************************
 *  Dynamic Task algorithm helper class
 *
 *  This class acts as a mediator between
 *  compute task (worker)threads to simplify
 *  busy/idle thread interactions and help
 *  eliminate shared data problems.
 *
 ************************************************/

class RenderThreadMediator  : public QObject
{
    Q_OBJECT

public:
    RenderThreadMediator(MandelBrotRenderer::RendererData& rendererData);


    bool getEnabled() const;
    void setEnabled(bool value);

    void addThreadToWaitingQueue(uint threadIndex);
    void removeThreadFromWaitingQueue(uint threadIndex);
    bool threadsAreWaiting() const;
    bool threadIsInWaitingQueue(uint threadIndex) const;
    void resetThreadMediator();
    void resetBusyThreadCount() { busyThreadCount.store(0); }
    void processThreadGoesIdle(int threadIndex);
    bool performDynamicThreadOperation(RenderWorker* workerThread, MandelBrotRenderer::dynamicThreadAction taskType);

    void cacheComputeSegmentForSharing(const ComputedDataSegment& newTaskSegment);
    bool computeSegmentsAreReadyForSharing() const;

    ComputedDataSegment& getNewTaskSegment();
    void clearNewTaskSegment();

    void incrementBusyThreadCount() {
        ++busyThreadCount;
        //std::cout << "threadCount++ ->  " << busyThreadCount << std::endl;
    }
    void decrementBusyThreadCount() {
        --busyThreadCount;
        //std::cout << "threadCount-- ->  " << busyThreadCount << std::endl;
        Q_ASSERT(busyThreadCount.load() >= 0);
    }
    bool BusyThreadsExist() const { return busyThreadCount.load() > 0; }
    int getBusyThreadCount() const { return busyThreadCount.load(); }

    bool getAllocationUnderway() const;
    void setAllocationUnderway(bool value);

    bool getRequestUnderway() const;
    void setRequestUnderway(bool value);

    void setThreadDone(uint threadIndex);

public slots:
    void setEnabledByState(int state);

private:

    bool requestNewTask(RenderWorker* workerThread);
    bool shareTask(RenderWorker* workerThread);

    MandelBrotRenderer::RendererData& rendererData;

    std::atomic<bool> allocationUnderway;
    std::atomic<bool> requestUnderway;

    mutable QMutex mutex;

    std::atomic<int> readyThreadIndex;
    std::array<int, MandelBrotRenderer::MAX_NUM_WORKER_THREADS> waitingThreads {};
    std::array<bool, MandelBrotRenderer::MAX_NUM_WORKER_THREADS> doneThreads {};

    std::atomic<int> busyThreadCount;
    std::atomic<int> waitingThreadCount;

    ComputedDataSegment segmentsForSharing;
    std::atomic<int> sharedSegmentCount;

};

#endif // RENDERTHREADMEDIATOR_H
