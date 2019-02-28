#ifndef COMPUTETASK_H
#define COMPUTETASK_H

#include <functional>
#include "mandelbrotrenderer.h"
#include "computeddatasegment.h"
#include "renderworker.h"

template <typename T>
class ComputeTaskGenerator
{
public:
    ComputeTaskGenerator(RenderWorker& workerOwner);

    RenderWorker::computeFunction generateComputeTask();


private:
    RenderWorker& workerOwner;
};

#include "ComputeTaskGenerator.cpp"

#endif // COMPUTETASK_H
