#ifndef RENDERERCONFIG_H
#define RENDERERCONFIG_H

#include <QString>
#include <QSize>
#include <QPoint>
#include <QFileInfo>

#include "mandelbrotrenderer.h"

class MandelbrotWidget;

class RendererConfig
{
public:
    RendererConfig() = default;

    RendererConfig(bool detailedDisplayEnabled,
                   QSize size, QPoint pos, const MandelBrotRenderer::CoordValue& centerX, const MandelBrotRenderer::CoordValue& centerY, double curScale, double pixmapScale,
                   bool threadMediatorEnabled, int colorMapSize, MandelBrotRenderer::internalDataType dataType);

    explicit RendererConfig(MandelBrotRenderer::RenderState&& state);

    bool operator==(RendererConfig &rhs) const;

    bool operator!=(RendererConfig &rhs) const;

    bool checkLegality() const;
    bool write(const QFileInfo& filePath);

    static std::pair<bool, RendererConfig> createFromFile(const QFileInfo& filePath);

    bool getDetailedDisplayEnabled() const;
    QSize getSize() const;
    QPoint getPos() const;
    double getCenterX() const;
    double getCenterY() const;
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
    const QString& getPreciseCenterX() const;
    const QString& getPreciseCenterY() const;
#endif
    double getCurScale() const;
    double getPixmapScale() const;
    bool getThreadMediatorEnabled() const;
    bool getColorMapSize() const;
    MandelBrotRenderer::RenderState getState() { return state;}
    MandelBrotRenderer::internalDataType getInternalDataType() const;

private:
    bool sizeIsInvalid(const QSize & size) const;

    MandelBrotRenderer::RenderState state;
};

#endif // RENDERERCONFIG_H
