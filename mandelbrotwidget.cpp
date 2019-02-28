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

#include <QPainter>
#include <QKeyEvent>
#include <QScrollBar>
#include <QApplication>
#include <QThread>
#include <QRubberBand>
#include <QMenuBar>
#include <QMessageBox>

#include <cmath>

#include <iostream>

#include "mandelbrotwidget.h"
#include "informationdisplay.h"
#include "toolsmenu.h"
#include "EditMenu.h"
#include "filemenu.h"
#include "RenderMenu.h"
#include "windowmenu.h"
#include "toolsoptionswidget.h"
#include "settingshandler.h"


//! [0]
const double DefaultCenterX = -0.637011;
const double DefaultCenterY = -0.0395159;
const double DefaultScale = 0.00403897;

const double ZoomInFactor = 0.8;
const double ZoomOutFactor = 1 / ZoomInFactor;
const int ScrollStep = 20;
//! [0]

//! [1]


MandelbrotWidget::MandelbrotWidget(QWidget *parent)
    : QMainWindow(parent), settingsHandler(this), thread(settingsHandler, this),
      centerX(notYetInitializedDouble), centerY(notYetInitializedDouble), pixmapScale(notYetInitializedDouble),
      curScale(notYetInitializedDouble), perPixelCoeff(notYetInitializedDouble),
      scaleHasChanged(false), renderInProgress(false), usingUndoRedo(false), numericType(internalDataType::doublePrecisionFloat),
      pauseMutexes(MAX_NUM_WORKER_THREADS, nullptr), paused(),
      historyLog(std::make_unique<RenderHistory>(this)), unsavedChangesExist(false)
{
    thread.processSettingUpdate(settingsHandler.getSettings());
    infoDisplayer = new InformationDisplay(settingsHandler);
    toolsMenu = new ToolsMenu(this->menuBar(), this, &thread, settingsHandler);
    editMenu = new EditMenu(this);
    fileMenu = new FileMenu(this->menuBar(), this, settingsHandler);
    renderMenu = new RenderMenu(this);
    windowMenu = new WindowMenu(&thread);
    centralPlotArea = new QWidget();
    messageBar = new QStatusBar(parent);
    newSelectionArea = new QRubberBand(QRubberBand::Rectangle, this);
    mandelbrotWidgetLayout =  new QVBoxLayout();
    progressBar =  new QProgressBar();
    logger = new QTextEdit();
    miscLogger = new QTextEdit();

    setStatusBar(messageBar);
    this->menuBar()->setStyleSheet("QMenuBar {background: lightgray; }");

    this->menuBar()->setLayout(new QHBoxLayout());

    settingsHandler.registerSettingsUser(this);

    thread.setOwnerOnce(this);

    connect(&thread, SIGNAL(renderedImage(const QImage*,double)), this, SLOT(updatePixmap(const QImage*,double)));

    connect(this, SIGNAL(quitAll()), &thread, SLOT(quitApplication()));
    //connect(this, SIGNAL(quitAll()), &thread, SLOT(clearBuffers()));
    connect(this, SIGNAL(halt()), &thread, SLOT(haltComputations()));
    //connect(this, SIGNAL(halt()), &thread, SLOT(clearBuffers()));

    connect(&thread, SIGNAL(numThreadsUpdate()), this, SLOT(resizeProgressBar()));
    connect(&thread, SIGNAL(numPassesUpdate()), this, SLOT(resizeProgressBar()));

    connect(this, SIGNAL(halt()), infoDisplayer, SLOT(stopProgress()));
    connect(this, SIGNAL(halt()), &thread, SLOT(stopTimer()));

    connect (&thread, SIGNAL(finished()), this, SLOT(accomplishQuit()));

    connect(&thread, SIGNAL(allDone()), this, SLOT(enableOptions()));

    connect(&thread, SIGNAL(numPassesUpdate()), this, SLOT(resizeProgressBar()));

    connect(&thread, SIGNAL(writeToLog(QString,bool)), this, SLOT(outputToLog(QString,bool)));

    connect(&thread, SIGNAL(renderStarting()), this, SLOT(disableFileMenu()));
    connect(&thread, SIGNAL(allDone()), this, SLOT(enableFileMenu()));

    QApplication::setQuitOnLastWindowClosed(true);

    setWindowTitle(tr("Mandelbrot"));

    infoDisplayer->prepareTimerInfo();

    prepareProgressBar();

    menuBar()->addMenu(fileMenu);

    menuBar()->addMenu(editMenu);

    menuBar()->addMenu(toolsMenu);

    menuBar()->addMenu(renderMenu);

    menuBar()->addMenu(windowMenu);

    menuBar()->addSeparator();

        for (auto& i : pauseMutexes) {
        i = new QMutex();
    }

    QAction* playAction = menuBar()->addAction("Play");
    QIcon playIcon(":/play.png");
    playAction->setIcon(playIcon);
    connect(playAction, SIGNAL(triggered()), this, SLOT(initiateRefresh()));

    QAction* stopAction = menuBar()->addAction("Stop");
    QIcon stopIcon(":/stop.png");
    stopAction->setIcon(stopIcon);
    connect(stopAction, SIGNAL(triggered()), this, SLOT(initiateHalt()));

    QAction* pauseAction = menuBar()->addAction("Pause");
    QIcon pauseIcon(":/pause.png");
    pauseAction->setIcon(pauseIcon);
    connect(pauseAction, SIGNAL(triggered()), this, SLOT(initiatePause()));


    menuBar()->show();

    this->setCentralWidget(centralPlotArea);
    this->centralWidget()->setLayout(mandelbrotWidgetLayout);

    mandelbrotWidgetLayout->addWidget(infoDisplayer);

    mandelbrotWidgetLayout->addWidget(progressBar);

    centralPlotArea->show();

    qRegisterMetaType<ComputedDataSegment>("ComputedDataSegment&");

    outputToLog("Log Output");
    outputToLog("->");
    logger->setWindowFlags(Qt::Window | Qt::WindowTitleHint |
                           Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::CustomizeWindowHint);
    logger->show();
    //miscLogger.show();

#ifndef QT_NO_CURSOR
    setCursor(Qt::CrossCursor);
#endif
    processWindowSize();
    writeSettings();
}

