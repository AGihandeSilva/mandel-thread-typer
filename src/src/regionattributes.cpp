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
: scaleFactor(0.0), originX("0.0"), originY("0.0"),
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
  preciseOriginX("0.0"), preciseOriginY("0.0"),
#endif
  minX(defaultDimension), maxX(defaultDimension), minY(defaultDimension), maxY(defaultDimension), fullHeight(defaultDimension)
{

}

RegionAttributes::RegionAttributes(double scaleFactor, MandelBrotRenderer::CoordValue& originX, MandelBrotRenderer::CoordValue& originY,
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
                                   QString preciseOriginX,
                                   QString preciseOriginY,
#endif
                                   int minX, int maxX, int minY, int maxY, int fullHeight)
    : scaleFactor(scaleFactor), originX(originX), originY(originY),
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
      preciseOriginX(preciseOriginX),
      preciseOriginY(preciseOriginY),
#endif
      minX(minX), maxX(maxX), minY(minY), maxY(maxY), fullHeight(fullHeight)
{

}

bool RegionAttributes::operator==(const RegionAttributes &other)
{
    DoubleResult originX_float_this = generateFloatFromString(originX);
    DoubleResult originY_float_this = generateFloatFromString(originY);
    DoubleResult originX_float_other = generateFloatFromString(other.originX);
    DoubleResult originY_float_other = generateFloatFromString(other.originY);
    return (
                comparefloatingPointValues(scaleFactor, other.scaleFactor) &&
                originX_float_this.second && originX_float_other.second &&
                comparefloatingPointValues(originX_float_this.first, originX_float_other.second) &&
                originY_float_this.second && originY_float_other.second &&
                comparefloatingPointValues(originY_float_this.first, originY_float_other.first) &&
            #if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
                  preciseOriginX == other.preciseOriginX &&
                  preciseOriginY == other.preciseOriginY &&
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
double RegionAttributes::getOriginX() const
{
    std::streamsize XYprecision = 6;
    PreciseFloatResult originX_float = generateFloatFromPreciseString(originX, XYprecision);
    Q_ASSERT(originX_float.second);
    return static_cast<double>(originX_float.first);
}

QString RegionAttributes::getPreciseOriginX() const
{
    return preciseOriginX;
}

QString RegionAttributes::getPreciseOriginY() const
{
    return preciseOriginY;
}

double RegionAttributes::getOriginY() const
{
    std::streamsize XYprecision = 6;
    PreciseFloatResult originY_float = generateFloatFromPreciseString(originY, XYprecision);
    Q_ASSERT(originY_float.second);
    return static_cast<double>(originY_float.first);
}
#else
double RegionAttributes::getOriginX() const
{
   DoubleResult originX_double =  generateFloatFromString(originX);
   Q_ASSERT(originX_double.second);
    return originX_double.first;
}

double RegionAttributes::getOriginY() const
{
   DoubleResult originY_double =  generateFloatFromString(originY);
   Q_ASSERT(originY_double.second);
    return originY_double.first;
}

#endif


