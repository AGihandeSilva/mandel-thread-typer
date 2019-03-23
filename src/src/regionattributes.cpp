#include "regionattributes.h"

#include "mandelbrotrenderer.h"
#include "ParameterMaker.h"
#include "PrecisionHandler.h"

using MandelBrotRenderer::comparefloatingPointValues;
using MandelBrotRenderer::generateFloatFromString;
using MandelBrotRenderer::DoubleResult;
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
using MandelBrotRenderer::generateFloatFromPreciseString;
using MandelBrotRenderer::PreciseFloatResult;
#endif

RegionAttributes::RegionAttributes()
: scaleFactor(0.0), centerX("0.0"), centerY("0.0"),
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
  preciseCenterX("0.0"), preciseCenterY("0.0"),
#endif
  minX(defaultDimension), maxX(defaultDimension), minY(defaultDimension), maxY(defaultDimension), fullHeight(defaultDimension)
{

}

RegionAttributes::RegionAttributes(double scaleFactor, MandelBrotRenderer::CoordValue& centerX, MandelBrotRenderer::CoordValue& centerY,
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
                                   QString preciseCenterX,
                                   QString preciseCenterY,
#endif
                                   int minX, int maxX, int minY, int maxY, int fullHeight)
    : scaleFactor(scaleFactor), centerX(centerX), centerY(centerY),
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
      preciseCenterX(preciseCenterX),
      preciseCenterY(preciseCenterY),
#endif
      minX(minX), maxX(maxX), minY(minY), maxY(maxY), fullHeight(fullHeight)
{

}

bool RegionAttributes::operator==(const RegionAttributes &other)
{
    DoubleResult centerX_float_this = generateFloatFromString(centerX);
    DoubleResult centerY_float_this = generateFloatFromString(centerY);
    DoubleResult centerX_float_other = generateFloatFromString(other.centerX);
    DoubleResult centerY_float_other = generateFloatFromString(other.centerY);
    return (
                comparefloatingPointValues(scaleFactor, other.scaleFactor) &&
                centerX_float_this.second && centerX_float_other.second &&
                comparefloatingPointValues(centerX_float_this.first, centerX_float_other.second) &&
                centerY_float_this.second && centerY_float_other.second &&
                comparefloatingPointValues(centerY_float_this.first,centerY_float_other.first) &&
            #if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
                  preciseCenterX == other.preciseCenterX &&
                  preciseCenterY == other.preciseCenterY &&
            #endif
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



#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
double RegionAttributes::getCenterX() const
{
    std::streamsize XYprecision = 6;
    PreciseFloatResult centerX_float = generateFloatFromPreciseString(centerX, XYprecision);
    Q_ASSERT(centerX_float.second);
    return static_cast<double>(centerX_float.first);
}

QString RegionAttributes::getPreciseCenterX() const
{
    return preciseCenterX;
}

QString RegionAttributes::getPreciseCenterY() const
{
    return preciseCenterY;
}

double RegionAttributes::getCenterY() const
{
    std::streamsize XYprecision = 6;
    PreciseFloatResult centerY_float = generateFloatFromPreciseString(centerY, XYprecision);
    Q_ASSERT(centerY_float.second);
    return static_cast<double>(centerY_float.first);
}
#else
double RegionAttributes::getCenterX() const
{
   DoubleResult centerX_double =  generateFloatFromString(centerX);
   Q_ASSERT(centerX_double.second);
    return centerX_double.first;
}

double RegionAttributes::getCenterY() const
{
   DoubleResult centerY_double =  generateFloatFromString(centerY);
   Q_ASSERT(centerY_double.second);
    return centerY_double.first;
}

#endif


