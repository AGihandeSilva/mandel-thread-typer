#include <QSlider>
#include <QLabel>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include <QString>
#include <QFrame>
#include <QList>
#include <QComboBox>
#include <QStandardItemModel>

#include <exception>
#include <iostream>
#include <map>

#include "toolsoptionswidget.h"
#include "renderthread.h"
#include "radiointegerbutton.h"
#include "renderthreadmediator.h"
#include "mandelbrotwidget.h"
#include "settingshandler.h"
#include "MandelbrotGuiTools.h"


using MandelBrotRenderer::addHorizontalLine;


ToolsOptionsWidget::ToolsOptionsWidget(RenderThread *masterThread, MandelbrotWidget* mainWidget, SettingsHandler& settingsHandler)
    : sliderTitle(nullptr), threadCountSlider(nullptr), numPassesTitle(nullptr), threadAlgorithmTitle(nullptr),
      colorMapTitle(nullptr), numericTypeTitle(nullptr), showInfoButton(nullptr), okOrCancelBox(nullptr),
      masterThread(masterThread), mainWidget(mainWidget),
      applicationSettingsHandler(settingsHandler),
      numPassValue(masterThread != nullptr ? masterThread->getRunningNumPasses() : MandelBrotRenderer::defaultNumPassesValue),
      numWorkerThreads(masterThread != nullptr ? masterThread->getNumWorkerThreads() : RenderThread::calculateInitialNumThreads()),
      threadMediatorEnabled(false),
      colorMapSize(MandelBrotRenderer::DefaultColormapSize),
      displayDetailedInfo(true)
{
    processSettingUpdate(settingsHandler.getSettings());
    setWindowTitle("Options");

    createThreadSlider();

    toolsOptionsLayout =  new QVBoxLayout;

    this->setLayout(toolsOptionsLayout);

    addThreadSlider();

    addHorizontalLine(this, toolsOptionsLayout);

    numPassesLayout =  new QHBoxLayout;

    numPassesButtons = addNumPassesArea();

    addHorizontalLine(this, toolsOptionsLayout);

    threadAlgorithmLayout = new QHBoxLayout;

    threadAlgorithmButtons = addthreadAlgorithmArea();

    addHorizontalLine(this, toolsOptionsLayout);

    colorMapSizeSetting = new QSpinBox;

    addColorMapSizeField();

    addHorizontalLine(this, toolsOptionsLayout);

    numericTypeSelection = new QComboBox;

    addNumericTypeSelector();

    addHorizontalLine(this, toolsOptionsLayout);

    addInfoControlButton();

    toolsOptionsLayout->setSizeConstraint(QLayout::SetFixedSize);

    updateFromSettings();

    applicationSettingsHandler.registerSettingsUser(this);
}

void ToolsOptionsWidget::processSettingUpdate(QSettings& settings)
{
    settings.beginGroup("Renderer");

    numWorkerThreads = settings.value("numWorkerThreads", masterThread->getNumWorkerThreads()).toInt();
    numPassValue = settings.value("currentNumPassValue", masterThread->getRunningNumPasses()).toInt();

    threadMediatorEnabled = settings.value("threadMediatorEnabled", masterThread->getThreadMediator().getEnabled()).toBool();

    colorMapSize = settings.value("colourMapSize", MandelBrotRenderer::DefaultColormapSize).toInt();

    settings.endGroup();

    settings.beginGroup("InformationDisplay");
    displayDetailedInfo = settings.value("detailedDisplayEnabled", true).toBool();
    settings.endGroup();
}

void ToolsOptionsWidget::updateAndShow()
{
    updateFromSettings();
    raise();
    show();
}

int ToolsOptionsWidget::findSelectedNumPassesButton()
{
    const MandelBrotRenderer::MQintVector& passesOptions = masterThread->getPossiblePassValues();

    int selectedButtonIndex = UNSELECTED_BUTTON;
    int buttonIndex = 0;

    for (int passes: passesOptions)
    {
        if (passes == numPassValue) {
            selectedButtonIndex = buttonIndex;
        }
        ++buttonIndex;
    }

    const int buttonIndexToSelect = selectedButtonIndex == UNSELECTED_BUTTON ? 0 : selectedButtonIndex;

    if (selectedButtonIndex == UNSELECTED_BUTTON)
    {
        std::cout << "WARNING: current 'NumPasses' value doesn't match GUI buttons, resetting it to match first button value"
                  << std::endl;
    }
    return buttonIndexToSelect;
}

void ToolsOptionsWidget::setSelectedNumPassButton(int buttonIndexToSelect, QButtonGroup *group)
{
    if (group == nullptr) {
        group = numPassesButtons;
    }
    Q_ASSERT(group != nullptr);
    Q_ASSERT(buttonIndexToSelect < group->buttons().size());
    group->buttons()[buttonIndexToSelect]->setChecked(true);
}

