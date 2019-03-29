/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "renderthread.h"

#include <QtWidgets>
#include <algorithm>

#include "renderworker.h"
#include "mandelbrotwidget.h"
#include "workerthreaddata.h"
#include "windowthreadinfo.h"
#include "settingshandler.h"

#include <cmath>
#include <iostream>
#include <functional>
#include <vector>

using namespace MandelBrotRenderer;

/************************************************************
 *  This class is responsible for a thread which
 *  primarily manages worker threads which actually execute
 *  the required set computations
 *
 *  Other Tasks performed here:
 *  Partial render results are currently gathered
 *  and processed by this thread for early visual
 *  feedback for the user
 *
 *  Checksums are computed based on the rendered
 *  pixel values
 *
 *  semaphores to synchronize the worker threads are
 *  managed by this thread
 *
 *  Much of the Qt slot/signal communication is
 *  also performed via this thread
 ************************************************************/


int RenderThread::count  = 0;

RenderThread::RenderThread(SettingsHandler&settingsHandler, QObject *parent)
    : QThread{ parent }, elapsedTimeLastRun(-1), passesDone(0), owner(nullptr),
      quitIsPending(false),
      finishTheLoop(false), currentPass(0),
      checksum(1),
      possiblePassValues{MAX_PASSES / 2, defaultNumPassesValue, MAX_PASSES - 1, MAX_PASSES},
      threadConfigurer(nullptr),
      numPassesConfigurer(nullptr),
      applicationSettingsHandler(settingsHandler),
      bufferedResults(MAX_NUM_WORKER_THREADS, MQuintVector(0)),
      bufferedAttributes(MAX_NUM_WORKER_THREADS, RegionAttributes()),
      centerX(MandelbrotWidget::unInitializedFloatString),  //TODO improve this, check for the lifetime of this static value
      centerY(MandelbrotWidget::unInitializedFloatString),
      scaleFactor(notYetInitializedDouble),
      abort(false), currentImage(nullptr), computationChunksDone(0), colorMapSize(MandelBrotRenderer::DefaultColormapSize),
      timerInSeconds(this),
      numWorkerThreads(calculateInitialNumThreads()),
      sem(nullptr),
      rendererData { numWorkerThreads, possiblePassValues[1], possiblePassValues[1], threadReallocationDefaultEnabled, colorMapSize,
                        internalDataType::unknownType, MandelBrotRenderer::notYetInitializedInt64},
      threadMediator(rendererData),
      displayer(nullptr)
{
    ++count;
    restart = false;
    abort = false;


    applicationSettingsHandler.registerSettingsUser(this);
}

void RenderThread::populateColorMap()
{
    colormap.resize(static_cast<std::size_t>(colorMapSize));
    uint count = 0;
    for (auto&  i : colormap)
        i = MandelBrotRenderer::rgbFromWaveLength(380.0 + ((count++) * 400.0 / colorMapSize));
}

void RenderThread::publishDynamicTasksEnabled()
{
    if (owner != nullptr) {
        this->owner->displayDynamicTasksInfo(dynamicThreadAllocationEnabled());
    }
    writeSettings();
}


void RenderThread::setOwnerOnce(MandelbrotWidget * owner)
{
    if (this->owner == nullptr) {
        this->owner = owner;
        connectUpOwner();
        owner->displayThreadsInfo(numWorkerThreads);
        owner->displayPassesInfo(rendererData.currentNumPassValue);
        publishDynamicTasksEnabled();
    }
}

void RenderThread::render(const MandelBrotRenderer::CoordValue& centerX, const MandelBrotRenderer::CoordValue& centerY,
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
                          const QString& preciseCenterX, const QString& preciseCenterY,
#endif
                          double scaleFactor,
                          QSize resultSize)
{
    QMutexLocker locker(&mutex);

    this->centerX = centerX;
    this->centerY = centerY;
    this->scaleFactor = scaleFactor;
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
    this->preciseCenterX = preciseCenterX;
    this->preciseCenterY = preciseCenterY;
#endif
    publishCoordinates();

    this->resultSize = resultSize;

    owner->displayDynamicTasksInfo(threadMediator.getEnabled());

    passesDone = 0;

    if (!isRunning()) {
        start(LowPriority);
    } else {
        restart = true;
        condition.wakeAll();
    }
}

