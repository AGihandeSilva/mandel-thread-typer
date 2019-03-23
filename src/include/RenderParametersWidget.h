#ifndef RENDERPARAMETERSWIDGET_H
#define RENDERPARAMETERSWIDGET_H

#include <QDialog>

#include "settingsuser.h"
#include "mandelbrotrenderer.h"

class RenderThread;
class MandelbrotWidget;
class QVBoxLayout;
class QLineEdit;
class SettingsHandler;
class QString;
class QLabel;
class QDoubleValidator;

class RenderParametersWidget : public QDialog, public SettingsUser, public MandelBrotRenderer::CoordinateListener
{
public:
    explicit RenderParametersWidget(RenderThread *masterThread,
                                    MandelbrotWidget& mainWidget, SettingsHandler& settingsHandler);

    void updateCoordData(const QString& centerX, const QString& centerY,
                         const QString& width, const QString& height) override;

public slots:
    void processSettingUpdate(QSettings& settings) override;
    void updateAndShow();
    void refresh();

private:

    void updateFromSettings();
    void initializeFieldsAndValidators();

    RenderThread *masterThread;
    MandelbrotWidget& mainWidget;
    SettingsHandler& applicationSettingsHandler;

    QVBoxLayout* renderParametersLayout;
    QLabel* xTitle;
    QLineEdit* xValue;
    QLabel* yTitle;
    QLineEdit* yValue;
    QLabel* widthTitle;
    QLineEdit* widthValue;
    QLabel* heightTitle;
    QLineEdit* heightValue;

    QDoubleValidator* xValidator;
    QDoubleValidator* yYValidator;
    QDoubleValidator* widthValidator;
    QDoubleValidator* heightValidator;
    void setUpFields();
};

#endif // RENDERPARAMETERSWIDGET_H