QButtonGroup *ToolsOptionsWidget::addNumPassesArea()
{
    auto group = new QButtonGroup();



    numPassesTitle = new QLabel("Number of Computation passes:");
    toolsOptionsLayout->addWidget(numPassesTitle);

    toolsOptionsLayout->addLayout(numPassesLayout);

    const MandelBrotRenderer::MQintVector& passesOptions = masterThread->getPossiblePassValues();

    for (int passes: passesOptions)
    {
        auto button = new RadioIntegerButton(passes, masterThread);
        group->addButton(button);
        numPassesLayout->addWidget(button);
        connect(button, SIGNAL(clicked(bool)), button, SLOT(enforceValue()));
    }

    int buttonIndexToSelect = findSelectedNumPassesButton();

    setSelectedNumPassButton(buttonIndexToSelect, group);

    masterThread->setNumberOfPasses(passesOptions[buttonIndexToSelect]);

    return group;
}

void ToolsOptionsWidget::setAlgorithmInGUI()
{
    const threadAlgorithm currentAlgorithm = threadMediatorEnabled ?
                threadAlgorithm::dynamic_algorithm : threadAlgorithm::uniform_algorithm;

    threadAlgorithmButtons->buttons()[MandelBrotRenderer::toUnderlyingType(currentAlgorithm)]->setChecked(true);
}

void ToolsOptionsWidget::setNumPassesInGUI()
{
    setSelectedNumPassButton(findSelectedNumPassesButton());
}

void ToolsOptionsWidget::setColorMapSizeinGUI()
{
    masterThread->setColormapSize(colorMapSize);
    colorMapSizeSetting->setValue(colorMapSize);
}

void ToolsOptionsWidget::setDetailedInfoInGUI()
{
    showInfoButton->setCheckState(displayDetailedInfo ? Qt::Checked : Qt::Unchecked);
}

void ToolsOptionsWidget::setNumericTypeInGUI()
{
    const MandelBrotRenderer::RendererData&  renderSettings = masterThread->getRendererData();
    numericTypeSelection->setCurrentIndex(toUnderlyingType(renderSettings.numericType));
}


void ToolsOptionsWidget::refresh()
{
    setAlgorithmInGUI();
    setNumWorkerThreadsInGUI();
    setNumPassesInGUI();
    setColorMapSizeinGUI();
    setDetailedInfoInGUI();
    setNumericTypeInGUI();
}

QButtonGroup *ToolsOptionsWidget::addthreadAlgorithmArea()
{
    auto group = new QButtonGroup();

    threadAlgorithmTitle = new QLabel("Thread Algorithm:");
    toolsOptionsLayout->addWidget(threadAlgorithmTitle);

    static const std::map<threadAlgorithm, QString> algorithmDescriptions { std::make_pair(threadAlgorithm::uniform_algorithm, "Uniform allocation"),
                std::make_pair(threadAlgorithm::dynamic_algorithm, "Dynamically redistribute tasks") };

    QCheckBox* simpleButton = new QCheckBox(algorithmDescriptions.at(threadAlgorithm::uniform_algorithm));
    simpleButton->setToolTip(tr("compute regions are shared evenly among threads, no task reallocation occurs"));
    group->addButton(simpleButton);
    threadAlgorithmLayout->addWidget(simpleButton);

    connect(simpleButton, SIGNAL(stateChanged(int)), &(masterThread->getThreadMediator()), SLOT(setEnabledByState(int)));

    connect(simpleButton, SIGNAL(stateChanged(int)), masterThread, SLOT(publishDynamicTasksEnabled()));

    QCheckBox* reAllocationsButton = new QCheckBox(algorithmDescriptions.at(threadAlgorithm::dynamic_algorithm));

    reAllocationsButton->setEnabled(dynamicTaskAllocationSupported);
    reAllocationsButton->setToolTip(tr("busy threads share compute tasks with newly idle threads"));

    group->addButton(reAllocationsButton);
    threadAlgorithmLayout->addWidget(reAllocationsButton);

    toolsOptionsLayout->addLayout(threadAlgorithmLayout);

    return group;
}

void ToolsOptionsWidget::addThreadSlider()
{
    toolsOptionsLayout->addWidget(sliderTitle);

    auto sliderLayout = new QGridLayout();

    auto sliderRegion = new QWidget(this);
    toolsOptionsLayout->addWidget(sliderRegion);

    const int maxCol = threadCountSlider->maximum() - threadCountSlider->minimum();
    sliderRegion->setLayout(sliderLayout);
    sliderLayout->addWidget(threadCountSlider, 0, 0, 1, maxCol + 1);

    QLabel* leftmostValue = new QLabel(QString::number(threadCountSlider->minimum()), sliderRegion);
    sliderLayout->addWidget(leftmostValue, 1, 0);
    QLabel* rightmostValue = new QLabel(QString::number(threadCountSlider->maximum()), sliderRegion);
    sliderLayout->addWidget(rightmostValue, 1, maxCol);

    toolsOptionsLayout->addWidget(sliderRegion);

}

void ToolsOptionsWidget::setNumWorkerThreadsInGUI()
{
    threadCountSlider->setSliderPosition(numWorkerThreads);
}

