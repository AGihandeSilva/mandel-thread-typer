#include "RendererConfig.h"

#include <QFile>
#include <QDataStream>
#include "PrecisionHandler.h"

#if (USE_BOOST_MULTIPRECISION == 1 || defined(__GNUC__))
using MandelBrotRenderer::generateFloatFromPreciseString;
#endif
using MandelBrotRenderer::DoubleResult;

RendererConfig::RendererConfig(bool detailedDisplayEnabled,
                   QSize size, QPoint pos, const MandelBrotRenderer::CoordValue& originX, const MandelBrotRenderer::CoordValue& originY, double curScale, double pixmapScale,
                   bool threadMediatorEnabled, int m_colorMapSize, MandelBrotRenderer::internalDataType dataType) :

    state { detailedDisplayEnabled, size, pos, originX, originY, curScale,
            pixmapScale, threadMediatorEnabled, m_colorMapSize, dataType} {}

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
    return state.m_threadMediatorEnabled;
}

bool RendererConfig::getColorMapSize() const
{
    return state.m_colorMapSize;
}

MandelBrotRenderer::internalDataType RendererConfig::getInternalDataType() const
{
    return state.m_numericType;
}

double RendererConfig::getPixmapScale() const
{
    return state.m_pixmapScale;
}

double RendererConfig::getCurScale() const
{
    return state.m_curScale;
}

double RendererConfig::getOriginY() const
{
    DoubleResult originY_float = MandelBrotRenderer::generateFloatFromString(state.m_originY);
    Q_ASSERT(originY_float.second);
    return originY_float.first;
}

#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
const QString& RendererConfig::getPreciseOriginX() const
{
    return state.m_originX;
}

const QString& RendererConfig::getPreciseOriginY() const
{
    return state.m_originY;
}
#endif

double RendererConfig::getOriginX() const
{
    DoubleResult originX_float = MandelBrotRenderer::generateFloatFromString(state.m_originX);
    Q_ASSERT(originX_float.second);
    return originX_float.first;
}

QPoint RendererConfig::getPos() const
{
    return state.m_pos;
}

QSize RendererConfig::getSize() const
{
    return state.m_size;
}

bool RendererConfig::getDetailedDisplayEnabled() const
{
    return state.m_detailedDisplayEnabled;
}

bool RendererConfig::checkLegality() const
{
    return !(sizeIsInvalid(state.m_size));
}
