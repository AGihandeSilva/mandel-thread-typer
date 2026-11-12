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
#include "ParametersMenu.h"
#include "RenderMenu.h"
#include "windowmenu.h"
#include "toolsoptionswidget.h"
#include "settingshandler.h"
#include "PrecisionHandler.h"


//! [0]
const double DefaultScale = 0.00403897;

const double ZoomInFactor = 0.8;
const double ZoomOutFactor = 1 / ZoomInFactor;
const int ScrollStep = 20;

QString MandelbrotWidget::m_DefaultOriginX("-0.637011");
QString MandelbrotWidget::m_DefaultOriginY("-0.0395159");

QString MandelbrotWidget::s_undefinedFloatString = "NaN";
QString MandelbrotWidget::s_unInitializedFloatString = "-1.0";

#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
using MandelBrotRenderer::generateFloatFromPreciseString;
#endif
//! [0]

//! [1]

using namespace MandelBrotRenderer;

MandelbrotWidget::MandelbrotWidget(QWidget *parent)
    : QMainWindow(parent), m_settingsHandler(this), m_infoDisplayer(*this, m_settingsHandler), m_thread(m_settingsHandler, this),
      m_originX(s_unInitializedFloatString), m_originY(s_unInitializedFloatString),
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
        m_preciseOriginX(s_undefinedFloatString), m_preciseOriginY(s_undefinedFloatString),
#endif
      m_pixmapScale(notYetInitializedDouble),
      m_curScale(notYetInitializedDouble), m_perPixelCoeff(notYetInitializedDouble),
      m_scaleHasChanged(false), m_renderInProgress(false), m_usingUndoRedo(false), m_numericType(internalDataType::doublePrecisionFloat),
      m_pauseMutexes(MAX_NUM_WORKER_THREADS, nullptr), m_paused(),
      m_historyLog(std::make_unique<RenderHistory>(this)), m_unsavedChangesExist(false),
      m_parameterSpace(-1.5, 2.5, -2.0, 2.5)
{
    m_thread.processSettingUpdate(m_settingsHandler.getSettings());

    static const int desiredPrecision = 6;
    registerCoordinateUser(&m_infoDisplayer, desiredPrecision);
    m_toolsMenu = new ToolsMenu(this->menuBar(), this, &m_thread, m_settingsHandler);
    m_editMenu = new EditMenu(this);
    m_fileMenu = new FileMenu(this->menuBar(), this, m_settingsHandler);
    m_parametersMenu = new ParametersMenu(this->menuBar(), this, &m_thread, m_settingsHandler);
    m_renderMenu = new RenderMenu(this);
    m_windowMenu = new WindowMenu(&m_thread);
    m_centralPlotArea = new QWidget();
    m_messageBar = new QStatusBar(parent);
    m_newSelectionArea = new QRubberBand(QRubberBand::Rectangle, this);
    m_mandelbrotWidgetLayout =  new QVBoxLayout();
    m_progressBar =  new QProgressBar();
    m_logger = new QTextEdit();
    m_miscLogger = new QTextEdit();

    qRegisterMetaType<ComputedDataSegment>("ComputedDataSegment&");

    connectToComponents();

    QApplication::setQuitOnLastWindowClosed(true);

    setWindowTitle(tr("Mandelbrot"));

    getInfoDisplayer().prepareTimerInfo();

    prepareProgressBar();

    prepareMenuBar();

    preparePauseLocks();

    setupLayout();

    setupLoggers();

#ifndef QT_NO_CURSOR
    setCursor(Qt::CrossCursor);
#endif
    processWindowSize();
    writeSettings();
}

