#ifndef INFORMATIONDISPLAY_H
#define INFORMATIONDISPLAY_H

#include <QLabel>
#include <QWidget>
#include <QGridLayout>
#include <QSize>

#include "settingsuser.h"
#include "mandelbrotrenderer.h"
#include <optional>

class SettingsHandler;
class MandelbrotWidget;


class InformationDisplay : public QWidget, public SettingsUser, public MandelBrotRenderer::CoordinateListener
{
    Q_OBJECT
public:
    explicit InformationDisplay(MandelbrotWidget& mainWidget, SettingsHandler& applicationSettingsHandler);
    void prepareTimerInfo();

    void updateCoordData(const QString& originX, const QString& originY,
                         const QString& width, const QString& height) override;
    void updateProgress(uint progressInPercent);
    void setThreadsInfo(int numThreads);
    void setPassesInfo(int numPasses);
    void setDynamicTasksInfo(bool dynamicTasksEnabled);
    void setColorMapSize(int m_colorMapSize);
    void displayInternalDataType(QString description);
    void setIterationSumCount(qint64 iterationSum);
    void setIterationsPerPixel(double iterationsPerPixel);
    void resetElapsedTimeInfo();
    void incrementTimeInfo();
    static constexpr uint getRequiredTimerTicksPerSecond() { return TIMER_TICKS_PER_SECOND; }
    float getElapsedTimeDisplayed() const;

    bool getDetailedDisplayEnabled() const;

    void processSettingUpdate(QSettings& settings) override;

    enum class renderState : int { idle = 0, running = 1, paused = 2, aborted = 3, unknown = 4 };

    Q_ENUM(renderState);

    void setRenderState(renderState newState);

public slots:
    void stopProgress() { m_inProgress = false; }
    void startProgress() { m_inProgress = true; }
    void setEnabledByButtonState(Qt::CheckState buttonState);
    void setEnabled(bool newEnabledState);
    void writeSettings();
    void updatePixmapSizeInfo(QSize size);

private:

    class InformationLabel : public QLabel
    {
public:
        InformationLabel()
            : InformationLabel("", "darkBlue", "rgba(128, 128, 180, 0.8)", "10px", "AlignCenter") {}

        explicit InformationLabel(const QString& text, const QString& colour = "white",
                                  const QString& backgroundColour = "rgba(0,0,0, 0.4)",
                                  const QString& radius = "5px",
                                  const QString& alignment = "AlignLeft")
        : QLabel(text)
        {
            setStyleSheet("QLabel {color : " + colour + "; " \
                          "border-radius:" + radius +"; background-color:" +
                          backgroundColour + " ; padding: 2px; qproperty-alignment: " + alignment + ";}");
        }
    };
private:
    void prepareRenderStateLabels();
    static bool shouldBeEnabled(Qt::CheckState buttonState);

    QGridLayout m_informationDisplayLayout;

    InformationLabel m_timeTakenLabel;
    InformationLabel m_timeTakenInfo;

    InformationLabel m_percentageDoneLabel;
    InformationLabel m_percentageDone;

    InformationLabel m_currentStateLabel;
    InformationLabel m_currentStateDescription;

    InformationLabel m_numThreadsLabel;
    InformationLabel m_numThreadsInfo;

    InformationLabel m_numPassesLabel;
    InformationLabel m_numPassesInfo;

    InformationLabel m_dynamicTasksLabel;
    InformationLabel m_dynamicTasksInfo;

    InformationLabel m_xLabel;
    InformationLabel m_yLabel;
    InformationLabel m_widthLabel;
    InformationLabel m_heightLabel;

    InformationLabel m_leftBottomXInfo;
    InformationLabel m_leftBottomYInfo;
    InformationLabel m_widthInfo;
    InformationLabel m_heightInfo;

    InformationLabel m_colorMapSizeLabel;
    InformationLabel m_colorMapSizeInfo;

    InformationLabel m_numericTypeLabel;
    InformationLabel m_numericTypeInfo;

    InformationLabel m_pixmapSizeLabel;
    InformationLabel m_pixmapSizeInfo;

    InformationLabel m_iterationSumLabel;
    InformationLabel m_iterationSumInfo;

    InformationLabel m_iterationsPerPixelLabel;
    InformationLabel m_iterationsPerPixelInfo;

    std::atomic<std::optional<uint>> m_elapsedTime;
    std::atomic<bool> m_TimeReset = false;

    renderState m_currentState;

    bool m_inProgress;
    bool m_detailedDisplayEnabled;

    static constexpr uint TIMER_TICKS_PER_SECOND = 100;
    static constexpr bool detailedDisplayEnabledByDefault = true;
    MandelbrotWidget* mainWidget;
    SettingsHandler& applicationSettingsHandler;

    std::map<renderState, QString> renderStateDescriptions;

    static constexpr int InformationDisplayPrecision = 6;

};

#endif // INFORMATIONDISPLAY_H
