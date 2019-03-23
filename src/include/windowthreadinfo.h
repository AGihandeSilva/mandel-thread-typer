#ifndef THREADINFOWIDGET_H
#define THREADINFOWIDGET_H

#include <QGroupBox>
#include <QMutex>

#include "renderthread.h"

#include <array>

class QPixmap;
class QGridLayout;
class RenderThread;
class ThreadIconMap;
class SettingsHandler;

class WindowThreadInfo : public QGroupBox, public SettingsUser
{
    Q_OBJECT
public:
    explicit WindowThreadInfo(RenderThread* masterThread, int numberOfThreads, const ThreadIconMap* threadIcons);

    void writeSettings();

    void processSettingUpdate(QSettings& settings) override;

public slots:
    void configureThreadInfo(int numberOfThreads, MandelBrotRenderer::threadState currentState = MandelBrotRenderer::threadState::idle);
    void setThreadState(uint threadIndex, int state);

private:
    void initialiseIndicators();
    void setThreadState(uint threadIndex, MandelBrotRenderer::threadState state);
    void closeEvent(QCloseEvent* event) override;

    RenderThread* masterThread;
    QGridLayout* threadInfoLayout;
    int displayedNumberOfThreads;

    SettingsHandler& settingsHandler;

    QMutex mutex;

    static constexpr int colMinWidth = 80;

    static constexpr int initialWidth = 210;
    static constexpr int initialHeight = 360;
    static constexpr int initialCoordPos = 200;

    const ThreadIconMap* const threadIcons;

    std::array<QLabel*, MandelBrotRenderer::MAX_NUM_WORKER_THREADS> threadStateIndicators {};
};

#endif // THREADINFOWIDGET_H
