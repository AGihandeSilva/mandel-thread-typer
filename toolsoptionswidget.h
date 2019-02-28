#ifndef TOOLSOPTIONSWIDGET_H
#define TOOLSOPTIONSWIDGET_H

#include <QDialog>

#include "settingsuser.h"
#include "mandelbrotrenderer.h"
#include "renderthread.h"

class QSlider;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QButtonGroup;
class QCheckBox;
class QDialogButtonBox;
class QSpinBox;
class QComboBox;
class MandelbrotWidget;
class SettingsHandler;

class ToolsOptionsWidget : public QDialog, public SettingsUser
{
public:
    explicit ToolsOptionsWidget(RenderThread *masterThread, MandelbrotWidget* mainWidget, SettingsHandler& settingsHandler);

    void refresh();

public slots:
    void processSettingUpdate(QSettings& settings) override;
    void updateAndShow();
    void enforceChosenDataType(int index);
    RenderThread::typeNameUser AddNumericTypeToSelector() const;

private:
    QButtonGroup* addNumPassesArea();
    QButtonGroup* addthreadAlgorithmArea();
    void addHorizontalLine();
    void addThreadSlider();
    void createThreadSlider();
    void addColorMapSizeField();
    void addNumericTypeSelector();
    void addInfoControlButton();
    void updateFromSettings();

    void initializeChosenDataType();

signals:
    void numericTypeChosen(int index);

    QVBoxLayout* toolsOptionsLayout;
    QLabel* sliderTitle;
    QSlider* threadCountSlider;
    QLabel* numPassesTitle;
    QLabel* threadAlgorithmTitle;
    QButtonGroup* numPassesButtons;
    QHBoxLayout* numPassesLayout;
    QButtonGroup* threadAlgorithmButtons;
    QHBoxLayout* threadAlgorithmLayout;
    QLabel* colorMapTitle;
    QSpinBox* colorMapSizeSetting;
    QLabel* numericTypeTitle;
    QComboBox* numericTypeSelection;
    QCheckBox* showInfoButton;
    QDialogButtonBox* okOrCancelBox;
    RenderThread *masterThread;
    MandelbrotWidget* mainWidget;

    SettingsHandler& applicationSettingsHandler;

    enum class threadAlgorithm { uniform_algorithm = 0, dynamic_algorithm =  1};

    int numPassValue;
    int numWorkerThreads;
    bool threadMediatorEnabled;
    int colorMapSize;
    bool displayDetailedInfo;

    static constexpr int UNSELECTED_BUTTON = -1;
    static constexpr int NUM_THREAD_ALGORITHMS = 2;
    static constexpr bool dynamicTaskAllocationSupported = true;
    static const threadAlgorithm defaultAlgorithm = threadAlgorithm::uniform_algorithm;

    void setNumWorkerThreadsInGUI();
    void setAlgorithmInGUI();
    void setNumPassesInGUI();
    void setColorMapSizeinGUI();
    void setDetailedInfoInGUI();
    void setNumericTypeInGUI();

    int findSelectedNumPassesButton();
    void setSelectedNumPassButton(int buttonIndexToSelect, QButtonGroup *group = nullptr);

};

#endif // TOOLSOPTIONSWIDGET_H
