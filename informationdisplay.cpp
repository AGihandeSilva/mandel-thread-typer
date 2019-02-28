#include "informationdisplay.h"
#include "renderthread.h"

#include "settingshandler.h"

InformationDisplay::InformationDisplay(SettingsHandler& applicationSettingsHandler)
    : timeTakenLabel("Render Time (s):"), percentageDoneLabel("Progress: "), currentStateLabel("Current State: "),
                numThreadsLabel("Threads:"), numPassesLabel("Passes:"), dynamicTasksLabel("Dynamic Tasks: "),
                xLabel("X:") ,yLabel("Y:"),
                widthLabel("Width:"), heightLabel("Height:"), colorMapSizeLabel("ColourMap Size:"), numericTypeLabel("Internal Data Type"),
                pixmapSizeLabel("Image Size: "), iterationSumLabel("Iterations sum: "), iterationsPerPixelLabel("Iterations per pixel"),
                elapsedTime(0),
                currentState(renderState::idle),
                inProgress(false),
                detailedDisplayEnabled(detailedDisplayEnabledByDefault), //initialize this here for clarity (will be changed by settings code)
                applicationSettingsHandler(applicationSettingsHandler)
{
    setVisible(detailedDisplayEnabled);
    prepareRenderStateLabels();
    setRenderState(currentState);

    QSizePolicy policyRetainSpace = sizePolicy();
    policyRetainSpace.setRetainSizeWhenHidden(true);
    setSizePolicy(policyRetainSpace);

    informationDisplayLayout.setAlignment(Qt::AlignTop | Qt::AlignLeft);
    constexpr int spacingBetweenColumns = 1;
    informationDisplayLayout.setHorizontalSpacing(spacingBetweenColumns);

    int currentRow = 0;
    informationDisplayLayout.addWidget(&timeTakenLabel, currentRow, 0);
    informationDisplayLayout.addWidget(&timeTakenInfo, currentRow, 1);

    ++currentRow;
    informationDisplayLayout.addWidget(&percentageDoneLabel, currentRow, 0);
    informationDisplayLayout.addWidget(&percentageDone, currentRow, 1);
    informationDisplayLayout.addWidget(&currentStateLabel, currentRow, 2);
    informationDisplayLayout.addWidget(&currentStateDescription, currentRow, 3);

    ++currentRow;
    informationDisplayLayout.addWidget(&numThreadsLabel, currentRow, 0);
    informationDisplayLayout.addWidget(&numThreadsInfo, currentRow, 1);

    informationDisplayLayout.addWidget(&numPassesLabel, currentRow, 2);
    informationDisplayLayout.addWidget(&numPassesInfo, currentRow, 3);
    ++currentRow;

    informationDisplayLayout.addWidget(&dynamicTasksLabel, currentRow, 0);
    informationDisplayLayout.addWidget(&dynamicTasksInfo, currentRow, 1);

    ++currentRow;
    informationDisplayLayout.addWidget(&xLabel, currentRow, 0);
    informationDisplayLayout.addWidget(&leftBottomXInfo, currentRow, 1);
    informationDisplayLayout.addWidget(&yLabel, currentRow, 2);
    informationDisplayLayout.addWidget(&leftBottomYInfo, currentRow, 3);

    ++currentRow;
    informationDisplayLayout.addWidget(&widthLabel, currentRow, 0);
    informationDisplayLayout.addWidget(&widthInfo, currentRow, 1);
    informationDisplayLayout.addWidget(&heightLabel, currentRow, 2);
    informationDisplayLayout.addWidget(&heightInfo, currentRow, 3);

    ++currentRow;
    informationDisplayLayout.addWidget(&colorMapSizeLabel, currentRow, 0);
    informationDisplayLayout.addWidget(&colorMapSizeInfo, currentRow, 1);
    informationDisplayLayout.addWidget(&numericTypeLabel, currentRow, 2);
    informationDisplayLayout.addWidget(&numericTypeInfo, currentRow, 3);

    ++currentRow;
    informationDisplayLayout.addWidget(&pixmapSizeLabel, currentRow, 0);
    informationDisplayLayout.addWidget(&pixmapSizeInfo, currentRow, 1);

    ++currentRow;
    informationDisplayLayout.addWidget(&iterationSumLabel, currentRow, 0);
    informationDisplayLayout.addWidget(&iterationSumInfo, currentRow, 1);
    informationDisplayLayout.addWidget(&iterationsPerPixelLabel, currentRow, 2);
    informationDisplayLayout.addWidget(&iterationsPerPixelInfo, currentRow, 3);

    setLayout(&informationDisplayLayout);
    applicationSettingsHandler.registerSettingsUser(this);
}

