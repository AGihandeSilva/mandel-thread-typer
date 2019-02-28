#include "regionattributes.h"

#include "mandelbrotrenderer.h"

using MandelBrotRenderer::comparefloatingPointValues;

RegionAttributes::RegionAttributes()
: scaleFactor(0.0), centerX(0.0), centerY(0.0), minX(defaultDimension), maxX(defaultDimension), minY(defaultDimension), maxY(defaultDimension), fullHeight(defaultDimension)
{

}

RegionAttributes::RegionAttributes(double scaleFactor, double centerX, double centerY,
                                   int minX, int maxX, int minY, int maxY, int fullHeight)
    : scaleFactor(scaleFactor), centerX(centerX), centerY(centerY),
      minX(minX), maxX(maxX), minY(minY), maxY(maxY), fullHeight(fullHeight)
{

}

bool RegionAttributes::operator==(const RegionAttributes &other)
{
    return (
                comparefloatingPointValues(scaleFactor, other.scaleFactor) &&
                comparefloatingPointValues(centerX, other.centerX) &&
                comparefloatingPointValues(centerY, other.centerY) &&
                minX == other.minX &&
                maxX == other.maxX &&
                minY == other.minY &&
                maxY == other.maxY &&
                fullHeight == other.fullHeight
                );
}

double RegionAttributes::getScaleFactor() const
{
    return scaleFactor;
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

double RegionAttributes::getCenterY() const
{
    return centerY;
}

double RegionAttributes::getCenterX() const
{
    return centerX;
}
