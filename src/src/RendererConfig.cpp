#include "RendererConfig.h"

#include <QFile>
#include <QDataStream>
#include "PrecisionHandler.h"

#if (USE_BOOST_MULTIPRECISION == 1 || defined(__GNUC__))
using MandelBrotRenderer::generateFloatFromPreciseString;
#endif
using MandelBrotRenderer::DoubleResult;

RendererConfig::RendererConfig(bool detailedDisplayEnabled,
                   QSize size, QPoint pos, const MandelBrotRenderer::CoordValue& centerX, const MandelBrotRenderer::CoordValue& centerY, double curScale, double pixmapScale,
                   bool threadMediatorEnabled, int colorMapSize, MandelBrotRenderer::internalDataType dataType) :

    state { detailedDisplayEnabled, size, pos, centerX, centerY, curScale,
            pixmapScale, threadMediatorEnabled, colorMapSize, dataType} {}

RendererConfig::RendererConfig(MandelBrotRenderer::RenderState&& state) : state(std::move(state)){}

bool RendererConfig::operator==(RendererConfig &rhs) const
{
    return (state == rhs.getState());
}

bool  RendererConfig::operator!=(RendererConfig &rhs) const
{
    return (!(*this == rhs));
}

bool RendererConfig::write(const QFileInfo& filePath)
{
    bool result = false;

    if (filePath.path().isEmpty()) {
            return false;
    }

    QFile outputFile(filePath.filePath());
    result = outputFile.open(QIODevice::WriteOnly);

    if (!result)
    {
            return result;
    }
    QDataStream out(&outputFile);

    out << state;

    result = (out.status() == QDataStream::Ok);

    return result;
}

std::pair<bool, RendererConfig> RendererConfig::createFromFile(const QFileInfo &filePath)
{
    bool result = false;

    RendererConfig configData;

    if (filePath.path().isEmpty()) {
            return std::make_pair(result, configData);
    }

    QFile inputFile(filePath.filePath());
    result = inputFile.open(QIODevice::ReadOnly);

    if (!result)
    {
            return std::make_pair(result, configData);
    }
    QDataStream in(&inputFile);

    in >> configData.state;

    result = (in.status() == QDataStream::Ok);

    return (std::make_pair(result, configData));
}

bool RendererConfig::sizeIsInvalid(const QSize &size) const
{
    return (size.isEmpty() ||
            size.width() > MandelBrotRenderer::MAX_RENDER_SIZE_X ||
            size.height() > MandelBrotRenderer::MAX_RENDER_SIZE_Y);
}

bool RendererConfig::getThreadMediatorEnabled() const
{
    return state.threadMediatorEnabled;
}

bool RendererConfig::getColorMapSize() const
{
    return state.colorMapSize;
}

MandelBrotRenderer::internalDataType RendererConfig::getInternalDataType() const
{
    return state.numericType;
}

double RendererConfig::getPixmapScale() const
{
    return state.pixmapScale;
}

double RendererConfig::getCurScale() const
{
    return state.curScale;
}

double RendererConfig::getCenterY() const
{
    DoubleResult centerY_float = MandelBrotRenderer::generateFloatFromString(state.centerY);
    Q_ASSERT(centerY_float.second);
    return centerY_float.first;
}

#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
const QString& RendererConfig::getPreciseCenterX() const
{
    return state.centerX;
}

const QString& RendererConfig::getPreciseCenterY() const
{
    return state.centerY;
}
#endif

double RendererConfig::getCenterX() const
{
    DoubleResult centerX_float = MandelBrotRenderer::generateFloatFromString(state.centerX);
    Q_ASSERT(centerX_float.second);
    return centerX_float.first;
}

QPoint RendererConfig::getPos() const
{
    return state.pos;
}

QSize RendererConfig::getSize() const
{
    return state.size;
}

bool RendererConfig::getDetailedDisplayEnabled() const
{
    return state.detailedDisplayEnabled;
}

bool RendererConfig::checkLegality() const
{
    return !(sizeIsInvalid(state.size));
}