void MandelbrotWidget::connectToComponents()
{
    setStatusBar(m_messageBar);
    m_settingsHandler.registerSettingsUser(this);
    m_thread.setOwnerOnce(this);

    connect(&m_thread, SIGNAL(renderedImage(const QImage*,double)), this, SLOT(updatePixmap(const QImage*,double)));

    connect(this, SIGNAL(quitAll()), &m_thread, SLOT(quitApplication()));
    connect(this, SIGNAL(halt()), &m_thread, SLOT(haltComputations()));

    connect(&m_thread, SIGNAL(numThreadsUpdate()), this, SLOT(resizeProgressBar()));
    connect(&m_thread, SIGNAL(numPassesUpdate()), this, SLOT(resizeProgressBar()));

    connect(this, SIGNAL(halt()), this, SLOT(handleHalt()));
    connect(this, SIGNAL(halt()), &m_thread, SLOT(stopTimer()));

    connect (&m_thread, SIGNAL(finished()), this, SLOT(accomplishQuit()));

    connect(&m_thread, SIGNAL(allDone()), this, SLOT(enableOptions()));

    connect(&m_thread, SIGNAL(numPassesUpdate()), this, SLOT(resizeProgressBar()));

    connect(&m_thread, SIGNAL(writeToLog(QString,bool)), this, SLOT(outputToLog(QString,bool)));

    connect(&m_thread, SIGNAL(renderStarting()), this, SLOT(disableFileMenu()));
    connect(&m_thread, SIGNAL(allDone()), this, SLOT(enableFileMenu()));
}

void MandelbrotWidget::preparePauseLocks()
{
    for (auto& i : m_pauseMutexes) {
        i = new QMutex();
    }
}

void MandelbrotWidget::prepareMenuBar()
{
    this->menuBar()->setStyleSheet("QMenuBar {background: lightgray; }");
    this->menuBar()->setLayout(new QHBoxLayout());

    menuBar()->addMenu(m_fileMenu);
    menuBar()->addMenu(m_editMenu);
    menuBar()->addMenu(m_toolsMenu);
    menuBar()->addMenu(m_parametersMenu);
    menuBar()->addMenu(m_renderMenu);
    menuBar()->addMenu(m_windowMenu);
    menuBar()->addSeparator();

    prepareButtons();

    menuBar()->show();
}

void MandelbrotWidget::prepareButtons()
{
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
}

void MandelbrotWidget::setupLayout()
{
    this->setCentralWidget(m_centralPlotArea);
    this->centralWidget()->setLayout(m_mandelbrotWidgetLayout);

    m_mandelbrotWidgetLayout->addWidget(&m_infoDisplayer);

    m_mandelbrotWidgetLayout->addWidget(m_progressBar);

    m_centralPlotArea->show();
}

void MandelbrotWidget::setupLoggers()
{
    outputToLog("Log Output");
    outputToLog("->");
    m_logger->setWindowFlags(Qt::Window | Qt::WindowTitleHint |
                           Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::CustomizeWindowHint);
    m_logger->show();
}

void MandelbrotWidget::processWindowSize()
{
    QMutexLocker locker(&m_infoDisplayMutex);
    QSize currentSize = size();
    getInfoDisplayer().updatePixmapSizeInfo(currentSize);
    const int pixelCount = currentSize.width() * currentSize.height();
    m_perPixelCoeff = (pixelCount != 0) ? 1.0 / static_cast<double>(pixelCount) : notYetInitializedDouble;
}

void MandelbrotWidget::adjustProgressBar()
{
    m_progressBar->reset();
    progressMade(0);
    m_progressBar->setMaximum(m_thread.getRunningNumPasses() * m_thread.getNumWorkerThreads());
}

void MandelbrotWidget::writeSettings()
{
    m_settingsHandler.getSettings().beginGroup("MainWindow");
        m_settingsHandler.getSettings().setValue("size", size());
        m_settingsHandler.getSettings().setValue("pos", pos());
    m_settingsHandler.getSettings().endGroup();

    m_settingsHandler.getSettings().beginGroup("RenderParameters");
        m_settingsHandler.getSettings().setValue("X", m_originX);
        m_settingsHandler.getSettings().setValue("Y", m_originY);
        m_settingsHandler.getSettings().setValue("curScale", m_curScale);
        m_settingsHandler.getSettings().setValue("pixmapScale", m_pixmapScale);
    m_settingsHandler.getSettings().endGroup();

    //TODO automate this using the SettingsUser interface?
    m_thread.writeSettings();
    {
        QMutexLocker locker(&m_infoDisplayMutex);
         getInfoDisplayer().writeSettings();

    }
    m_fileMenu->writeSettings();
}

void MandelbrotWidget::closeEvent(QCloseEvent *event)
{
    writeSettings();
    m_settingsHandler.reset();
    event->accept();
}

