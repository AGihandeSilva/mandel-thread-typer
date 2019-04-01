#include "RenderParametersWidget.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QSizePolicy>
#include <QFontMetrics>
#include <QDoubleValidator>
#include <QDialogButtonBox>

#include "settingshandler.h"

#include "MandelbrotGuiTools.h"
#include "mandelbrotwidget.h"

using MandelBrotRenderer::addHorizontalLine;

RenderParametersWidget::RenderParametersWidget(RenderThread *masterThread, MandelbrotWidget& mainWidget, SettingsHandler& settingsHandler)
    : masterThread(masterThread), mainWidget(mainWidget),
      applicationSettingsHandler(settingsHandler), renderParametersLayout(nullptr),
        xTitle(nullptr), xValue(nullptr), yTitle(nullptr), yValue(nullptr),
        widthTitle(nullptr), widthValue(nullptr), heightTitle(nullptr), heightValue(nullptr),
        acceptOrCancelBox(nullptr), xValidator(nullptr), yValidator(nullptr),
        widthValidator(nullptr), heightValidator(nullptr)
{
    setWindowTitle("Render Parameters");
    renderParametersLayout = new QVBoxLayout;
    initializeFieldsAndValidators();
    this->setLayout(renderParametersLayout);
    setUpFields();

    updateFromSettings();

    applicationSettingsHandler.registerSettingsUser(this);
    mainWidget.registerCoordinateUser(this,
              MandelBrotRenderer::CoordinateListenerfullPrecision  );

    connect(acceptOrCancelBox, SIGNAL(accepted()), this, SLOT(processNewRegionParameters()));
    connect(acceptOrCancelBox, SIGNAL(rejected()), masterThread, SLOT(publishCoordinates()));
}

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

    yValue->setValidator(yValidator);
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

    acceptOrCancelBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, this);
    renderParametersLayout->addWidget(acceptOrCancelBox);
}

void RenderParametersWidget::initializeFieldsAndValidators()
{
    xValue = new QLineEdit;

    yValue = new QLineEdit;
    widthValue = new QLineEdit;
    heightValue = new QLineEdit;


    MandelBrotRenderer::RegionLimits parameterBoundaryValues = mainWidget.getParameterSpace();
    const double minSize = 1.0E-70;

    xValidator = new RevertingDoubleValidator(xValue);
    xValidator->setBottom(parameterBoundaryValues.xMin);
    xValidator->setTop(parameterBoundaryValues.xMax);

    yValidator = new RevertingDoubleValidator(yValue);
    yValidator->setBottom(parameterBoundaryValues.yMin);
    yValidator->setTop(parameterBoundaryValues.yMax);

    widthValidator = new RevertingDoubleValidator(widthValue);
    widthValidator->setBottom(minSize);
    widthValidator->setTop(parameterBoundaryValues.xMax - parameterBoundaryValues.xMin);

    heightValidator = new RevertingDoubleValidator(heightValue);
    heightValidator->setBottom(minSize);
    heightValidator->setTop(parameterBoundaryValues.yMax - parameterBoundaryValues.yMin);
}


void RenderParametersWidget::updateCoordData(const QString &originX, const QString &originY, const QString &width, const QString &height)
{
    QFont currentFont = font();
    QFontMetrics metrics(currentFont);
    const int extraMarginInPercent = 20;

    if (!originX.isEmpty()) {
        int pixelsWidth = ((100 + extraMarginInPercent) * metrics.size(Qt::TextSingleLine, originX).width()) / 100;
        xValue->setMinimumWidth(pixelsWidth);
        xValue->setText(originX);
        xValidator->setRevertValue(originX);
    }
    if (!originY.isEmpty()) {
        int pixelsWidth = ((100 + extraMarginInPercent) * metrics.size(Qt::TextSingleLine, originY).width()) / 100;
        yValue->setMinimumWidth(pixelsWidth);
        yValue->setText(originY);
        yValidator->setRevertValue(originY);
    }
    widthValue->setText(width);
    widthValidator->setRevertValue(width);
    heightValue->setText(height);
    heightValidator->setRevertValue(height);
}

void RenderParametersWidget::processSettingUpdate(QSettings &)
{
    QString X;
    QString Y;
    applicationSettingsHandler.getXYSettingsData(X, Y);

    xValue->setText(X);
    xValidator->setRevertValue(X);
    yValue->setText(Y);
    yValidator->setRevertValue(Y);
}

void RenderParametersWidget::updateAndShow()
{
    raise();
    show();
}

void RenderParametersWidget::refresh()
{
    updateFromSettings();
    masterThread->publishCoordinates();
}

void RenderParametersWidget::processNewRegionParameters()
{
    //std::cout << static_cast<const char*>(__FUNCTION__) << std::endl;
    //std::cout << "originX" << xValue->text().toStdString() << std::endl;
    bool accepted = mainWidget.changeRegionParameters(xValue->text(), yValue->text(), widthValue->text(), heightValue->text());
    std::cout << "parameters validation result: " << accepted << std::endl;

    if (!accepted)
    {
        mainWidget.writeTransientStatusMessage("Input Parameters were invalid, reverting to last good set", true);
        revertParameters();
    }
}

void RenderParametersWidget::revertParameters()
{
    xValue->setText(xValidator->getRevertValue());
    yValue->setText(yValidator->getRevertValue());
    widthValue->setText(widthValidator->getRevertValue());
    heightValue->setText(heightValidator->getRevertValue());
}

void RenderParametersWidget::updateFromSettings()
{
    processSettingUpdate(applicationSettingsHandler.getSettings());
}
