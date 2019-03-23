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

#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QElapsedTimer>

#include <QTimer>
#include <QSemaphore>

#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>

#include <memory>
#include <map>
#include <array>
#include <utility>

QT_BEGIN_NAMESPACE
class QImage;
QT_END_NAMESPACE

#include "computeddatasegment.h"
#include "informationdisplay.h"
#include "mandelbrotrenderer.h"
#include "regionattributes.h"
#include "renderthreadmediator.h"
#include "settingsuser.h"
#include "buttonuser.h"

//#define DEBUG_RAW_RESULTS

class MandelbrotWidget;
class RenderWorker;
class QAbstractSlider;
class QButtonGroup;
class WindowThreadInfo;
class SettingsHandler;

struct RendererData
{
    int pendingNumWorkerThreads;
    int currentNumPassValue;
    int nextNumPassValue;
    bool threadMediatorEnabled;
};

//! [0]
class RenderThread : public QThread, public SettingsUser, public ButtonUser
{
    Q_OBJECT

public:
    explicit RenderThread(SettingsHandler& settingsHandler, QObject *parent = nullptr);
    virtual ~RenderThread() override { --count; wait(); }

    void render(const MandelBrotRenderer::CoordValue& centerX, const MandelBrotRenderer::CoordValue& centerY,
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
              QString preciseCenterX, QString preciseCenterY,
#endif
                double scaleFactor, QSize resultSize);

    qint64      getElapsedTimeLastRun() const { return elapsedTimeLastRun; }
    int         getPassesDone() const { return passesDone; }
    int         getNumWorkerThreads() const { return numWorkerThreads; }
    MandelBrotRenderer::internalDataType getInternalDataType() const;

    const MandelBrotRenderer::MQintVector& getPossiblePassValues() const { return possiblePassValues; }

    int  getRunningNumPasses() const { return rendererData.currentNumPassValue; }
    void setOwnerOnce(MandelbrotWidget * owner);
    void setThreadConfigurer(QAbstractSlider *configurer);
    void setThreadInfoDisplayer(WindowThreadInfo* displayer);
    void setThreadState(uint index, MandelBrotRenderer::threadState state);
    bool dynamicThreadAllocationEnabled();

    RenderThreadMediator& getThreadMediator() { return threadMediator; }

    SettingsHandler& getApplicationSettings() const { return applicationSettingsHandler; }

    virtual void processIntegerValueFromButtonPress(int value) override;

    static int calculateInitialNumThreads();

    const MandelBrotRenderer::RendererData& getRendererData() const;

    using typeNameUser = std::function<void (const QString&, bool) >;

public slots:
    void cleanup();
    void drawPreComputedData(ComputedDataSegment& data);
    void markThreadProgressComplete();
    void quitApplication();
    void haltComputations();
    void clearBuffers();
    void setNumberOfPasses(int value);
    void publishDynamicTasksEnabled();
    void writeSettings();
    void pauseTimer();
    void stopTimer();
    void resumeTimer();
    void setColormapSize(int value);
    void setInternalDataType(const QString& description);
    void initializeSupportedTypesTable(typeNameUser& nameUser);

    void processSettingUpdate(QSettings& settings) override;


signals:
    void renderedImage(const QImage *image, double scaleFactor);
    void signalThreadState(uint index, int stateAsInt);

    void stopThreads(bool);

    void sendRestart(bool newRestartValue);
    void sendAbort(bool newAbortValue);
    void segmentDone(int passesDone);
    void renderStarting();
    void chunkDone(int passesDone);
    void allDone();
    void numThreadsUpdate();
    void numPassesUpdate();
    void sendStatusMessage(const QString& message, bool isWarning = false, int timeout = 0);
    void sendTransientStatusMessage(const QString& message, bool isWarning = false);

    void writeToLog(const QString& outputString, bool show = false) const;

private slots:
    void startTimer();
    void setNumberOfThreads(int value);
private:
    void run() override;
    void connectUpOwner();
    void halt(const bool quitApplication);
    void createChecksum(bool forcedToStop) const;
    void adjustWorkerThreadCount();
    int adjustNumPasses();
    void releaseHelpers(std::vector<RenderWorker *>& helpers);

    void AddNumericTypeToSelector(const QString& description, MandelBrotRenderer::internalDataType dataType,
                                  typeNameUser& nameUser, bool enabled = true);
    QString getDataTypeName();

    qint64 elapsedTimeLastRun;
    QElapsedTimer timer;
    int passesDone;
    MandelbrotWidget* owner;
    std::atomic<bool> quitIsPending;
    bool finishTheLoop;
    int currentPass;
    mutable qint64 checksum;
    const MandelBrotRenderer::MQintVector possiblePassValues;

    MandelBrotRenderer::TypePrecisonMap precisionRangeInfo;

    QAbstractSlider* threadConfigurer;
    QButtonGroup* numPassesConfigurer;
    SettingsHandler& applicationSettingsHandler;

    std::vector<MandelBrotRenderer::MQuintVector> bufferedResults;
    std::vector<RegionAttributes> bufferedAttributes;

    struct supportedType
    {
        MandelBrotRenderer::internalDataType type;
        bool supported;
    };

    std::map<QString, supportedType> typeToDescriptionMap;

#ifdef DEBUG_RAW_RESULTS
    bool detectMismatches(MQuintVector results) const;
    mutable std::vector<MQuintVector> finalResults;
    mutable bool equals;
#endif //DEBUG_RAW_RESULTS

    QMutex mutex;
    QMutex mutex2;
    QWaitCondition condition;
    MandelBrotRenderer::CoordValue centerX;
    MandelBrotRenderer::CoordValue centerY;
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
    QString preciseCenterX;
    QString preciseCenterY;
#endif
    double scaleFactor;

    QSize resultSize;
    bool restart;
    bool abort;
    const QImage * currentImage;
    int computationChunksDone;
    int colorMapSize;

    QTimer timerInSeconds;

    int numWorkerThreads;
    QSemaphore* sem;

    MandelBrotRenderer::RendererData rendererData;
    RenderThreadMediator threadMediator;

    WindowThreadInfo* displayer;
    MandelBrotRenderer::colorMapStore colormap {};

    static constexpr int PROGRESS_BAR_WAIT_IN_MS = 200;
    static constexpr int REQUESTED_TIMER_TICKS_PER_SECOND = InformationDisplay::getRequiredTimerTicksPerSecond();
    static constexpr int MS_IN_ONE_SEC = 1000;
    static constexpr bool threadReallocationDefaultEnabled = true;

    static int count;
    void populateColorMap();
    void InitializeDynamicValuesInGUI();
    void prepareForNewTasks();
};
//! [0]

#endif // RENDERTHREAD_H