InformationDisplay& MandelbrotWidget::getInfoDisplayer()
{
    return m_infoDisplayer;
}

RendererConfig MandelbrotWidget::generateConfigData()
{
    QMutexLocker locker(&m_infoDisplayMutex);
    auto result = RendererConfig(getInfoDisplayer().getDetailedDisplayEnabled(),
                          size(),
                          pos(),
                          m_originX,
                          m_originY,
                          m_curScale,
                          m_pixmapScale,
                          m_thread.getRendererData().m_threadMediatorEnabled,
                          m_thread.getRendererData().m_colorMapSize,
                          m_thread.getRendererData().m_numericType);

    return (result);
}

void MandelbrotWidget::enforceConfigData(RendererConfig &newConfigData)
{
    RendererConfig originalConfig = generateConfigData();

    QMutexLocker locker(&m_infoDisplayMutex);
    getInfoDisplayer().setEnabled(newConfigData.getDetailedDisplayEnabled());
    m_originX = QString::number(newConfigData.getOriginX());
    m_originY = QString::number(newConfigData.getOriginY());
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
    m_preciseOriginX = newConfigData.getPreciseOriginX();
    m_preciseOriginY = newConfigData.getPreciseOriginY();
#endif
    m_curScale = newConfigData.getCurScale();
    m_pixmapScale = newConfigData.getPixmapScale();
    m_thread.getThreadMediator().setEnabled(newConfigData.getThreadMediatorEnabled());
    setInternalDataType(newConfigData.getInternalDataType());

    move(newConfigData.getPos());
    resize(newConfigData.getSize());

    if (originalConfig != newConfigData) {
        initiateRefresh();
    }
}
void MandelbrotWidget::prepareProgressBar()
{
    m_progressBar->setMinimum(0);
    adjustProgressBar();
    m_progressBar->setStyleSheet("QProgressBar {color : darkgray; }");
    hideProgress();

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
        m_fileMenu->saveParamsAs();
    }

    return (ret != QMessageBox::Cancel);
}

void MandelbrotWidget::showProgress()
{
    m_progressBar->setVisible(true); outputToLog("Showing Progress!");
}
void MandelbrotWidget::hideProgress()
{
    m_progressBar->setVisible(false); outputToLog("Hiding Progress!");
    //QApplication::beep();
}

void MandelbrotWidget::handleHalt()
{
    QMutexLocker locker(&m_infoDisplayMutex);
    getInfoDisplayer().stopProgress();
}

void MandelbrotWidget::handleRenderStarting()
{
    m_renderInProgress = true;
    showProgress();
    resetElapsedTime();
}

void MandelbrotWidget::handleAllDone()
{
    hideProgress();
    m_paused.store(false);
    m_renderInProgress = false;
}

void MandelbrotWidget::resetElapsedTime()
{
    QMutexLocker locker(&m_infoDisplayMutex);
    getInfoDisplayer().resetElapsedTimeInfo();
    getInfoDisplayer().startProgress();
    //QApplication::beep();
}
void MandelbrotWidget::incrementElapsedTime()
{
    QMutexLocker locker(&m_infoDisplayMutex);
    getInfoDisplayer().incrementTimeInfo();
}

void MandelbrotWidget::resizeProgressBar()
{
    adjustProgressBar();
}

void MandelbrotWidget::restoreDefaults()
{
    m_settingsHandler.getSettings().remove("MainWindow");
    m_settingsHandler.getSettings().remove("RenderParameters");
    m_settingsHandler.getSettings().remove("WindowThreadInfo");
    m_settingsHandler.getSettings().remove("Renderer");
    m_settingsHandler.getSettings().remove("InformationDisplay");

    m_settingsHandler.notifyUsersSettingsUpdatedByOwner(this);
    m_toolsMenu->refreshOptionsGUI();
    m_parametersMenu->refreshParametersGUI();

    update();
    if (QSize(kInitialWidth, kInitialHeight) == size()) {
        executeRender();
    }
}

void MandelbrotWidget::disableFileMenu()
{
    m_fileMenu->setEnabled(false);
}

void MandelbrotWidget::enableFileMenu()
{
    m_fileMenu->setEnabled(true);
}

