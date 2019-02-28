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

#include "renderthread.h"
#include "computeddatasegment.h"
#include "settingshandler.h"
#include "settingsuser.h"
#include "RendererConfig.h"
#include "RenderHistory.h"

class ToolsMenu;
class FileMenu;
class EditMenu;
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
    void updateCoordInfo(double centerX, double centerY, double scaleFactor,
                         QSize resultSize);
    void setThreadsInfo(int numThreads);
    void setPassesInfo(int numPasses);
    void setDynamicTasksInfo(bool dynamicTasksEnabled);
    void setIterationSumCount(int64_t iterationSum);
    float getElapsedTimeDisplayed() const;

    InformationDisplay *getInfoDisplayer() const;
    const QPixmap& getPixmap() {return pixmap; }

    RendererConfig generateConfigData();
    void enforceConfigData(RendererConfig& newConfigData);

    QMutex* getMutex(std::size_t index) { return pauseMutexes[index]; }
    bool getUnsavedChangesExist() const;

    void setUnsavedChangesExist(bool value);

    MandelBrotRenderer::internalDataType getNumericType() const;

public slots:
    void setColorMapSizeInfo(int colorMapSize);
    void setInternalDataType(const QString& description);
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
    void writeTransientStatusMessage(const QString& message, bool isWarning = false);

    void outputToLog(const QString& outputString, bool show = false);

    void pause();
    void release(bool forceFlow = false);
    void undo();
    void redo();
    void setUndoEnabled(bool enabled);
    void setRedoEnabled(bool enabled);
    void setUsingUndoRedo() { usingUndoRedo = true; }

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
    void updatePixmap(const QImage *image, double scaleFactor);
    void zoom(double zoomFactor);
    void enableOptions();
    void disableOptions();
    void showProgress();
    void hideProgress();

private:
    void scroll(int deltaX, int deltaY);
    void updateView();
    void adjustProgressBar();
    void writeSettings();
    void closeEvent(QCloseEvent* event) override;
    void zoomToSelectedRectangle(QMouseEvent *event);
    void setIterationsPerPixel(int64_t iterationSum);

    InformationDisplay* infoDisplayer;
    QTextEdit* logger;
    QTextEdit* miscLogger;
    QVBoxLayout* mandelbrotWidgetLayout;
    QMutex mutex;
    QProgressBar* progressBar;
    ToolsMenu* toolsMenu;
    EditMenu* editMenu;
    FileMenu* fileMenu;
    RenderMenu* renderMenu;
    WindowMenu* windowMenu;
    SettingsHandler settingsHandler;

    QWidget* centralPlotArea;
    QStatusBar* messageBar;
    QRubberBand* newSelectionArea;
    QPoint newSelectionOrigin;

    RenderThread thread;
    QPixmap pixmap;
    QPoint pixmapOffset;
    QPoint lastDragPos;
    double centerX;
    double centerY;
    double pixmapScale;
    double curScale;
    double perPixelCoeff;
    void prepareProgressBar();
    bool scaleHasChanged;
    bool renderInProgress;
    bool usingUndoRedo;
    MandelBrotRenderer::internalDataType  numericType;

    std::vector<QMutex*> pauseMutexes;
    std::atomic<bool> paused;

    std::unique_ptr<RenderHistory> historyLog;
    bool unsavedChangesExist;

    static const int kInitialWidth = 550;
    static const int kInitialHeight = 500;
    static const int kInitialPosCoord = 400;
    void processWindowSize();
};
//! [0]

#endif // MANDELBROTWIDGET_H