internalDataType RenderThread::getInternalDataType() const
{
    return rendererData.numericType;
}

void RenderThread::startTimer()
{
    timerInSeconds.start(MS_IN_ONE_SEC / REQUESTED_TIMER_TICKS_PER_SECOND);
}

void RenderThread::stopTimer()
{
    timerInSeconds.stop();
}

void RenderThread::pauseTimer()
{
    int remainingTime = timerInSeconds.remainingTime();
    timerInSeconds.stop();
    timerInSeconds.setInterval(remainingTime);
}

void RenderThread::resumeTimer()
{
    timerInSeconds.start(MS_IN_ONE_SEC / REQUESTED_TIMER_TICKS_PER_SECOND);
}

void RenderThread::setNumberOfThreads(int value)
{
    if (value < MIN_NUM_WORKER_THREADS ||
        value > calculateInitialNumThreads())
    {
        throw std::out_of_range("Specified number of threads is unsupported");
    }

    rendererData.pendingNumWorkerThreads = value;
}

void RenderThread::setNumberOfPasses(int value)
{
    rendererData.nextNumPassValue = value;
}

void RenderThread::cleanup()
{
    QMutexLocker locker(&mutex);
    sem->release();
    QObject* signalSender = QObject::sender();
    if (signalSender != nullptr)
    {
        auto thread = dynamic_cast<RenderWorker* >(signalSender);
        if (thread != nullptr)
        {
            emit writeToLog("cleanup thread, p:" + QString::number(thread->getPassValue()));

            if (!thread->isCleanedUp())
            {
                signalSender->disconnect();
                //signalSender->deleteLater();
                thread->setCleanedUp();
            }
            else
            {
                emit writeToLog("cleanup already done!.... p:"
                                   + QString::number(thread->getPassValue()));
            }
        }
        else
        {
            emit writeToLog("Oops, cleaning up something strange!");
            Q_ASSERT(false);
        }

    }

    emit writeToLog("sem available: " + QString::number(sem->available()));
    if (sem->available() == numWorkerThreads)
    {
        std::cout << "All done" << std::endl;

        displayer->configureThreadInfo(numWorkerThreads);

        if (!quitIsPending) {
            emit renderedImage(currentImage, scaleFactor);
        }
        threadMediator.resetThreadMediator();
    }
}

void RenderThread::connectUpOwner()
{
    //use exceptions instead?
    Q_ASSERT(owner != nullptr);

    connect(this, SIGNAL(chunkDone(int)), owner, SLOT(progressMade(int)));

    connect(this, SIGNAL(renderStarting()), owner, SLOT(handleRenderStarting()));
    //TODO reduce the number of connections
    connect(this, SIGNAL(renderStarting()), owner, SLOT(resetElapsedTime()));
    connect(this, SIGNAL(renderStarting()), owner, SLOT(disableOptions()));
    connect(this, SIGNAL(renderStarting()), this, SLOT(startTimer()));

    connect(this, SIGNAL(allDone()), owner, SLOT(handleAllDone()));
    connect(this, SIGNAL(allDone()), this, SLOT(stopTimer()));

    connect(&timerInSeconds, SIGNAL(timeout()), owner, SLOT(incrementElapsedTime()));

    connect(this, SIGNAL(signalThreadState(uint,int)), displayer, SLOT(setThreadState(uint,int)));

    connect(this, SIGNAL(sendStatusMessage(QString,bool,int)), owner, SLOT(writeStatusMessage(QString,bool,int)));
    connect(this, SIGNAL(sendTransientStatusMessage(QString,bool)), owner, SLOT(writeTransientStatusMessage(QString,bool)));
}

void RenderThread::halt(const bool quitApplication)
{
    std::cout << "In " << static_cast<const char*>(__FUNCTION__) << std::endl;
    abort = true;

    quitIsPending = quitApplication;
}