void MandelbrotWidget::writeStatusMessage(const QString& message, bool isWarning, int timeout)
{
    const QString colour = isWarning ? "red" : "black";
    statusBar()->setStyleSheet(colour);
    statusBar()->showMessage(message, timeout);
}

void MandelbrotWidget::writeTransientStatusMessage(const QString& message, InformationDisplay::renderState renderState, bool isWarning)
{
    writeStatusMessage(message, isWarning, TRANSIENT_MESSAGE_DURATION_IN_MS);

    QMutexLocker locker(&m_infoDisplayMutex);
    getInfoDisplayer().setRenderState(renderState);
}

void MandelbrotWidget::pause()
{
    int locked = 0;
    if (m_paused.load())
    {
        return;
    }
    QMutexLocker locker(&m_mutex);
    m_paused.store(true);

    for (auto& i : m_pauseMutexes)
    {
        i->lock();
        ++locked;
    }
    m_thread.pauseTimer();
}

void MandelbrotWidget::release(bool forceFlow)
{
    if (!forceFlow && !(m_paused.load()))
    {
        return;
    }

    QMutexLocker locker(&m_mutex);
    m_paused.store(false);

    for (auto& i : m_pauseMutexes) {
        i->try_lock();
        i->unlock();
    }
    m_thread.resumeTimer();
    
}

void MandelbrotWidget::undo()
{
    m_historyLog->undo();
}

void MandelbrotWidget::redo()
{
    m_historyLog->redo();
}

void MandelbrotWidget::setUndoEnabled(bool enabled)
{
    m_editMenu->setUndoEnabled(enabled);
}

void MandelbrotWidget::setRedoEnabled(bool enabled)
{
    m_editMenu->setRedoEnabled(enabled);
}

void MandelbrotWidget::registerCoordinateUser(MandelBrotRenderer::CoordinateListener* listener,
                                              MandelBrotRenderer::CoordinateListenerConfig config)
{
    m_coordinateUsers.push_back(std::make_pair(listener, config));
}

template <typename T>
bool MandelbrotWidget::validateRegion(T originX, T originY, T width, T height)
{
    bool regionOK = true;

    if (width < T(0.0) || height < T(0.0)) {
        regionOK = false;
    }

    regionOK = regionOK && (originX > static_cast<T>(m_parameterSpace.m_xMin));
    regionOK = regionOK && (originY > static_cast<T>(m_parameterSpace.m_yMin));
    regionOK = regionOK && (originX + width < static_cast<T>(m_parameterSpace.m_xMax));
    regionOK = regionOK && (originY + height < static_cast<T>(m_parameterSpace.m_yMax));

    return regionOK;
}

/*
 * Adjust the rendered region to match the given parameters
 * (if they are contained within the allowed parameter space)
 *
 * NB: this function expects string parameters holding
 * pre-validated numeric values
 *
 * If the change occurs, the exact width and height will
 * probably not exactly match the input values, due to
 * pixel boundaries
 *
 * This function uses the current pixel scaling -
 * it will not execute the change if the final image will
 * be very small or large using that scaling
 *
 * returns: true if the change was accepted and made,
 *          false otherwise
 */
