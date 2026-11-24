#include "regionattributes.h"

#include "mandelbrotrenderer.h"
#include "ParameterMaker.h"
#include "PrecisionHandler.h"

using MandelBrotRenderer::comparefloatingPointValues;
using MandelBrotRenderer::generateFloatFromString;
using MandelBrotRenderer::DoubleResult;
using MandelBrotRenderer::generateFloatFromPreciseString;
using MandelBrotRenderer::PreciseFloatResult;

RegionAttributes::RegionAttributes()
: scaleFactor("0.0"), originX("0.0"), originY("0.0"),
  preciseOriginX("0.0"), preciseOriginY("0.0"),
  minX(defaultDimension), maxX(defaultDimension), minY(defaultDimension), maxY(defaultDimension), fullHeight(defaultDimension)
{

}

RegionAttributes::RegionAttributes(MandelBrotRenderer::CoordValue& scaleFactor, MandelBrotRenderer::CoordValue& originX, MandelBrotRenderer::CoordValue& originY,
                                   QString preciseOriginX,
                                   QString preciseOriginY,
                                   int minX, int maxX, int minY, int maxY, int fullHeight)
    : scaleFactor(scaleFactor), originX(originX), originY(originY),
      preciseOriginX(preciseOriginX),
      preciseOriginY(preciseOriginY),
      minX(minX), maxX(maxX), minY(minY), maxY(maxY), fullHeight(fullHeight)
{

}

bool RegionAttributes::operator==(const RegionAttributes &other)
{
    DoubleResult originX_float_this = generateFloatFromString(originX);
    DoubleResult originY_float_this = generateFloatFromString(originY);
    DoubleResult originX_float_other = generateFloatFromString(other.originX);
    DoubleResult originY_float_other = generateFloatFromString(other.originY);
    DoubleResult scaleFactor_this = generateFloatFromString(scaleFactor);
    DoubleResult scaleFactor_other = generateFloatFromString(other.scaleFactor);
    return (
                scaleFactor_this.second && scaleFactor_other.second &&
                comparefloatingPointValues(scaleFactor_this.first, scaleFactor_other.first) &&
                originX_float_this.second && originX_float_other.second &&
                comparefloatingPointValues(originX_float_this.first, originX_float_other.second) &&
                originY_float_this.second && originY_float_other.second &&
                comparefloatingPointValues(originY_float_this.first, originY_float_other.first) &&
                  preciseOriginX == other.preciseOriginX &&
                  preciseOriginY == other.preciseOriginY &&
                minX == other.minX &&
                maxX == other.maxX &&
                minY == other.minY &&
                maxY == other.maxY &&
                fullHeight == other.fullHeight
                );
}

int RegionAttributes::getMinX() const
{
    return minX;
}

int RegionAttributes::getMaxX() const
{
    return maxX;
}

int RegionAttributes::getMinY() const
{
    return minY;
}

int RegionAttributes::getMaxY() const
{
    return maxY;
}

int RegionAttributes::getFullHeight() const
{
    return fullHeight;
}

int RegionAttributes::computeRawDataSize() const
{
 return ((maxX + 1 - minX) *
         (maxY + 1 - minY));
}

void RegionAttributes::adjustYValues(int currentYPos, bool isLowerHalf)
{
    const int remainingPixelLines = maxY - currentYPos;
    const int newSize = remainingPixelLines / 2;
    const int newYBoundary =  currentYPos + newSize;
    if (isLowerHalf)
    {
        minY = newYBoundary;
    }
    else
    {
        maxY = newYBoundary;
    }
}

QString RegionAttributes::getPreciseOriginX() const
{
    return preciseOriginX;
}

QString RegionAttributes::getPreciseOriginY() const
{
    return preciseOriginY;
}