#ifdef DEBUG_RAW_RESULTS
bool RenderThread::detectMismatches(MQuintVector results) const
{
    if (finalResults.size() > 2) {
        finalResults.clear();
    }

    finalResults.push_back(results);

    int firstFailurePoint = -1;
    MQuintVector mismatch0(0);
    MQuintVector mismatch1(0);
    int mismatches(0);
    equals = true;

    if (finalResults.size() == 2 &&
            finalResults[0].size() == finalResults[1].size())
    {
        for (int i = 0; i < finalResults[0].size(); ++i) {
            if (finalResults[0][i] != finalResults[1][i])
            {
                if (equals) {
                   firstFailurePoint = i;
                }
                equals = false;
                ++mismatches;
            }

            if (!equals)
            {
                mismatch0.push_back(finalResults[0][i]);
                mismatch1.push_back(finalResults[1][i]);
            }
        }
    }

    return equals;
}
#endif //DEBUG_RAW_RESULTS

void RenderThread::createChecksum(bool forcedToStop) const
{
    if (currentImage == nullptr)
    {
        emit writeToLog("no image for checksum!",
                           true);
    }
    checksum = 0;
    const uchar *st = currentImage->bits();
    int pixelCount = currentImage->width() * currentImage->height();

    std::vector<uchar> results;
    results.assign(st, st + pixelCount);

    for (const auto& i : results) {
        checksum += i;
    }

    auto elapsedTime = static_cast<double>(owner->getElapsedTimeDisplayed());

    bool dynamicAlgorithmActive = threadMediator.getEnabled();

    emit writeToLog("Threads: " + QString::number(numWorkerThreads) +
                       ", Passes: " + QString::number(rendererData.currentNumPassValue) +
                       ", PixelCount: " + QString::number(pixelCount) +
                       ", Checksum: " + QString::number(checksum) +
                       ", Dynamic Task Allocation: " + getBoolValueAsString(dynamicAlgorithmActive) +
                       ", Truncated: " + getBoolValueAsString(forcedToStop, boolDescriptionMode::true_false) +
                       ", ColourMapSize: " + QString::number(rendererData.colorMapSize) +
                       ", Internal Data Type: " + QString::number(toUnderlyingType(rendererData.numericType)) +
                       ", Iteration Sum: " + QString::number(rendererData.iterationSumCount) +
                       ", Time: " + QString::number(elapsedTime),
                       true);
#ifdef DEBUG_RAW_RESULTS
    detectMismatches(results);
#endif //DEBUG_RAW_RESULTS
}

void RenderThread::setThreadConfigurer(QAbstractSlider *configurer)
{
    if (threadConfigurer == nullptr) {
        threadConfigurer = configurer;
        connect(configurer, SIGNAL(valueChanged(int)), this, SLOT(setNumberOfThreads(int)));
    }
}

void RenderThread::setThreadInfoDisplayer(WindowThreadInfo *displayer)
{
    this->displayer = displayer;
    Q_ASSERT(threadConfigurer != nullptr);
    connect(threadConfigurer, SIGNAL(valueChanged(int)), displayer, SLOT(configureThreadInfo(int)));
}

void RenderThread::setThreadState(uint index, threadState state)
{
    emit signalThreadState(index, toUnderlyingType(state));
}

bool RenderThread::dynamicThreadAllocationEnabled()
{
    return (threadMediator.getEnabled());
}

void RenderThread::adjustWorkerThreadCount()
{
    if (numWorkerThreads != rendererData.pendingNumWorkerThreads) {
        numWorkerThreads = rendererData.pendingNumWorkerThreads;
        owner->displayThreadsInfo(numWorkerThreads);
        for (size_t i = 0; i < static_cast<size_t>(numWorkerThreads); ++i) {
            bufferedResults.at(i).clear();
        }

        delete(sem);
        sem = new QSemaphore(numWorkerThreads);

        emit numThreadsUpdate();
    }
}

int RenderThread::adjustNumPasses()
{
    const int NumPasses = rendererData.nextNumPassValue;

    if (rendererData.currentNumPassValue != NumPasses) {
        rendererData.currentNumPassValue = NumPasses;
    }

    owner->displayPassesInfo(rendererData.currentNumPassValue);
    emit numPassesUpdate();

    return NumPasses;
}