bool MandelbrotWidget::changeRegionParameters(const QString &originXvalue,
                                              const QString &originYvalue,
                                              const QString &widthValue,
                                              const QString &heightValue)
{
    int pWidth = m_pixmap.width();
    int pHeight = m_pixmap.height();
    if (pWidth <= 0 || pHeight <= 0) {
        return false;
    }

#if (USE_BOOST_MULTIPRECISION == 1 || defined(__GNUC__))
    PreciseFloatResult originX_result = generateFloatFromPreciseString(originXvalue);
    PreciseFloatResult originY_result = generateFloatFromPreciseString(originYvalue);
#else
    auto originX_result  = generateFloatFromString(originXvalue);
    auto originY_result  = generateFloatFromString(originYvalue);
#endif

    auto width_result    = generateFloatFromString(widthValue);
    auto height_result   = generateFloatFromString(heightValue);

    Q_ASSERT(originX_result.second);
    auto originX_f = originX_result.first;
    Q_ASSERT(originY_result.second);
    auto originY_f = originY_result.first;
    Q_ASSERT(width_result.second);
    auto width_f = width_result.first;
    Q_ASSERT(height_result.second);
    auto height_f = height_result.first;

    bool regionOk= originX_result.second && originY_result.second &&
            width_result.second && height_result.second;

#if (USE_BOOST_MULTIPRECISION == 1 || defined(__GNUC__))
    regionOk = regionOk && validateRegion<Float128>(originX_f, originY_f,
                                                    static_cast<Float128>(width_f),
                                                    static_cast<Float128>(height_f));
#else
    regionOk = regionOk && validateRegion<double>(originX_f, originY_f, width_f, height_f);
#endif

    std::cout << "new Region OK:" << regionOk << std::endl;

    if (regionOk) {

        //prepare to do some more validity checks
        regionOk = false;

        constexpr int minimumWidgetSize = 50;
        constexpr int maximumPixelWidth = 4000;
        constexpr int maximumPixelHeight = 2000;
        constexpr int maxPixelCount = maximumPixelWidth * maximumPixelHeight;

        //compute the old and new sizes of the set parameter regions
        const auto originalPixelCount = m_pixmap.width() * m_pixmap.height();
        const double regionSize = m_curScale * originalPixelCount * m_curScale;

        Q_ASSERT(regionSize > 0);
        if (regionSize > 0) {

            //TODO: switch between these approaches depending on a GUI switch?
            //scale to preserve the pixel area of the widget
            //const auto newRegionSize = width_f * height_f;
            //auto newScale = m_curScale * sqrt(newRegionSize / regionSize);

            //preserve scale
            auto newScale = m_curScale;

            if (newScale > 0) {
                //recompute the new widths
                int newWidth = static_cast<int>(width_f / newScale);
                int newHeight = static_cast<int>(height_f / newScale);

                int pixelCount = newWidth * newHeight;

                if (pixelCount > minimumWidgetSize &&
                    pixelCount < maxPixelCount) {

                    m_originX = originXvalue;
                    m_originY = originYvalue;
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
                    m_preciseOriginX = originXvalue;
                    m_preciseOriginY = originYvalue;
#endif

                    m_curScale = newScale;
                    m_pixmapScale = newScale;
                    if (newWidth != pWidth ||
                        newHeight != pHeight) {
                        resize(static_cast<int>(newWidth), static_cast<int>(newHeight));
                    } else {
                        initiateRefresh();
                    }
                    regionOk = true;
                }
            }
        }
    }

    return regionOk;
}

void MandelbrotWidget::progressMade(int computationChunksDone)
{
    outputToLog("<<< ProgressMade" + QString::number(computationChunksDone) + ">>>");
    m_progressBar->setValue(computationChunksDone);
    QMutexLocker locker(&m_infoDisplayMutex);
    getInfoDisplayer().updateProgress(static_cast<uint>((100 * computationChunksDone) / m_progressBar->maximum()));
}
//! [1]

//! [2]
void MandelbrotWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    if (m_pixmap.isNull()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, tr("Rendering initial image, please wait..."));
//! [2] //! [3]
        return;
//! [3] //! [4]
    }
//! [4]

//! [5]
//    if (m_curScale == m_pixmapScale) {
    if (m_scaleHasChanged) {
//! [5] //! [6]
        painter.drawPixmap(m_pixmapOffset, m_pixmap);
//! [6] //! [7]
    } else {
//! [7] //! [8]
        double scaleFactor = m_pixmapScale / m_curScale;
        auto newWidth = int(m_pixmap.width() * scaleFactor);
        auto newHeight = int(m_pixmap.height() * scaleFactor);
        auto newX = m_pixmapOffset.x() + (m_pixmap.width() - newWidth) / 2;
        auto newY = m_pixmapOffset.y() + (m_pixmap.height() - newHeight) / 2;

        painter.save();
        painter.translate(newX, newY);
        painter.scale(scaleFactor, scaleFactor);
        QRectF exposed = painter.transform().inverted().mapRect(rect());
        exposed = exposed.adjusted(-1, -1, 1, 1);
        painter.drawPixmap(exposed, m_pixmap, exposed);
        painter.restore();
    }
    m_scaleHasChanged = false;
