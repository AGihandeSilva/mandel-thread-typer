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

#ifndef MANDELBROTWIDGET_H
#define MANDELBROTWIDGET_H

#include <QTextEdit>
#include <QVBoxLayout>
#include <QMutex>
#include <QProgressBar>
#include <QMenu>
#include <QStatusBar>
#include <QToolBar>

#include <QPixmap>
#include <QMainWindow>

#include "mandelbrotrenderer.h"
#include "renderthread.h"
#include "computeddatasegment.h"
#include "settingshandler.h"
#include "settingsuser.h"
#include "RendererConfig.h"
#include "RenderHistory.h"


class ToolsMenu;
class FileMenu;
class EditMenu;
class ParametersMenu;
class RenderMenu;
class WindowMenu;
class QRubberBand;

//! [0]
class MandelbrotWidget : public QMainWindow, public SettingsUser
{
    Q_OBJECT
public:
    MandelbrotWidget(QWidget *parent = nullptr);

    enum class logMode { normal, misc };

    void outputToLog(const QString& outputString, logMode outputMode, bool show = false);
    void executeRender(bool hideProgress = false);
    float getElapsedTimeDisplayed();

    const QPixmap& getPixmap() {return m_pixmap; }

    RendererConfig generateConfigData();
    void enforceConfigData(RendererConfig& newConfigData);

    QMutex* getMutex(std::size_t index) { return m_pauseMutexes[index]; }
    bool getUnsavedChangesExist() const { return m_unsavedChangesExist; }

    void setUnsavedChangesExist(bool value) { m_unsavedChangesExist = value; }

    MandelBrotRenderer::internalDataType getNumericType() const { return m_numericType; }

    QString computeDeltaWithHigherPrecision(QString& value, int deltaPixels, QString& currentScale);

    /*
     * Get the edge values of the permitted region for manually entered parameters
     */
    MandelBrotRenderer::RegionLimits getParameterSpace() const { return m_parameterSpace; }

public slots:
    InformationDisplay& getInfoDisplayer();
    void updateCoordInfo(const MandelBrotRenderer::CoordValue& originX, const MandelBrotRenderer::CoordValue& originY,
        const MandelBrotRenderer::CoordValue& scaleFactor);
    void displayThreadsInfo(int numThreads);
    void displayPassesInfo(int numPasses);
    void setIterationSumCount(qint64 iterationSum);
    void displayColorMapSizeInfo(int m_colorMapSize);
    void displayInternalDataType(const QString& description);
    void initiateQuit();
    void accomplishQuit();
    void initiateHalt();
    void initiateRefresh();
    void initiatePause();
    void handleRenderStarting();
    void handleAllDone();
    void progressMade(int computationChunksDone);
    void resetElapsedTime();
    void incrementElapsedTime();
    void resizeProgressBar();
    void restoreDefaults();
    void disableFileMenu();
    void enableFileMenu();
    void writeStatusMessage(const QString& message, bool isWarning = false, int timeout = 0);
    void writeTransientStatusMessage(const QString& message, 
        InformationDisplay::renderState renderState = InformationDisplay::renderState::idle, bool isWarning = false);
    void HandleDisplayDynamicTasksInfo(bool dynamicTasksEnabled);
    void UpdateIterationCount(qint64 iterationCount);

    void outputToLog(const QString& outputString, bool show = false);

    void pause();
    void release(bool forceFlow = false);
    void undo();
    void redo();
    void setUndoEnabled(bool enabled);
    void setRedoEnabled(bool enabled);
    void setUsingUndoRedo() { m_usingUndoRedo = true; }
    void clearUsingUndoRedo() { m_usingUndoRedo = false; }
    void registerCoordinateUser(MandelBrotRenderer::CoordinateListener* listener,
                                MandelBrotRenderer::CoordinateListenerConfig config);
    bool changeRegionParameters(const QString &originXvalue,
                                const QString &originYvalue,
                                const QString &widthValue,
                                const QString &heightValue);

signals:
    void quitAll();
    void halt();

private:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *event) override;
#endif
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void processSettingUpdate(QSettings& settings) override;
    bool confirmOperation(const QString& message, const QString& informative, bool suggestSave = false);

private slots:
    void updatePixmap(const QImage* image, QString scaleFactorString);
    void zoom(double zoomFactor);
    void enableOptions();
    void disableOptions();
    void showProgress();
    void hideProgress();
    void handleHalt();

private:
    void setInternalDataType(MandelBrotRenderer::internalDataType);
    void scroll(int deltaX, int deltaY);
    void adjustProgressBar();
    void writeSettings();
    void closeEvent(QCloseEvent* event) override;
    void zoomToSelectedRectangle(QMouseEvent *event);
    void displayIterationsPerPixel(qint64 iterationSum);
    void processWindowSize();
    void prepareProgressBar();

    template <typename T>
    bool validateRegion(T originX, T originY, T width, T height);

    QTextEdit* m_logger;
    QTextEdit* m_miscLogger;
    QVBoxLayout* m_mandelbrotWidgetLayout;
    QMutex m_mutex;
    QProgressBar* m_progressBar;
    ToolsMenu* m_toolsMenu;
    EditMenu* m_editMenu;
    FileMenu* m_fileMenu;
    ParametersMenu* m_parametersMenu;
    RenderMenu* m_renderMenu;
    WindowMenu* m_windowMenu;
    SettingsHandler m_settingsHandler;
    InformationDisplay m_infoDisplayer;

    QWidget* m_centralPlotArea;
    QStatusBar* m_messageBar;
    QRubberBand* m_newSelectionArea;
    QPoint m_newSelectionOrigin;

    RenderThread m_thread;
    QPixmap m_pixmap;
    QPoint m_pixmapOffset;
    QPoint m_lastDragPos;
    MandelBrotRenderer::CoordValue m_originX;
    MandelBrotRenderer::CoordValue m_originY;
    QString m_preciseOriginX;
    QString m_preciseOriginY;
    MandelBrotRenderer::CoordValue  m_pixmapScale;
    MandelBrotRenderer::CoordValue  m_curScale;
    double m_perPixelCoeff;
    bool m_scaleHasChanged;
    bool m_renderInProgress;
    bool m_usingUndoRedo;
    MandelBrotRenderer::internalDataType m_numericType;
    MandelBrotRenderer::ListenerGroup m_coordinateUsers;

    std::vector<QMutex*> m_pauseMutexes;
    std::atomic<bool> m_paused;

    QRecursiveMutex m_infoDisplayMutex;

    std::unique_ptr<RenderHistory> m_historyLog;
    bool m_unsavedChangesExist;

    const MandelBrotRenderer::RegionLimits m_parameterSpace;

    static const int kInitialWidth = 550;
    static const int kInitialHeight = 500;
    static const int kInitialPosCoord = 400;

    static QString m_DefaultOriginX;
    static QString m_DefaultOriginY;

    void connectToComponents();
    void prepareMenuBar();
    void prepareButtons();
    void setupLayout();
    void setupLoggers();

    void preparePauseLocks();

public:
    static QString s_undefinedFloatString;
    static QString s_unInitializedFloatString;
    static QString getDefaultOriginX() { return m_DefaultOriginX; }
    static QString getDefaultOriginY() { return m_DefaultOriginY; }
    static QString s_DefaultScaleFloatString;
};
//! [0]

#endif // MANDELBROTWIDGET_H