void RenderThread::releaseHelpers(std::vector <RenderWorker *>& helpers)
{
    for (auto i : helpers) {
        if (i != nullptr) {
            delete(i);
        }
    }
    helpers.clear();
}

bool RenderThread::getTypeIsSupported(const QString& typeDescription) const
{
    Q_ASSERT(!descriptionToTypeMap.empty());
    bool supported = false;

    auto findResult = descriptionToTypeMap.find(typeDescription);
    if (findResult != descriptionToTypeMap.end()) {
        supported = (*findResult).second.supported;
        Q_ASSERT(getTypeIsSupported((*findResult).second.type) == supported);
    }

    return supported;
}

//TODO look into reducing the code duplication here
QString RenderThread::getTypeDescription(internalDataType type) const
{
    Q_ASSERT(!typeToDescriptionMap.empty());

    QString description;
    auto findResult = typeToDescriptionMap.find(type);

    if (findResult != typeToDescriptionMap.end()) {
        description = (*findResult).second.typeString;
     }
    return description;
}

bool RenderThread::getTypeIsSupported(MandelBrotRenderer::internalDataType type) const
{
    Q_ASSERT(!typeToDescriptionMap.empty());

    bool supported = false;
    auto findResult = typeToDescriptionMap.find(type);

    if (findResult != typeToDescriptionMap.end()) {
        supported = (*findResult).second.supported;
     }
    return supported;
}

void RenderThread::AddNumericTypeToSelector(const QString &description, internalDataType dataType,
                                            typeNameUser& nameUser, bool enabled)
{
    supportedType newTypeValue {dataType, enabled};

    auto i = descriptionToTypeMap.insert(std::make_pair(description, newTypeValue));

    Q_ASSERT(i.second);

    supportedTypeString newDescriptionValue { description, enabled};
    auto j = typeToDescriptionMap.insert(std::make_pair(dataType, newDescriptionValue));
    Q_ASSERT(j.second);

    nameUser(description, enabled);
}

void RenderThread::initializeSupportedTypesTable(typeNameUser& nameUser)
{
    //TODO find a way to more safely link the enum value and combobox indices

#if (USE_BOOST_MULTIPRECISION == 1)
    constexpr bool boostTypesUsed = true;
    constexpr bool gccLikeTypesUsed = true;
#else
    constexpr bool boostTypesUsed = false;
#if defined(__GNUC__)
    constexpr bool gccLikeTypesUsed = true;
#else
    constexpr bool gccLikeTypesUsed = false;
#endif //__GNUC__
#endif //USE_BOOST_MULTIPRECISION

    AddNumericTypeToSelector("float (single precision)", internalDataType::singlePrecisionFloat, nameUser);
    AddNumericTypeToSelector("double (double precision)", internalDataType::doublePrecisionFloat, nameUser);
    AddNumericTypeToSelector("customized floating point type", internalDataType::customFloat20, nameUser, boostTypesUsed);
    AddNumericTypeToSelector("20 decimal digit precision float", internalDataType::float20dd, nameUser, boostTypesUsed);
    AddNumericTypeToSelector("30 decimal digit precision float", internalDataType::float30dd, nameUser, boostTypesUsed);
    AddNumericTypeToSelector("50 decimal digit precision float", internalDataType::float50dd, nameUser, boostTypesUsed);
    AddNumericTypeToSelector("80 bit floating type", internalDataType::float80, nameUser, gccLikeTypesUsed);
    AddNumericTypeToSelector("128 bit floating type", internalDataType::float128, nameUser, gccLikeTypesUsed);
    AddNumericTypeToSelector("32 bit integer", internalDataType::int32, nameUser);
    AddNumericTypeToSelector("64 bit integer", internalDataType::int64, nameUser);
    AddNumericTypeToSelector("128 bit integer", internalDataType::int128, nameUser, gccLikeTypesUsed);
}

void RenderThread::setColormapSize(int value)
{
    //TODO: throw exception if required memory is lacking?
    colorMapSize = value;
    //TODO remove the duplication
    rendererData.colorMapSize = colorMapSize;
    populateColorMap();
}