//! [8] //! [9]

    QString text = tr("Use mouse wheel or the '+' and '-' keys to zoom. "
                      "Press and hold left mouse button to scroll.");
    QFontMetrics metrics = painter.fontMetrics();
    int textWidth = metrics.horizontalAdvance(text);

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
    int numDegrees = event->angleDelta().y() / 8;
    double numSteps = numDegrees / 15.0;
    zoom(pow(ZoomInFactor, numSteps));
}
//! [12]
#endif

//! [13]
void MandelbrotWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_lastDragPos = event->pos();
        return;
    }

    if (event->button() == Qt::RightButton) {
        m_newSelectionOrigin = event->pos();
        m_newSelectionArea->setGeometry(QRect(m_newSelectionOrigin, QSize()));
        m_newSelectionArea->show();
    }
}
//! [13]

//! [14]
void MandelbrotWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_pixmapOffset += event->pos() - m_lastDragPos;
        m_lastDragPos = event->pos();
        update();
        return;
    }
    if(event->buttons() & Qt::RightButton) {
        m_newSelectionArea->setGeometry(QRect(m_newSelectionOrigin, event->pos()).normalized());
    }
}
//! [14]

//! [15]
void MandelbrotWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pixmapOffset += event->pos() - m_lastDragPos;
        m_lastDragPos = QPoint();

        int deltaX = (width() - m_pixmap.width()) / 2 - m_pixmapOffset.x();
        int deltaY = (height() - m_pixmap.height()) / 2 - m_pixmapOffset.y();
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

    m_newSelectionArea->hide();
    auto farX = event->pos().x();
    auto farY = event->pos().y();

    auto originalPixelCount = m_pixmap.width() * m_pixmap.height();
    int newWidth = abs(farX - m_newSelectionOrigin.x());
    int newHeight = abs(farY - m_newSelectionOrigin.y());
    auto newPixelCount = newWidth * newHeight;
    double scaleFactor = sqrt(static_cast<double>(newPixelCount) / static_cast<double>(originalPixelCount));
    if (scaleFactor > MINIMUM_ZOOM_FACTOR) {
        auto centresTranslationX = (farX + m_newSelectionOrigin.x() - m_pixmap.width()) / 2;
        auto centresTranslationY = (farY + m_newSelectionOrigin.y() - m_pixmap.height()) / 2;
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

    m_settingsHandler.getXYSettingsData(m_originX, m_originY);
    settings.beginGroup("RenderParameters");
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
        m_preciseOriginX = m_originX;
        m_preciseOriginY = m_originY;
#endif
        m_curScale = settings.value("curScale", DefaultScale).toDouble();
        m_pixmapScale = settings.value("pixmapScale", DefaultScale).toDouble();
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
    QMutexLocker locker(&m_mutex);

    QTextEdit* loggerToUse = outputMode == logMode::normal ? m_logger : m_miscLogger;

    loggerToUse->append(outputString);

 }

 void MandelbrotWidget::outputToLog(const QString& outputString, bool show)
 {
     outputToLog(outputString, logMode::normal, show);
 }

//! [16]
void MandelbrotWidget::updatePixmap(const QImage *image, double scaleFactor)
{
    if (!m_lastDragPos.isNull())
        return;

    m_pixmap = QPixmap::fromImage(*image);
    m_pixmapOffset = QPoint();
    m_lastDragPos = QPoint();
    m_pixmapScale = scaleFactor;
    update();
    outputToLog("update Pixmap!");
}
//! [16]
//!

void MandelbrotWidget::updateCoordInfo(const MandelBrotRenderer::CoordValue& originX, const MandelBrotRenderer::CoordValue& originY,
                                       double scaleFactor)
{
    QString originX_s;
    QString originY_s;
    QSize currentSize = size();
    QString widthAsString(QString::number(currentSize.width() * scaleFactor));
    QString heightAsString(QString::number(currentSize.height() * scaleFactor));

    QMutexLocker locker(&m_infoDisplayMutex);

    for (auto i : m_coordinateUsers) {
        int precision = i.second;

        //try to generate a numeric variable to ensure the numeric format is valid
        MandelBrotRenderer::DoubleResult originX_float = generateFloatFromString(originX);
        if (originX_float.second) {
            if (precision > 0) {
                originX_s.setNum(originX_float.first, 'g', precision);
            } else {
                originX_s = originX;
            }
        }
        MandelBrotRenderer::DoubleResult originY_float = generateFloatFromString(originY);
        if (originY_float.second) {
            if (precision > 0) {
                originY_s.setNum(originY_float.first, 'g', precision);
            } else {
                originY_s = originY;
            }
        }

        (i.first)->updateCoordData(originX_s, originY_s, widthAsString, heightAsString);
        originX_s.clear();
        originY_s.clear();
    }
}