void MandelbrotWidget::processWindowSize()
{
    QSize currentSize = size();
    infoDisplayer->updatePixmapSizeInfo(currentSize);
    const int pixelCount = currentSize.width() * currentSize.height();
    perPixelCoeff = (pixelCount != 0) ? 1.0 / static_cast<double>(pixelCount) : notYetInitializedDouble;
}

void MandelbrotWidget::adjustProgressBar()
{
    progressBar->reset();
    progressMade(0);
    progressBar->setMaximum(thread.getRunningNumPasses() * thread.getNumWorkerThreads());
}

void MandelbrotWidget::writeSettings()
{
    settingsHandler.getSettings().beginGroup("MainWindow");
        settingsHandler.getSettings().setValue("size", size());
        settingsHandler.getSettings().setValue("pos", pos());
    settingsHandler.getSettings().endGroup();

    settingsHandler.getSettings().beginGroup("RenderParameters");
        settingsHandler.getSettings().setValue("centerX", centerX);
        settingsHandler.getSettings().setValue("centerY", centerY);
        settingsHandler.getSettings().setValue("curScale", curScale);
        settingsHandler.getSettings().setValue("pixmapScale", pixmapScale);
    settingsHandler.getSettings().endGroup();

    //TODO automate this using the SettingsUser interface?
    thread.writeSettings();
    infoDisplayer->writeSettings();
    fileMenu->writeSettings();
}

void MandelbrotWidget::closeEvent(QCloseEvent *event)
{
    writeSettings();
    settingsHandler.reset();
    event->accept();
}

InformationDisplay *MandelbrotWidget::getInfoDisplayer() const
{
    return infoDisplayer;
}

RendererConfig MandelbrotWidget::generateConfigData()
{
    auto result = RendererConfig(getInfoDisplayer()->getDetailedDisplayEnabled(),
                          size(),
                          pos(),
                          centerX,
                          centerY,
                          curScale,
                          pixmapScale,
                          thread.getRendererData().threadMediatorEnabled,
                          thread.getRendererData().colorMapSize,
                          thread.getRendererData().numericType);

    return (result);
}