QString RenderThread::getDataTypeName()
{
    QString result;
    //TODO change this if the number of types starts to grow
    for (const auto& i : descriptionToTypeMap) {

        if (i.second.type == rendererData.numericType) {
            result = i.first;
            break;
        }
    }
    return result;
}

void RenderThread::setInternalDataType(const QString& description)
{
    internalDataType result = internalDataType::doublePrecisionFloat;

    auto findResult = descriptionToTypeMap.find(description);

    Q_ASSERT(findResult != descriptionToTypeMap.end());
    bool typeIsSupported = (*findResult).second.supported;
    Q_ASSERT(typeIsSupported);

    if(findResult != descriptionToTypeMap.end() && typeIsSupported){
        result = (*findResult).second.type;
        owner->displayInternalDataType(description);
    }

    rendererData.numericType = result;
}

const MandelBrotRenderer::RendererData& RenderThread::getRendererData() const
{
    return rendererData;
}

int RenderThread::calculateInitialNumThreads()
{
    return (std::min<int>(MAX_NUM_WORKER_THREADS,
             MIN_NUM_UNUSED_THREADS < QThread::idealThreadCount() ? QThread::idealThreadCount() - MIN_NUM_UNUSED_THREADS : 1 ));
}

void RenderThread::writeSettings()
{
    applicationSettingsHandler.getSettings().beginGroup("Renderer");

    applicationSettingsHandler.getSettings().setValue("numWorkerThreads", rendererData.pendingNumWorkerThreads);
    applicationSettingsHandler.getSettings().setValue("currentNumPassValue", rendererData.currentNumPassValue);
    applicationSettingsHandler.getSettings().setValue("nextNumPassValue", rendererData.nextNumPassValue);
    applicationSettingsHandler.getSettings().setValue("threadMediatorEnabled", threadMediator.getEnabled());
    applicationSettingsHandler.getSettings().setValue("colourMapSize", rendererData.colorMapSize);
    applicationSettingsHandler.getSettings().setValue("internalNumericType", toUnderlyingType(rendererData.numericType));
    applicationSettingsHandler.getSettings().endGroup();
    applicationSettingsHandler.getSettings().sync();
}

void RenderThread::processSettingUpdate(QSettings &settings)
{
    settings.beginGroup("Renderer");

    numWorkerThreads = settings.value("numWorkerThreads", calculateInitialNumThreads()).toInt();
    rendererData.pendingNumWorkerThreads = numWorkerThreads;
    rendererData.currentNumPassValue = settings.value("currentNumPassValue", possiblePassValues[1]).toInt();
    rendererData.nextNumPassValue = rendererData.currentNumPassValue;
    rendererData.colorMapSize = settings.value("colourMapSize", MandelBrotRenderer::DefaultColormapSize).toInt();
    rendererData.numericType = static_cast<internalDataType>(settings.value("internalNumericType", toUnderlyingType(internalDataType::doublePrecisionFloat)).toInt());

    threadMediator.setEnabled(settings.value("threadMediatorEnabled", threadReallocationDefaultEnabled).toBool());

    settings.endGroup();
}

void RenderThread::InitializeDynamicValuesInGUI()
{
    owner->displayInternalDataType(getDataTypeName());
    owner->setIterationSumCount(rendererData.iterationSumCount);
}

void RenderThread::prepareForNewTasks()
{
    adjustWorkerThreadCount();
    displayer->configureThreadInfo(numWorkerThreads, threadState::starting);
    threadMediator.resetThreadMediator();
    populateColorMap();
    rendererData.iterationSumCount = 0;
}

