#include "RenderParametersWidget.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QSizePolicy>
#include <QFontMetrics>
#include <QDoubleValidator>

#include "settingshandler.h"

#include "MandelbrotGuiTools.h"
#include "mandelbrotwidget.h"

using MandelBrotRenderer::addHorizontalLine;

void RenderParametersWidget::setUpFields()
{
    QFont currentFont = font();
    QFontMetrics metrics(currentFont);

    const int magnitude = 6;
    const int boxHeight = 20;
    const int otherCharacters = 3;
    const int characterWidth = otherCharacters + magnitude +
#if (USE_BOOST_MULTIPRECISION == 1 || defined(__GNUC__))
            std::numeric_limits<MandelBrotRenderer::Float128>::max_digits10;
#else
            std::numeric_limits<double>::max_digits10;
#endif
    const int extraMarginInPercent = 5;
    QString testString(characterWidth, '8');
    int pixelsWidth = ((100 + extraMarginInPercent) * metrics.size(Qt::TextSingleLine, testString).width()) / 100;


    xTitle = new QLabel("X:");
    renderParametersLayout->addWidget(xTitle);

    xValue->setValidator(xValidator);
    xValue->setFixedHeight(boxHeight);
    xValue->setMinimumWidth(pixelsWidth / 2);
    xValue->setMaximumWidth(pixelsWidth);
    xValue->setSizePolicy(QSizePolicy::Ignored, QSizePolicy ::Fixed);
    renderParametersLayout->addWidget(xValue);
    addHorizontalLine(this, renderParametersLayout);

    yTitle = new QLabel("Y:");
    renderParametersLayout->addWidget(yTitle);

    yValue->setValidator(yYValidator);
    yValue->setFixedHeight(boxHeight);
    yValue->setMinimumWidth(pixelsWidth / 2);
    yValue->setMaximumWidth(pixelsWidth);
    yValue->setSizePolicy(QSizePolicy::Ignored, QSizePolicy ::Fixed);
    renderParametersLayout->addWidget(yValue);
    addHorizontalLine(this, renderParametersLayout);


    widthTitle = new QLabel("Width:");
    renderParametersLayout->addWidget(widthTitle);

    widthValue->setValidator(heightValidator);
    widthValue->setMaximumWidth(pixelsWidth);
    widthValue->setFixedHeight(boxHeight);
    renderParametersLayout->addWidget(widthValue);
    addHorizontalLine(this, renderParametersLayout);

    heightTitle = new QLabel("Height:");
    renderParametersLayout->addWidget(heightTitle);

    widthValue->setValidator(widthValidator);
    heightValue->setMaximumWidth(pixelsWidth);
    heightValue->setFixedHeight(boxHeight);
    renderParametersLayout->addWidget(heightValue);
    addHorizontalLine(this, renderParametersLayout);
}

RenderParametersWidget::RenderParametersWidget(RenderThread *masterThread, MandelbrotWidget& mainWidget, SettingsHandler& settingsHandler)
    : masterThread(masterThread), mainWidget(mainWidget),
      applicationSettingsHandler(settingsHandler), renderParametersLayout(nullptr),
        xTitle(nullptr), xValue(nullptr), yTitle(nullptr), yValue(nullptr),
        widthTitle(nullptr), widthValue(nullptr), heightTitle(nullptr), heightValue(nullptr),
        xValidator(nullptr), yYValidator(nullptr),
        widthValidator(nullptr), heightValidator(nullptr)
{
    setWindowTitle("Render Parameters");
    renderParametersLayout = new QVBoxLayout;


    initializeFieldsAndValidators();




    //renderParametersLayout->setSizeConstraint(QLayout::SetMinimumSize);
    //renderParametersLayout->SetMinimumSize(width, height);
    this->setLayout(renderParametersLayout);

    setUpFields();

    updateFromSettings();

    applicationSettingsHandler.registerSettingsUser(this);
    mainWidget.registerCoordinateUser(this,
              MandelBrotRenderer::CoordinateListenerfullPrecision  );
}

void RenderParametersWidget::initializeFieldsAndValidators()
{
    xValue = new QLineEdit;

    yValue = new QLineEdit;
    widthValue = new QLineEdit;
    heightValue = new QLineEdit;

    //TODO enable editing of these fields to make another
    //user input flow
    xValue->setReadOnly(true);
    yValue->setReadOnly(true);
    widthValue->setReadOnly(true);
    heightValue->setReadOnly(true);

    MandelBrotRenderer::RegionLimits parameterBoundaryValues = mainWidget.getParameterSpace();
    const double minSize = 1.0E-70;

    xValidator = new QDoubleValidator(xValue);
    xValidator->setBottom(parameterBoundaryValues.xMin);
    xValidator->setTop(parameterBoundaryValues.xMax);

    yYValidator = new QDoubleValidator(yValue);
    yYValidator->setBottom(parameterBoundaryValues.yMin);
    yYValidator->setTop(parameterBoundaryValues.yMax);

    widthValidator = new QDoubleValidator(widthValue);
    widthValidator->setBottom(minSize);
    widthValidator->setTop(parameterBoundaryValues.xMax - parameterBoundaryValues.xMin);

    heightValidator = new QDoubleValidator(heightValue);
    heightValidator->setBottom(minSize);
    heightValidator->setTop(parameterBoundaryValues.yMax - parameterBoundaryValues.yMin);
}


void RenderParametersWidget::updateCoordData(const QString &centerX, const QString &centerY, const QString &width, const QString &height)
{
    QFont currentFont = font();
    QFontMetrics metrics(currentFont);
    const int extraMarginInPercent = 20;


    if (!centerX.isEmpty()) {
        int pixelsWidth = ((100 + extraMarginInPercent) * metrics.size(Qt::TextSingleLine, centerX).width()) / 100;
        xValue->setMinimumWidth(pixelsWidth);
        xValue->setText(centerX);
    }
    if (!centerY.isEmpty()) {
        int pixelsWidth = ((100 + extraMarginInPercent) * metrics.size(Qt::TextSingleLine, centerY).width()) / 100;
        yValue->setMinimumWidth(pixelsWidth);
        yValue->setText(centerY);
    }
    widthValue->setText(width);
    heightValue->setText(height);
}

void RenderParametersWidget::processSettingUpdate(QSettings &settings)
{
    settings.beginGroup("RenderParameters");
        xValue->setText(settings.value("centerX", MandelbrotWidget::getDefaultCenterX()).toString());
        yValue->setText(settings.value("centerY", MandelbrotWidget::getDefaultCenterY()).toString());
        //curScale = settings.value("curScale", DefaultScale).toDouble();
        //pixmapScale = settings.value("pixmapScale", DefaultScale).toDouble();
    settings.endGroup();
}

void RenderParametersWidget::updateAndShow()
{
    updateFromSettings();
    raise();
    show();
}

void RenderParametersWidget::refresh()
{
}

void RenderParametersWidget::updateFromSettings()
{
    processSettingUpdate(applicationSettingsHandler.getSettings());
    refresh();
}