void MandelbrotWidget::enforceConfigData(RendererConfig &newConfigData)
{
    RendererConfig originalConfig = generateConfigData();
    getInfoDisplayer()->setEnabled(newConfigData.getDetailedDisplayEnabled());
    centerX = newConfigData.getCenterX();
    centerY = newConfigData.getCenterY();
    curScale = newConfigData.getCurScale();
    pixmapScale = newConfigData.getPixmapScale();
    thread.getThreadMediator().setEnabled(newConfigData.getThreadMediatorEnabled());

    move(newConfigData.getPos());
    resize(newConfigData.getSize());

    if (originalConfig != newConfigData) {
        initiateRefresh();
    }
}
void MandelbrotWidget::prepareProgressBar()
{
    progressBar->setMinimum(0);
    adjustProgressBar();
    progressBar->setStyleSheet("QProgressBar {color : darkgray; }");
    hideProgress();

}

MandelBrotRenderer::internalDataType MandelbrotWidget::getNumericType() const
{
    return numericType;
}

bool MandelbrotWidget::confirmOperation(const QString& message, const QString& informative, bool suggestSave)
{
    QMessageBox msgBox;
    msgBox.setText(message);

    if (suggestSave)
    {
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
        msgBox.setDefaultButton(QMessageBox::Discard);
    }
    else
    {
        msgBox.setStandardButtons(QMessageBox::Ok);
    }
    msgBox.addButton(QMessageBox::Cancel);

    msgBox.setInformativeText(informative);

    int ret = msgBox.exec();

    if (ret == QMessageBox::Save)
    {
        fileMenu->saveParamsAs();
    }

    return (ret != QMessageBox::Cancel);
}

bool MandelbrotWidget::getUnsavedChangesExist() const
{
    return unsavedChangesExist;
}

void MandelbrotWidget::setUnsavedChangesExist(bool value)
{
    unsavedChangesExist = value;
}

void MandelbrotWidget::showProgress()
{
    progressBar->setVisible(true); outputToLog("Showing Progress!");
}
void MandelbrotWidget::hideProgress()
{
    progressBar->setVisible(false); outputToLog("Hiding Progress!");
    //QApplication::beep();
}

void MandelbrotWidget::handleRenderStarting()
{
    renderInProgress = true;
    showProgress();
}

void MandelbrotWidget::handleAllDone()
{
    hideProgress();
    paused.store(false);
    renderInProgress = false;
}

void MandelbrotWidget::resetElapsedTime()
{
    infoDisplayer->resetElapsedTimeInfo();
    infoDisplayer->startProgress();
    //QApplication::beep();
}
void MandelbrotWidget::incrementElapsedTime()
{
    infoDisplayer->incrementTimeInfo();
}

void MandelbrotWidget::resizeProgressBar()
{
    adjustProgressBar();
}

void MandelbrotWidget::restoreDefaults()
{
    settingsHandler.getSettings().remove("MainWindow");
    settingsHandler.getSettings().remove("RenderParameters");
    settingsHandler.getSettings().remove("WindowThreadInfo");
    settingsHandler.getSettings().remove("Renderer");
    settingsHandler.getSettings().remove("InformationDisplay");

    settingsHandler.notifyUsersSettingsUpdatedByOwner(this);
    toolsMenu->refreshOptionsGUI();

    update();
    if (QSize(kInitialWidth, kInitialHeight) == size()) {
        executeRender();
    }
}

void MandelbrotWidget::disableFileMenu()
{
    fileMenu->setEnabled(false);
}

void MandelbrotWidget::enableFileMenu()
{
    fileMenu->setEnabled(true);
}

void MandelbrotWidget::writeStatusMessage(const QString& message, bool isWarning, int timeout)
{
    const QString colour = isWarning ? "red" : "black";
    statusBar()->setStyleSheet(colour);
    statusBar()->showMessage(message, timeout);
}

void MandelbrotWidget::writeTransientStatusMessage(const QString& message, bool isWarning)
{
    writeStatusMessage(message, isWarning, TRANSIENT_MESSAGE_DURATION_IN_MS);
}