void RenderThread::run()
{
    bool endThisRun = false;
    owner->displayDynamicTasksInfo(dynamicThreadAllocationEnabled());

    std::vector <RenderWorker *> helpers(static_cast<std::size_t>(numWorkerThreads));

    InitializeDynamicValuesInGUI();

    sem = new QSemaphore(numWorkerThreads);
    while (!endThisRun) {
        mutex.lock();

        timer.start();
        elapsedTimeLastRun = 0;

        QSize resultSize = this->resultSize;
        double scaleFactor = this->scaleFactor;
        //TODO - fix this shadowing
        MandelBrotRenderer::CoordValue centerX = this->centerX;
        MandelBrotRenderer::CoordValue centerY = this->centerY;
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
        QString preciseCenterX = this->preciseCenterX;
        QString preciseCenterY = this->preciseCenterY;
#endif
        mutex.unlock();

        auto halfWidth = static_cast<double>(resultSize.width()) / 2.0;

        auto halfHeight = static_cast<double>(resultSize.height()) / 2.0;
        double fullHeight = resultSize.height();
        QImage image(resultSize, QImage::Format_RGB32);
        currentImage = &image;

        int pass = 0;
        if (!quitIsPending) {
            //quit flow skips signalling to dependent GUI elements in order to simplify its appearance
            emit(sendStatusMessage("Rendering in progress"));
            emit(renderStarting());

            abort = false;

            std::cout << "Dynamic flow enabled state: " << threadMediator.getEnabled() << std::endl;
        }
        emit chunkDone(0);
        passesDone = 0;

        computationChunksDone = 0;

        sem->acquire(numWorkerThreads);

        mutex.lock();
        releaseHelpers(helpers);

        sem->release(numWorkerThreads);
        prepareForNewTasks();

        const int NumPasses = adjustNumPasses();

        const double roundOffCorrection =  0.25;
        const double heightStep = (fullHeight + roundOffCorrection)/ numWorkerThreads;
        mutex.unlock();

        clearBuffers();

        while (pass < NumPasses) {
            bool allBlack = true;

            //wait for all threads ready
            sem->acquire(numWorkerThreads);

            mutex.lock();

            releaseHelpers(helpers);

            threadMediator.resetBusyThreadCount();

            if (quitIsPending) {
                pass = NumPasses;
                sem->release(numWorkerThreads);
                mutex.unlock();
                endThisRun = true;
                break;
            }

            threadMediator.resetThreadMediator();

            passesDone = pass;

            emit renderedImage(currentImage, scaleFactor);

            rendererData.iterationSumCount = 0;

            double currentHeight = -halfHeight;

            std::cout << "**** " << "pass: " << pass << " ****" << std::endl;

            for (int i = 0; i < numWorkerThreads; ++i)
            {
                /* launch worker tasks in new threads to start computing immediately */
                helpers.emplace_back
                        (new RenderWorker(this,
                                                owner,
                                                 static_cast<uint>(pass),
                                                 static_cast<uint>(NumPasses),
                                                 colormap,
                                                 restart,
                                                 abort,
                                                 i,
                                                ComputedDataSegment( RegionAttributes(scaleFactor,
                                                                                      centerX, centerY,
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
                                                                                      preciseCenterX,
                                                                                      preciseCenterY,
#endif
                                                                                      static_cast<int>(-halfWidth),
                                                                                      static_cast<int>(halfWidth),
                                                                                      static_cast<int>(currentHeight),
                                                                                      static_cast<int>(currentHeight + heightStep),
                                                                                      static_cast<int>(fullHeight)),
                                                                     &image,
                                                                    i,
                                                                    bufferedResults[static_cast<size_t>(i)],
                                                                    bufferedAttributes[static_cast<size_t>(i)])
                                                ,
                                                 haltChecker([&]{
                                                        return abort;
                                                        }),
                                                    *owner->getMutex(static_cast<std::size_t>(i))
                                                 )
                        );
                currentHeight += heightStep;
            }

            mutex.unlock();

            if (allBlack && pass == 0) {
                pass = NumPasses / 2;

                computationChunksDone = (pass  - 1) * numWorkerThreads;
                emit chunkDone(computationChunksDone);

            } else {

                if (!restart)
                {
                    elapsedTimeLastRun = timer.elapsed();
                    passesDone = pass;
                }
                ++pass;
            }
        }

        bool forcedToStop = false;

        if (pass >= NumPasses)
        {
            sem->acquire(numWorkerThreads);
            int busyThreads = threadMediator.getBusyThreadCount();
            Q_ASSERT(busyThreads == 0);
            emit writeToLog("emitting allDone, pass: " + QString::number(pass));

            emit chunkDone(numWorkerThreads * NumPasses);
            msleep(PROGRESS_BAR_WAIT_IN_MS);

            owner->setIterationSumCount(rendererData.iterationSumCount);

            sem->release(numWorkerThreads);

            std::cout << " <<<<<<<<<<<<<<<<<< ALL DONE >>>>>>>> " << "available: " << sem->available() << std::endl;

            emit(allDone());
            forcedToStop = abort;
            abort = false;
        }

        releaseHelpers(helpers);
        createChecksum(forcedToStop);
        emit(sendTransientStatusMessage("Rendering completed"));
        owner->getInfoDisplayer()->setRenderState(
            forcedToStop ? InformationDisplay::renderState::aborted : InformationDisplay::renderState::idle);

        if (!quitIsPending) {
            mutex.lock();
            if (!restart && !abort) {
                condition.wait(&mutex);
            }
            restart = false;
            mutex.unlock();
        }
        else
        {
            sem->acquire(numWorkerThreads);
            int busyThreads = threadMediator.getBusyThreadCount();
            Q_ASSERT(busyThreads == 0);
            sem->release(numWorkerThreads);
        }
    }
    std::cout << " master thread run done!" << std::endl;
}

