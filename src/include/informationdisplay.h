#ifndef INFORMATIONDISPLAY_H
#define INFORMATIONDISPLAY_H

#include <QLabel>
#include <QWidget>
#include <QGridLayout>
#include <QSize>

#include "settingsuser.h"
#include "mandelbrotrenderer.h"

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
    void setColorMapSize(int colorMapSize);
    void displayInternalDataType(QString description);
    void setIterationSumCount(int64_t iterationSum);
    void setIterationsPerPixel(double iterationsPerPixel);
    void resetElapsedTimeInfo();
    void incrementTimeInfo();
    static constexpr uint getRequiredTimerTicksPerSecond() { return TIMER_TICKS_PER_SECOND; }
    float getElapsedTimeDisplayed() const;

    bool getDetailedDisplayEnabled() const;

    void processSettingUpdate(QSettings& settings) override;

    enum class renderState { idle = 0, running = 1, paused = 2, aborted = 3, unknown = 4 };

    void setRenderState(renderState newState);

public slots:
    void stopProgress() { inProgress = false; }
    void startProgress() { inProgress = true; }
    void setEnabledByButtonState(int buttonState);
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
    static bool shouldBeEnabled(int buttonState);

    QGridLayout informationDisplayLayout;

    InformationLabel timeTakenLabel;
    InformationLabel timeTakenInfo;

    InformationLabel percentageDoneLabel;
    InformationLabel percentageDone;

    InformationLabel currentStateLabel;
    InformationLabel currentStateDescription;

    InformationLabel numThreadsLabel;
    InformationLabel numThreadsInfo;

    InformationLabel numPassesLabel;
    InformationLabel numPassesInfo;

    InformationLabel dynamicTasksLabel;
    InformationLabel dynamicTasksInfo;

    InformationLabel xLabel;
    InformationLabel yLabel;
    InformationLabel widthLabel;
    InformationLabel heightLabel;

    InformationLabel leftBottomXInfo;
    InformationLabel leftBottomYInfo;
    InformationLabel widthInfo;
    InformationLabel heightInfo;

    InformationLabel colorMapSizeLabel;
    InformationLabel colorMapSizeInfo;

    InformationLabel numericTypeLabel;
    InformationLabel numericTypeInfo;

    InformationLabel pixmapSizeLabel;
    InformationLabel pixmapSizeInfo;

    InformationLabel iterationSumLabel;
    InformationLabel iterationSumInfo;

    InformationLabel iterationsPerPixelLabel;
    InformationLabel iterationsPerPixelInfo;

    uint elapsedTime;

    renderState currentState;

    bool inProgress;
    bool detailedDisplayEnabled;

    static constexpr uint TIMER_TICKS_PER_SECOND = 100;
    static constexpr bool detailedDisplayEnabledByDefault = true;
    MandelbrotWidget* mainWidget;
    SettingsHandler& applicationSettingsHandler;

    std::map<renderState, QString> renderStateDescriptions;

    static constexpr int InformationDisplayPrecision = 6;

};

#endif // INFORMATIONDISPLAY_H
