#include "windowthreadinfo.h"
#include "threadiconmap.h"
#include "settingshandler.h"

#include <QGridLayout>
#include <QSizePolicy>
#include <QCloseEvent>
#include <QSettings>

WindowThreadInfo::WindowThreadInfo(RenderThread* masterThread, int numberOfThreads, const ThreadIconMap* threadIcons) :
    QGroupBox("Thread Information"), masterThread(masterThread),displayedNumberOfThreads(numberOfThreads),
    settingsHandler(masterThread->getApplicationSettings()),
    threadIcons(threadIcons)
{
    this->setStyleSheet("QGroupBox { font-weight: bold; } ");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
    masterThread->setThreadInfoDisplayer(this);
    threadInfoLayout = new QGridLayout();
    initialiseIndicators();

    settingsHandler.registerSettingsUser(this);
}

void WindowThreadInfo::configureThreadInfo(int numberOfThreads, threadState currentState)
{
    displayedNumberOfThreads = numberOfThreads;
    for (uint i = 0; i < MAX_NUM_WORKER_THREADS; ++i) {
        setThreadState(i, i < static_cast<size_t>(numberOfThreads) ? currentState : threadState::disabled);
    }
}

void WindowThreadInfo::setThreadState(uint threadIndex, threadState state)
{
    setThreadState(threadIndex, toUnderlyingType(state));
}

void WindowThreadInfo::writeSettings()
{
    SettingsHandler& settingsHandler = masterThread->getApplicationSettings();

    settingsHandler.getSettings().beginGroup("WindowThreadInfo");
    settingsHandler.getSettings().setValue("size", size());
    settingsHandler.getSettings().setValue("pos", pos());
    settingsHandler.getSettings().setValue("visible", isVisible());
    settingsHandler.getSettings().endGroup();

}

void WindowThreadInfo::processSettingUpdate(QSettings &settings)
{
    settings.beginGroup("WindowThreadInfo");
    resize(settings.value("size", QSize(initialWidth, initialHeight)).toSize());
    move(settings.value("pos", QPoint(initialCoordPos, initialCoordPos)).toPoint());
    setVisible(settings.value("visible", false).toBool());
    settings.endGroup();
}

void WindowThreadInfo::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}


void WindowThreadInfo::setThreadState(uint threadIndex, int state)
{
    QMutexLocker locker(&mutex);
    Q_ASSERT(static_cast<size_t>(threadIndex) < MAX_NUM_WORKER_THREADS);
    threadStateIndicators.at(static_cast<size_t>(threadIndex))->setPixmap(threadIcons->getThreadBitmaps().at(static_cast<size_t>(state)));
}


void WindowThreadInfo::initialiseIndicators()
{
    int currentRow(0);
    const Qt::Alignment alignment = Qt::AlignHCenter;

    QLabel* threadLabel = new QLabel("Thread");
    QLabel* stateLabel = new QLabel("State");
    threadInfoLayout->addWidget(threadLabel, currentRow, 0, alignment);
    threadInfoLayout->addWidget(stateLabel, currentRow, 1, alignment);
    ++currentRow;

    for (auto& indicator : threadStateIndicators) {
            QLabel* aStateText (new QLabel());
            aStateText->setText(QString::number(currentRow++));
            QLabel* aStateIndicator (new QLabel());
            aStateIndicator->setPixmap(threadIcons->getThreadBitmaps().at(toUnderlyingType(threadState::idle)));

            aStateIndicator->setFrameStyle(QFrame::Panel | QFrame::Sunken);
            aStateIndicator->setLineWidth(ThreadIndicatorFrameWidth);

            indicator = aStateIndicator;
            threadInfoLayout->addWidget(aStateText, currentRow, 0, alignment);
            threadInfoLayout->addWidget(indicator, currentRow, 1, alignment);
    }
    threadInfoLayout->setSizeConstraint(QLayout::SetMaximumSize);

    threadInfoLayout->setColumnMinimumWidth(0, colMinWidth);
    threadInfoLayout->setColumnMinimumWidth(1, colMinWidth);
    setLayout(threadInfoLayout);
}