void RenderThread::processIntegerValueFromButtonPress(int value)
{
    setNumberOfPasses(value);
}

void RenderThread::publishCoordinates() const
{
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
    owner->updateCoordInfo(preciseCenterX, preciseCenterY, scaleFactor);
#else
    owner->updateCoordInfo(centerX, centerY, scaleFactor);
#endif
}

 void RenderThread::drawPreComputedData(ComputedDataSegment& data)
{
    QMutexLocker locker(&mutex2);
    int i = 0;

    if(data.hasBeenUsed())
    {
        return;
    }

    MQuintVector rawResultData = data.extractRawResultData();

    if (rawResultData.empty())
    {
        //TODO add a flag to check this is as expected
        return;
    }

    rendererData.iterationSumCount += data.getFullResultData().iterationSum;
    owner->setIterationSumCount(rendererData.iterationSumCount);

    QImage *image = data.getImage();

    if (image == nullptr || image->isNull() || image->format() == QImage::Format_Invalid)
    {
       emit writeToLog("NULL or invalid image referenced in segment data!");
       return;
    }

    const int segmentIndex = data.getSegmentIndex();

    emit writeToLog("drawing data for segment: " + QString::number(segmentIndex), false);

    const int minX = data.getMinX();
    const int maxX = data.getMaxX();
    const int minY = data.getMinY();
    const int maxY = data.getMaxY();
    const int fullHeight = data.getFullHeight();

    for (int y = minY; y < maxY; ++y)
    {
        auto scanLine =
                reinterpret_cast<uint *>(image->scanLine(y + ((fullHeight)/ 2)));

        for (int x = minX; x < maxX; ++x)
        {

            *scanLine++ = rawResultData[i++];
            //*scanLine++ = rawResultData.at(i++);
        }
    }

    bufferedResults[static_cast<size_t>(segmentIndex)].clear();
    bufferedResults[static_cast<size_t>(segmentIndex)].append(rawResultData);

    bufferedAttributes[static_cast<size_t>(segmentIndex)] = data.getAttributes();

    rawResultData.clear();

    QObject* signalSender = QObject::sender();
    auto thread = dynamic_cast<RenderWorker* >(signalSender);
    if (thread != nullptr)
    {
        WorkerThreadData&& threadData = thread->transferWorkerThreadData();
        emit writeToLog("acquired data, pass: " + QString::number(threadData.getPassData()));
        threadData.getValues().push_back(minX);
    }
 }

 void RenderThread::markThreadProgressComplete()
 {
     emit chunkDone(++computationChunksDone);
 }


 void RenderThread::quitApplication()
 {
    halt(true);
 }

 void RenderThread::haltComputations()
 {
     halt(false);
 }

 void RenderThread::clearBuffers()
 {
     bufferedAttributes.clear();

     for (int i = 0; i < MAX_NUM_WORKER_THREADS; ++i)
     {
        bufferedAttributes.emplace_back(RegionAttributes());
     }
 }