void MandelbrotWidget::displayThreadsInfo(int numThreads)
{
    QMutexLocker locker(&m_infoDisplayMutex);
    getInfoDisplayer().setThreadsInfo(numThreads);
}

void MandelbrotWidget::displayPassesInfo(int numPasses)
{
    QMutexLocker locker(&m_infoDisplayMutex);
    getInfoDisplayer().setPassesInfo(numPasses);
}

void MandelbrotWidget::HandleDisplayDynamicTasksInfo(bool dynamicTasksEnabled)
{
    QMutexLocker locker(&m_infoDisplayMutex);
    getInfoDisplayer().setDynamicTasksInfo(dynamicTasksEnabled);
}
void MandelbrotWidget::UpdateIterationCount(qint64 iterationCount)
{
    setIterationSumCount(iterationCount);
}

void MandelbrotWidget::setIterationSumCount(qint64 iterationSum)
{
    {
        QMutexLocker locker(&m_infoDisplayMutex);
        getInfoDisplayer().setIterationSumCount(iterationSum);
    }
    displayIterationsPerPixel(iterationSum);
}

void MandelbrotWidget::displayIterationsPerPixel(qint64 iterationSum)
{
    QMutexLocker locker(&m_infoDisplayMutex);
    getInfoDisplayer().setIterationsPerPixel(static_cast<double>(iterationSum) * m_perPixelCoeff);
}

void MandelbrotWidget::displayColorMapSizeInfo(int m_colorMapSize)
{
    QMutexLocker locker(&m_infoDisplayMutex);
    getInfoDisplayer().setColorMapSize(m_colorMapSize);
}

void MandelbrotWidget::displayInternalDataType(const QString& description)
{
    if (m_thread.getTypeIsSupported(description)){
        QMutexLocker locker(&m_infoDisplayMutex);
        getInfoDisplayer().displayInternalDataType(description);
    }
}

void MandelbrotWidget::setInternalDataType(internalDataType newTypeSetting)
{
    m_numericType = m_thread.getTypeIsSupported(newTypeSetting) ?
                newTypeSetting : MandelBrotRenderer::defaultRendererType;

    QString typeDescription = m_thread.getTypeDescription(m_numericType);
    Q_ASSERT(!typeDescription.isEmpty());
    displayInternalDataType(typeDescription);

    //TODO : confirm that this absolutely cannot occur when a render is underway
    m_thread.setInternalDataType(typeDescription);
}

float MandelbrotWidget::getElapsedTimeDisplayed()
{
    QMutexLocker locker(&m_infoDisplayMutex);
    return getInfoDisplayer().getElapsedTimeDisplayed();
}

void MandelbrotWidget::initiateQuit()
{
    std::cout << static_cast<const char*>(__FUNCTION__) << std::endl;
    if (!confirmOperation("<b>Quit Renderer<b>", 
        "Do you really want to exit?", m_unsavedChangesExist))
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
    { QMutexLocker locker(&m_infoDisplayMutex);
        getInfoDisplayer().setRenderState(
            InformationDisplay::renderState::aborted);
    }
    
    emit halt();
}

void MandelbrotWidget::initiateRefresh()
{
    std::cout << static_cast<const char*>(__FUNCTION__) << std::endl;
    bool restartingPausedRun = m_paused.load();
    release();

    if (restartingPausedRun)
    {
        QMutexLocker locker(&m_infoDisplayMutex);
        getInfoDisplayer().setRenderState(InformationDisplay::renderState::running);
        return;
    }

    executeRender();
}