void ToolsOptionsWidget::createThreadSlider()
{
    const int maxNumThreads = RenderThread::calculateInitialNumThreads();
    sliderTitle = new QLabel("Threads:");

    threadCountSlider = new QSlider(Qt::Horizontal, this);
    threadCountSlider->setRange(MandelBrotRenderer::MIN_NUM_WORKER_THREADS, maxNumThreads);
    threadCountSlider->setTickInterval(1);

    if ( numWorkerThreads < MandelBrotRenderer::MIN_NUM_WORKER_THREADS ||
         numWorkerThreads > maxNumThreads)
    {
        throw std::out_of_range("Number of worker threads out of supported range");
    }


    threadCountSlider->setTickPosition(QSlider::TicksBothSides);
    threadCountSlider->setToolTip("choose the number of CPU threads to use");

    masterThread->setThreadConfigurer(threadCountSlider);
    setNumWorkerThreadsInGUI();
}

void ToolsOptionsWidget::addColorMapSizeField()
{
    constexpr int MIN_COLORMAP_SIZE = 128;
    constexpr int MAX_COLORMAP_SIZE = 32768;
    colorMapSizeSetting->setRange(MIN_COLORMAP_SIZE, MAX_COLORMAP_SIZE);

    colorMapSizeSetting->setToolTip(tr("Press RETURN to re-render the image with the new color map setting"));

    Q_ASSERT(colorMapSizeSetting != nullptr);
    Q_ASSERT(colorMapSize >= colorMapSizeSetting->minimum() &&
             colorMapSize <= colorMapSizeSetting->maximum());
    setColorMapSizeinGUI();

    colorMapSizeSetting->setValue(colorMapSize);

    connect(colorMapSizeSetting, SIGNAL(valueChanged(int)), masterThread, SLOT(setColormapSize(int)));
    connect(colorMapSizeSetting, SIGNAL(valueChanged(int)), mainWidget, SLOT(displayColorMapSizeInfo(int)));

    connect(colorMapSizeSetting, SIGNAL(editingFinished()), mainWidget, SLOT(initiateRefresh()));

    colorMapTitle = new QLabel("Colour Map Size [" +
                                QString::number(colorMapSizeSetting->minimum()) + " to " +
                                QString::number(colorMapSizeSetting->maximum()) + " shades]:");
    toolsOptionsLayout->addWidget(colorMapTitle);
    toolsOptionsLayout->addWidget(colorMapSizeSetting);

    mainWidget->displayColorMapSizeInfo(colorMapSize);
}

void ToolsOptionsWidget::enforceChosenDataType(int index)
{
    QString description = numericTypeSelection->itemText(index);
    masterThread->setInternalDataType(description);
}

void ToolsOptionsWidget::initializeChosenDataType()
{
     const MandelBrotRenderer::RendererData&  renderSettings = masterThread->getRendererData();

     Q_ASSERT(numericTypeSelection != nullptr);

     //TODO: find a better way to do this - we are tying renderThread and ComboBox orderings here
     numericTypeSelection->setCurrentIndex(toUnderlyingType(renderSettings.numericType));
}

RenderThread::typeNameUser  ToolsOptionsWidget::AddNumericTypeToSelector() const
{
    return ([&] (const QString& description, bool supported)
    {
        auto model = dynamic_cast<QStandardItemModel *>(numericTypeSelection->model() );

        if (supported || model != nullptr)
        {
            numericTypeSelection->addItem(description);

            if (!supported)
            {
                Q_ASSERT(numericTypeSelection->insertPolicy() == QComboBox::InsertAtBottom);
                int index = numericTypeSelection->count() - 1;
                Q_ASSERT(index >= 0);
                model->item(index)->setEnabled(false);
            }
        }
    });
}

void ToolsOptionsWidget::addNumericTypeSelector()
{
    numericTypeTitle = new QLabel("Internal data type");

    numericTypeSelection->setInsertPolicy(QComboBox::InsertAtBottom);

    RenderThread::typeNameUser adder = AddNumericTypeToSelector();

    masterThread->initializeSupportedTypesTable(adder);

    initializeChosenDataType();

    connect(numericTypeSelection, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [=](int index){ enforceChosenDataType(index); });

    toolsOptionsLayout->addWidget(numericTypeTitle);
    toolsOptionsLayout->addWidget(numericTypeSelection);
}

void ToolsOptionsWidget::addInfoControlButton()
{
    showInfoButton = new QCheckBox("Show Detailed Information");
    showInfoButton->setEnabled(true);


    connect(showInfoButton, SIGNAL(stateChanged(int)), mainWidget->getInfoDisplayer(), SLOT(setEnabledByButtonState(int)));

    toolsOptionsLayout->addWidget(showInfoButton);
    setDetailedInfoInGUI();
}

void ToolsOptionsWidget::updateFromSettings()
{
    processSettingUpdate(applicationSettingsHandler.getSettings());
    refresh();
}