void MandelbrotWidget::pause()
{
    int locked = 0;
    if (paused.load())
    {
        return;
    }
    QMutexLocker locker(&mutex);
    paused.store(true);

    for (auto& i : pauseMutexes)
    {
        i->lock();
        ++locked;
    }
    thread.pauseTimer();
}

void MandelbrotWidget::release(bool forceFlow)
{
    if (!forceFlow && !(paused.load()))
    {
        return;
    }

    QMutexLocker locker(&mutex);
    paused.store(false);

    for (auto& i : pauseMutexes) {
        i->try_lock();
        i->unlock();
    }
    thread.resumeTimer();
    
}

void MandelbrotWidget::undo()
{
    historyLog->undo();
}

void MandelbrotWidget::redo()
{
    historyLog->redo();
}

void MandelbrotWidget::setUndoEnabled(bool enabled)
{
    editMenu->setUndoEnabled(enabled);
}

void MandelbrotWidget::setRedoEnabled(bool enabled)
{
    editMenu->setRedoEnabled(enabled);
}

void MandelbrotWidget::progressMade(int computationChunksDone)
{
    outputToLog("<<< ProgressMade" + QString::number(computationChunksDone) + ">>>");
    progressBar->setValue(computationChunksDone);
    infoDisplayer->updateProgress(static_cast<uint>((100 * computationChunksDone) / progressBar->maximum()));
}
//! [1]

//! [2]
void MandelbrotWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    if (pixmap.isNull()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, tr("Rendering initial image, please wait..."));
//! [2] //! [3]
        return;
//! [3] //! [4]
    }
//! [4]

//! [5]
//    if (curScale == pixmapScale) {
    if (scaleHasChanged) {
//! [5] //! [6]
        painter.drawPixmap(pixmapOffset, pixmap);
//! [6] //! [7]
    } else {
//! [7] //! [8]
        double scaleFactor = pixmapScale / curScale;
        auto newWidth = int(pixmap.width() * scaleFactor);
        auto newHeight = int(pixmap.height() * scaleFactor);
        auto newX = pixmapOffset.x() + (pixmap.width() - newWidth) / 2;
        auto newY = pixmapOffset.y() + (pixmap.height() - newHeight) / 2;

        painter.save();
        painter.translate(newX, newY);
        painter.scale(scaleFactor, scaleFactor);
        QRectF exposed = painter.matrix().inverted().mapRect(rect()).adjusted(-1, -1, 1, 1);
        painter.drawPixmap(exposed, pixmap, exposed);
        painter.restore();
    }
    scaleHasChanged = false;
//! [8] //! [9]

    QString text = tr("Use mouse wheel or the '+' and '-' keys to zoom. "
                      "Press and hold left mouse button to scroll.");
    QFontMetrics metrics = painter.fontMetrics();
    int textWidth = metrics.width(text);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 127));
    painter.drawRect((width() - textWidth) / 2 - 5, 0, textWidth + 10, metrics.lineSpacing() + 5);
    painter.setPen(Qt::white);
    painter.drawText((width() - textWidth) / 2, metrics.leading() + metrics.ascent(), text);
}
//! [9]

//! [10]
void MandelbrotWidget::resizeEvent(QResizeEvent * /* event */)
{
    processWindowSize();
    executeRender();
}
//! [10]

//! [11]
void MandelbrotWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        zoom(ZoomInFactor);
        break;
    case Qt::Key_Minus:
        zoom(ZoomOutFactor);
        break;
    case Qt::Key_Left:
        scroll(-ScrollStep, 0);
        break;
    case Qt::Key_Right:
        scroll(+ScrollStep, 0);
        break;
    case Qt::Key_Down:
        scroll(0, -ScrollStep);
        break;
    case Qt::Key_Up:
        scroll(0, +ScrollStep);
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}
//! [11]

#if QT_CONFIG(wheelevent)
//! [12]
void MandelbrotWidget::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    double numSteps = numDegrees / 15.0;
    zoom(pow(ZoomInFactor, numSteps));
}
//! [12]
#endif

