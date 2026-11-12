#include "informationdisplay.h"
#include "renderthread.h"

#include "settingshandler.h"
#include "PrecisionHandler.h"
#include "mandelbrotwidget.h"

#if (USE_BOOST_MULTIPRECISION == 1 || defined(__GNUC__))
using MandelBrotRenderer::generateFloatFromString;
#endif

InformationDisplay::InformationDisplay(MandelbrotWidget& mainWidget,SettingsHandler& applicationSettingsHandler)
    : m_timeTakenLabel("Render Time (s):"), m_percentageDoneLabel("Progress: "), m_currentStateLabel("Current State: "),
                m_numThreadsLabel("Threads:"), m_numPassesLabel("Passes:"), m_dynamicTasksLabel("Dynamic Tasks: "),
                m_xLabel("X:") ,m_yLabel("Y:"),
                m_widthLabel("Width:"), m_heightLabel("Height:"), m_colorMapSizeLabel("ColourMap Size:"), m_numericTypeLabel("Internal Data Type"),
                m_pixmapSizeLabel("Image Size: "), m_iterationSumLabel("Iterations sum: "), m_iterationsPerPixelLabel("Iterations per pixel"),
                m_elapsedTime(std::optional<uint>{}),
                m_currentState(renderState::idle),
                m_inProgress(false),
                m_detailedDisplayEnabled(detailedDisplayEnabledByDefault), //initialize this here for clarity (will be changed by settings code)
                mainWidget(&mainWidget),
                applicationSettingsHandler(applicationSettingsHandler)
{
    setVisible(m_detailedDisplayEnabled);
    prepareRenderStateLabels();
    setRenderState(m_currentState);

    QSizePolicy policyRetainSpace = sizePolicy();
    policyRetainSpace.setRetainSizeWhenHidden(true);
    setSizePolicy(policyRetainSpace);

    m_informationDisplayLayout.setAlignment(Qt::AlignTop | Qt::AlignLeft);
    constexpr int spacingBetweenColumns = 1;
    m_informationDisplayLayout.setHorizontalSpacing(spacingBetweenColumns);

    int currentRow = 0;
    m_informationDisplayLayout.addWidget(&m_timeTakenLabel, currentRow, 0);
    m_informationDisplayLayout.addWidget(&m_timeTakenInfo, currentRow, 1);

    ++currentRow;
    m_informationDisplayLayout.addWidget(&m_percentageDoneLabel, currentRow, 0);
    m_informationDisplayLayout.addWidget(&m_percentageDone, currentRow, 1);
    m_informationDisplayLayout.addWidget(&m_currentStateLabel, currentRow, 2);
    m_informationDisplayLayout.addWidget(&m_currentStateDescription, currentRow, 3);

    ++currentRow;
    m_informationDisplayLayout.addWidget(&m_numThreadsLabel, currentRow, 0);
    m_informationDisplayLayout.addWidget(&m_numThreadsInfo, currentRow, 1);

    m_informationDisplayLayout.addWidget(&m_numPassesLabel, currentRow, 2);
    m_informationDisplayLayout.addWidget(&m_numPassesInfo, currentRow, 3);
    ++currentRow;

    m_informationDisplayLayout.addWidget(&m_dynamicTasksLabel, currentRow, 0);
    m_informationDisplayLayout.addWidget(&m_dynamicTasksInfo, currentRow, 1);

    ++currentRow;
    m_informationDisplayLayout.addWidget(&m_xLabel, currentRow, 0);
    m_informationDisplayLayout.addWidget(&m_leftBottomXInfo, currentRow, 1);
    m_informationDisplayLayout.addWidget(&m_yLabel, currentRow, 2);
    m_informationDisplayLayout.addWidget(&m_leftBottomYInfo, currentRow, 3);

    ++currentRow;
    m_informationDisplayLayout.addWidget(&m_widthLabel, currentRow, 0);
    m_informationDisplayLayout.addWidget(&m_widthInfo, currentRow, 1);
    m_informationDisplayLayout.addWidget(&m_heightLabel, currentRow, 2);
    m_informationDisplayLayout.addWidget(&m_heightInfo, currentRow, 3);

    ++currentRow;
    m_informationDisplayLayout.addWidget(&m_colorMapSizeLabel, currentRow, 0);
    m_informationDisplayLayout.addWidget(&m_colorMapSizeInfo, currentRow, 1);
    m_informationDisplayLayout.addWidget(&m_numericTypeLabel, currentRow, 2);
    m_informationDisplayLayout.addWidget(&m_numericTypeInfo, currentRow, 3);

    ++currentRow;
    m_informationDisplayLayout.addWidget(&m_pixmapSizeLabel, currentRow, 0);
    m_informationDisplayLayout.addWidget(&m_pixmapSizeInfo, currentRow, 1);

    ++currentRow;
    m_informationDisplayLayout.addWidget(&m_iterationSumLabel, currentRow, 0);
    m_informationDisplayLayout.addWidget(&m_iterationSumInfo, currentRow, 1);
    m_informationDisplayLayout.addWidget(&m_iterationsPerPixelLabel, currentRow, 2);
    m_informationDisplayLayout.addWidget(&m_iterationsPerPixelInfo, currentRow, 3);

    setLayout(&m_informationDisplayLayout);
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
    m_timeTakenInfo.setText("\n0.00");
    m_timeTakenInfo.setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_timeTakenInfo.show();
}

