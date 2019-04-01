#ifndef RENDERPARAMETERSWIDGET_H
#define RENDERPARAMETERSWIDGET_H

#include <QDialog>
#include <QDoubleValidator>

#include "settingsuser.h"
#include "mandelbrotrenderer.h"

class RenderThread;
class MandelbrotWidget;
class QVBoxLayout;
class QLineEdit;
class SettingsHandler;
class QString;
class QLabel;
class QDialogButtonBox;

class RenderParametersWidget : public QDialog, public SettingsUser, public MandelBrotRenderer::CoordinateListener
{
    Q_OBJECT
public:
    explicit RenderParametersWidget(RenderThread *masterThread,
                                    MandelbrotWidget& mainWidget, SettingsHandler& settingsHandler);

    void updateCoordData(const QString& originX, const QString& originY,
                         const QString& width, const QString& height) override;

public slots:
    void processSettingUpdate(QSettings&) override;
    void updateAndShow();
    void refresh();
    void processNewRegionParameters();

private:

    class RevertingDoubleValidator : public QDoubleValidator
    {
        public:
            explicit RevertingDoubleValidator(QObject *parent = nullptr)
             : QDoubleValidator(parent) {}

            void setRevertValue(const QString& text) { originalText = text; }
            QString getRevertValue() const { return originalText; }

            void fixup(QString& input) const override
            {
                input = originalText;
            }

        private:
            QString originalText;
    };

    void updateFromSettings();
    void initializeFieldsAndValidators();
    void revertParameters();

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
    QDialogButtonBox* acceptOrCancelBox;

    RevertingDoubleValidator* xValidator;
    RevertingDoubleValidator* yValidator;
    RevertingDoubleValidator* widthValidator;
    RevertingDoubleValidator* heightValidator;
    void setUpFields();
};

#endif // RENDERPARAMETERSWIDGET_H