//! [13]
void MandelbrotWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        lastDragPos = event->pos();
        return;
    }

    if (event->button() == Qt::RightButton) {
        newSelectionOrigin = event->pos();
        newSelectionArea->setGeometry(QRect(newSelectionOrigin, QSize()));
        newSelectionArea->show();
    }
}
//! [13]

//! [14]
void MandelbrotWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        pixmapOffset += event->pos() - lastDragPos;
        lastDragPos = event->pos();
        updateView();
        return;
    }
    if(event->buttons() & Qt::RightButton) {
        newSelectionArea->setGeometry(QRect(newSelectionOrigin, event->pos()).normalized());
    }
}
//! [14]

//! [15]
void MandelbrotWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        pixmapOffset += event->pos() - lastDragPos;
        lastDragPos = QPoint();

        int deltaX = (width() - pixmap.width()) / 2 - pixmapOffset.x();
        int deltaY = (height() - pixmap.height()) / 2 - pixmapOffset.y();
        scroll(deltaX, deltaY);
        return;
    }
    
    if (event->button() == Qt::RightButton) {
        zoomToSelectedRectangle(event);
    }
}

//TODO : this function needs revision to handle higher precision types properly
void MandelbrotWidget::zoomToSelectedRectangle(QMouseEvent *event)
{
    static const double MINIMUM_ZOOM_FACTOR = 0.01;

    newSelectionArea->hide();
    auto farX = event->pos().x();
    auto farY = event->pos().y();

    auto originalPixelCount = pixmap.width() * pixmap.height();
    int newWidth = abs(farX - newSelectionOrigin.x());
    int newHeight = abs(farY - newSelectionOrigin.y());
    auto newPixelCount = newWidth * newHeight;
    double scaleFactor = sqrt(static_cast<double>(newPixelCount) / static_cast<double>(originalPixelCount));
    if (scaleFactor > MINIMUM_ZOOM_FACTOR) {
        auto centresTranslationX = (farX + newSelectionOrigin.x() - pixmap.width()) / 2;
        auto centresTranslationY = (farY + newSelectionOrigin.y() - pixmap.height()) / 2;
        scroll(centresTranslationX, centresTranslationY);

        zoom(scaleFactor);
        resize(QSize(static_cast<int>(newWidth / scaleFactor), static_cast<int>(newHeight / scaleFactor)));
    }
}

void MandelbrotWidget::processSettingUpdate(QSettings &settings)
{
    settings.beginGroup("MainWindow");
        resize(settings.value("size", QSize(kInitialWidth, kInitialHeight)).toSize());
        move(settings.value("pos", QPoint(kInitialPosCoord, kInitialPosCoord)).toPoint());
    settings.endGroup();

    settings.beginGroup("RenderParameters");
        centerX = settings.value("centerX", DefaultCenterX).toDouble();
        centerY = settings.value("centerY", DefaultCenterY).toDouble();
        curScale = settings.value("curScale", DefaultScale).toDouble();
        pixmapScale = settings.value("pixmapScale", DefaultScale).toDouble();
    settings.endGroup();
}
//! [15]
//!
 void MandelbrotWidget::outputToLog(const QString& outputString, logMode outputMode, bool show )
 {
     if (!show)
     {
         return;
     }
    QMutexLocker locker(&mutex);

    QTextEdit* loggerToUse = outputMode == logMode::normal ? logger : miscLogger;

    loggerToUse->append(outputString);

 }

 void MandelbrotWidget::outputToLog(const QString& outputString, bool show)
 {
     outputToLog(outputString, logMode::normal, show);
 }

//! [16]
void MandelbrotWidget::updatePixmap(const QImage *image, double scaleFactor)
{
    if (!lastDragPos.isNull())
        return;

    pixmap = QPixmap::fromImage(*image);
    pixmapOffset = QPoint();
    lastDragPos = QPoint();
    pixmapScale = scaleFactor;
    updateView();
    outputToLog("update Pixmap!");
}
//! [16]
//!

void MandelbrotWidget::updateView()
{
    update();
}

