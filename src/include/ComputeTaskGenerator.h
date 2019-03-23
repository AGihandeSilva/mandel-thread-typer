#ifndef COMPUTETASK_H
#define COMPUTETASK_H

#include <functional>
#include "mandelbrotrenderer.h"
#include "computeddatasegment.h"
#include "renderworker.h"

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