void InformationDisplay::prepareRenderStateLabels()
{
    renderStateDescriptions.insert(std::make_pair(renderState::idle, "Idle"));
    renderStateDescriptions.insert(std::make_pair(renderState::running, "Running"));
    renderStateDescriptions.insert(std::make_pair(renderState::paused, "Paused"));
    renderStateDescriptions.insert(std::make_pair(renderState::aborted, "Terminated"));
    renderStateDescriptions.insert(std::make_pair(renderState::unknown, "Unknown"));
}

void InformationDisplay::prepareTimerInfo()
{
    timeTakenInfo.setText("\n0.00");
    timeTakenInfo.setAlignment(Qt::AlignLeft | Qt::AlignTop);
    timeTakenInfo.show();
}

void InformationDisplay::setThreadsInfo(int numThreads)
{
    numThreadsInfo.setText(QString::number(numThreads));
}

void InformationDisplay::setPassesInfo(int numPasses)
{
    numPassesInfo.setText(QString::number(numPasses));
}

void InformationDisplay::setDynamicTasksInfo(bool dynamicTasksEnabled)
{
    dynamicTasksInfo.setText(MandelBrotRenderer::getBoolValueAsString(dynamicTasksEnabled));
}

void InformationDisplay::setColorMapSize(int colorMapSize)
{
    colorMapSizeInfo.setText(QString::number(colorMapSize));
}

void InformationDisplay::setInternalDataType(QString description)
{
    int firstBracketPos = description.indexOf("(");
    if (firstBracketPos != -1) {
        description.truncate(firstBracketPos);
    }
    numericTypeInfo.setText(description);
}

void InformationDisplay::setIterationSumCount(int64_t iterationSum)
{
    QString result("-");

    if (iterationSum >= 0) {
        result = QString::number(iterationSum);
    }

    iterationSumInfo.setText(result);
}

void InformationDisplay::setIterationsPerPixel(double iterationsPerPixel)
{
    QString result = iterationsPerPixel >= 0 ? QString::number(iterationsPerPixel) : "-";
    iterationsPerPixelInfo.setText(result);
}

void InformationDisplay::updateCoordInfo(double centerX, double centerY, double scaleFactor,
                     QSize size)
{
    leftBottomXInfo.setText(QString::number(centerX));
    leftBottomYInfo.setText(QString::number(centerY));
    widthInfo.setText(QString::number(size.width() * scaleFactor));
    heightInfo.setText(QString::number(size.height() * scaleFactor));
}

void InformationDisplay::updateProgress(uint progressInPercent)
{
    if (inProgress)
    {
        percentageDone.setText(QString::number(progressInPercent) + "%");
    }
}

void InformationDisplay::resetElapsedTimeInfo()
{
    elapsedTime = 0;
    timeTakenInfo.setText(QString::number(0));
}

void InformationDisplay::incrementTimeInfo()
{
    ++elapsedTime;
    timeTakenInfo.setText(QString::number(static_cast<double>(elapsedTime) / TIMER_TICKS_PER_SECOND ));
}

float InformationDisplay::getElapsedTimeDisplayed() const
{
    return static_cast<float>(elapsedTime) / TIMER_TICKS_PER_SECOND;
}

void InformationDisplay::setEnabledByButtonState(int buttonState)
{
    detailedDisplayEnabled = shouldBeEnabled(buttonState);
    setEnabled(detailedDisplayEnabled);
}

void InformationDisplay::setEnabled(bool newEnabledState)
{
    setVisible(newEnabledState);
}


void InformationDisplay::writeSettings()
{
    applicationSettingsHandler.getSettings().beginGroup("InformationDisplay");

    applicationSettingsHandler.getSettings().setValue("detailedDisplayEnabled", detailedDisplayEnabled);

    applicationSettingsHandler.getSettings().endGroup();
}

void InformationDisplay::updatePixmapSizeInfo(QSize size)
{
    pixmapSizeInfo.setText(QString::number(size.width()) + " x " + QString::number(size.height()));
}

bool InformationDisplay::shouldBeEnabled(int buttonState)
{
    return (static_cast<Qt::CheckState>(buttonState) == Qt::Checked);
}

bool InformationDisplay::getDetailedDisplayEnabled() const
{
    return detailedDisplayEnabled;
}

void InformationDisplay::processSettingUpdate(QSettings &settings)
{
    settings.beginGroup("InformationDisplay");

    detailedDisplayEnabled = settings.value("detailedDisplayEnabled", detailedDisplayEnabledByDefault).toBool();
    setVisible(detailedDisplayEnabled);

    settings.endGroup();
}

void InformationDisplay::setRenderState(renderState newState)
{
    currentState = newState;
    auto result = renderStateDescriptions.find(newState);

    if (result == renderStateDescriptions.end())
    {
        result = renderStateDescriptions.find(renderState::unknown);
        Q_ASSERT(result != renderStateDescriptions.end());
    }
    currentStateDescription.setText((*result).second);
}