void MandelbrotWidget::initiatePause()
{
    if (!m_renderInProgress)
    {
        return;
    }

    {
        QMutexLocker locker(&m_infoDisplayMutex);
        getInfoDisplayer().setRenderState(
            InformationDisplay::renderState::paused);
    }
    
    pause();
}

void MandelbrotWidget::accomplishQuit()
{
    std::cout << static_cast<const char*>(__FUNCTION__) << std::endl;
    QApplication::closeAllWindows();
    while (m_thread.isRunning());
    std::cout << "isRunning returned false!" << std::endl;
    m_thread.quit();
    m_thread.wait();
    QApplication::quit();
}


//! [17]
void MandelbrotWidget::executeRender(bool hideProgress)
{
    { QMutexLocker locker(&m_infoDisplayMutex);
        getInfoDisplayer().setRenderState(
            InformationDisplay::renderState::running);
    }
    
    release(true);
    m_paused.store(false);
    if (!hideProgress) {
        m_toolsMenu->setEnabled(false);
        showProgress();
    }
    if (!m_usingUndoRedo) {
        if (m_historyLog->saveState() && m_historyLog->undoIsPossible()) {
            m_unsavedChangesExist = true;
        }
    } else if (m_historyLog->redoIsPossible()) {
        //intermediate state that should be offered for saving on exit
        m_unsavedChangesExist = true;
    }

#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
    if (m_preciseOriginX == s_undefinedFloatString || m_preciseOriginY == s_undefinedFloatString) {
        m_preciseOriginX = m_originX;
        m_preciseOriginY = m_originY;
    }
#endif

    m_thread.render(m_originX, m_originY,
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
                m_preciseOriginX, m_preciseOriginY,
#endif
                  m_curScale, size());
}

void MandelbrotWidget::zoom(double zoomFactor)
{
    m_curScale *= zoomFactor;
    update();
    executeRender();
    outputToLog("New operation : zoom");
    m_scaleHasChanged = true;
}

void MandelbrotWidget::enableOptions()
{
    m_toolsMenu->setEnabled(true);
}

void MandelbrotWidget::disableOptions()
{
    m_toolsMenu->setEnabled(false);
}

QString MandelbrotWidget::computeDeltaWithHigherPrecision(QString& value, int deltaPixels, double currentScale)
{
#if (USE_BOOST_MULTIPRECISION == 1 || defined(__GNUC__))
    Float128 preciseDelta = deltaPixels * static_cast<Float128>(currentScale);
    PreciseFloatResult floatValue = generateFloatFromPreciseString(value);
    Q_ASSERT(floatValue.second);
    Float128 preciseOrigin = floatValue.second ? floatValue.first : Float128(0.0);
    Float128 preciseValue = preciseOrigin + preciseDelta;
    return (MandelBrotRenderer::generatePreciseFloatingPointString(preciseValue));
#else
    auto floatValue = generateFloatFromString(value);
    double originValue = floatValue.second ? floatValue.first : 0.0;
    double preciseValue = originValue + deltaPixels * currentScale;
    return (MandelBrotRenderer::generateDoubleAsString(preciseValue));
#endif
}

//! [18]
void MandelbrotWidget::scroll(int deltaX, int deltaY)
{
#if (USE_BOOST_MULTIPRECISION == 1 || defined(__GNUC__))
    m_preciseOriginX = computeDeltaWithHigherPrecision(m_originX, deltaX, m_curScale);
    m_preciseOriginY = computeDeltaWithHigherPrecision(m_originY, deltaY, m_curScale);
    m_originX = m_preciseOriginX;
    m_originY = m_preciseOriginY;
#else
    auto parsedOriginX = generateFloatFromString(m_originX);
    double originX_origin = parsedOriginX.second ? parsedOriginX.first : 0.0;
    double currentOriginX = originX_origin + deltaX * m_curScale;

    auto parsedOriginY = generateFloatFromString(m_originY);
    double originY_origin = parsedOriginY.second ? parsedOriginY.first : 0.0;
    double currentOriginY = originY_origin + deltaY * m_curScale;
    m_originX = MandelBrotRenderer::generateDoubleAsString(currentOriginX);
    m_originY = MandelBrotRenderer::generateDoubleAsString(currentOriginY);
#endif
    update();
    executeRender();
    outputToLog("New operation : scroll");
}
//! [18]