void MandelbrotWidget::updateCoordInfo(double centerX, double centerY, double scaleFactor,
                     QSize resultSize)
{
    infoDisplayer->updateCoordInfo(centerX, centerY, scaleFactor, resultSize);
}

void MandelbrotWidget::setThreadsInfo(int numThreads)
{
    infoDisplayer->setThreadsInfo(numThreads);
}

void MandelbrotWidget::setPassesInfo(int numPasses)
{
    infoDisplayer->setPassesInfo(numPasses);
}

void MandelbrotWidget::setDynamicTasksInfo(bool dynamicTasksEnabled)
{
    infoDisplayer->setDynamicTasksInfo(dynamicTasksEnabled);
}

void MandelbrotWidget::setIterationSumCount(int64_t iterationSum)
{
    infoDisplayer->setIterationSumCount(iterationSum);
    setIterationsPerPixel(iterationSum);
}

void MandelbrotWidget::setIterationsPerPixel(int64_t iterationSum)
{
    infoDisplayer->setIterationsPerPixel(static_cast<double>(iterationSum) * perPixelCoeff);
}

void MandelbrotWidget::setColorMapSizeInfo(int colorMapSize)
{
    infoDisplayer->setColorMapSize(colorMapSize);
}

void MandelbrotWidget::setInternalDataType(const QString& description)
{
    infoDisplayer->setInternalDataType(description);
}

float MandelbrotWidget::getElapsedTimeDisplayed() const
{
    return infoDisplayer->getElapsedTimeDisplayed();
}

void MandelbrotWidget::initiateQuit()
{
    std::cout << static_cast<const char*>(__FUNCTION__) << std::endl;
    if (!confirmOperation("<b>Quit Renderer<b>", 
        "Do you really want to exit?", unsavedChangesExist))
    {
        return;
    }
    release(true);
    emit quitAll();
    executeRender(true);
}

void MandelbrotWidget::initiateHalt()
{
    std::cout << static_cast<const char*>(__FUNCTION__) << std::endl;
    release(true);
    getInfoDisplayer()->setRenderState(InformationDisplay::renderState::aborted);
    emit halt();
}

void MandelbrotWidget::initiateRefresh()
{
    std::cout << static_cast<const char*>(__FUNCTION__) << std::endl;
    bool restartingPausedRun = paused.load();
    release();

    if (restartingPausedRun)
    {
        getInfoDisplayer()->setRenderState(InformationDisplay::renderState::running);
        return;
    }

    executeRender();
}

void MandelbrotWidget::initiatePause()
{
    if (!renderInProgress)
    {
        return;
    }

    getInfoDisplayer()->setRenderState(InformationDisplay::renderState::paused);
    pause();
}

void MandelbrotWidget::accomplishQuit()
{
    std::cout << static_cast<const char*>(__FUNCTION__) << std::endl;
    QApplication::closeAllWindows();
    while (thread.isRunning());
    std::cout << "isRunning returned false!" << std::endl;
    QApplication::quit();
}


//! [17]
void MandelbrotWidget::executeRender(bool hideProgress)
{
    getInfoDisplayer()->setRenderState(InformationDisplay::renderState::running);
    release(true);
    paused.store(false);
    if (!hideProgress) {
        toolsMenu->setEnabled(false);
        showProgress();
    }
    if (!usingUndoRedo) {
        if (historyLog->saveState() && historyLog->undoIsPossible()) {
            unsavedChangesExist = true;
        }
    }
    thread.render(centerX, centerY, curScale, size());
    usingUndoRedo = false;
}

void MandelbrotWidget::zoom(double zoomFactor)
{
    curScale *= zoomFactor;
    updateView();
    executeRender();
    outputToLog("New operation : zoom");
    scaleHasChanged = true;
}

void MandelbrotWidget::enableOptions()
{
    toolsMenu->setEnabled(true);
}

void MandelbrotWidget::disableOptions()
{
    toolsMenu->setEnabled(false);
}
//! [17]

//! [18]
void MandelbrotWidget::scroll(int deltaX, int deltaY)
{
    centerX += deltaX * curScale;
    centerY += deltaY * curScale;
    updateView();
    executeRender();
    outputToLog("New operation : scroll");
}
//! [18]