void InformationDisplay::setThreadsInfo(int numThreads)
{
    m_numThreadsInfo.setText(QString::number(numThreads));
}

void InformationDisplay::setPassesInfo(int numPasses)
{
    m_numPassesInfo.setText(QString::number(numPasses));
}

void InformationDisplay::setDynamicTasksInfo(bool dynamicTasksEnabled)
{
    m_dynamicTasksInfo.setText(MandelBrotRenderer::getBoolValueAsString(dynamicTasksEnabled));
}

void InformationDisplay::setColorMapSize(int m_colorMapSize)
{
    m_colorMapSizeInfo.setText(QString::number(m_colorMapSize));
}

void InformationDisplay::displayInternalDataType(QString description)
{
    int firstBracketPos = description.indexOf("(");
    if (firstBracketPos != -1) {
        description.truncate(firstBracketPos);
    }
    m_numericTypeInfo.setText(description);
}

void InformationDisplay::setIterationSumCount(qint64 iterationSum)
{
    QString result("-");

    if (iterationSum >= 0) {
        result = QString::number(iterationSum);
    }

    m_iterationSumInfo.setText(result);
}

void InformationDisplay::setIterationsPerPixel(double iterationsPerPixel)
{
    QString result = iterationsPerPixel >= 0 ? QString::number(iterationsPerPixel) : "-";
    m_iterationsPerPixelInfo.setText(result);
}

void InformationDisplay::updateCoordData(const QString& originX, const QString& originY,
                     const QString& width, const QString& height)
{
    if (!originX.isEmpty()) {
        m_leftBottomXInfo.setText(originX);
    }
    if (!originY.isEmpty()) {
        m_leftBottomYInfo.setText(originY);
    }
    m_widthInfo.setText(width);
    m_heightInfo.setText(height);
}

void InformationDisplay::updateProgress(uint progressInPercent)
{
    if (m_inProgress)
    {
        m_percentageDone.setText(QString::number(progressInPercent) + "%");
    }
}

void InformationDisplay::resetElapsedTimeInfo()
{
    if (!m_TimeReset &&
        !m_elapsedTime.load().has_value() ||
        m_elapsedTime.load().value() != 0)
    {
        m_TimeReset = true;
        m_elapsedTime.store(0);
    }
}

void InformationDisplay::incrementTimeInfo()
{
    uint value = m_elapsedTime.load().value_or(0);
    m_TimeReset = false;
    m_elapsedTime.store(++value);
    m_timeTakenInfo.setText(QString::number(static_cast<double>(m_elapsedTime.load().value()) / TIMER_TICKS_PER_SECOND));
}

float InformationDisplay::getElapsedTimeDisplayed() const
{
    const uint value = m_elapsedTime.load().value_or(0);
    return static_cast<float>(value) / TIMER_TICKS_PER_SECOND;
}

void InformationDisplay::setEnabledByButtonState(Qt::CheckState buttonState)
{
    m_detailedDisplayEnabled = shouldBeEnabled(buttonState);
    setEnabled(m_detailedDisplayEnabled);
}

void InformationDisplay::setEnabled(bool newEnabledState)
{
    setVisible(newEnabledState);
}


void InformationDisplay::writeSettings()
{
    applicationSettingsHandler.getSettings().beginGroup("InformationDisplay");

    applicationSettingsHandler.getSettings().setValue("detailedDisplayEnabled", m_detailedDisplayEnabled);

    applicationSettingsHandler.getSettings().endGroup();
}

void InformationDisplay::updatePixmapSizeInfo(QSize size)
{
    m_pixmapSizeInfo.setText(QString::number(size.width()) + " x " + QString::number(size.height()));
}

bool InformationDisplay::shouldBeEnabled(Qt::CheckState buttonState)
{
    return (buttonState == Qt::Checked);
}

bool InformationDisplay::getDetailedDisplayEnabled() const
{
    return m_detailedDisplayEnabled;
}

void InformationDisplay::processSettingUpdate(QSettings &settings)
{
    settings.beginGroup("InformationDisplay");

    m_detailedDisplayEnabled = settings.value("detailedDisplayEnabled", detailedDisplayEnabledByDefault).toBool();
    setVisible(m_detailedDisplayEnabled);

    settings.endGroup();
}

void InformationDisplay::setRenderState(renderState newState)
{
    m_currentState = newState;
    auto result = renderStateDescriptions.find(newState);

    if (result == renderStateDescriptions.end())
    {
        result = renderStateDescriptions.find(renderState::unknown);
        Q_ASSERT(result != renderStateDescriptions.end());
    }
    m_currentStateDescription.setText((*result).second);
}
