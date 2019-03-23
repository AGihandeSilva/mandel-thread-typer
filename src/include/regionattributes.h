#ifndef REGIONATTRIBUTES_H
#define REGIONATTRIBUTES_H

#include "mandelbrotrenderer.h"

#include <QString>

class RegionAttributes
{
public:
   RegionAttributes();

   explicit RegionAttributes(double scaleFactor,
                     MandelBrotRenderer::CoordValue& centerX,
                     MandelBrotRenderer::CoordValue& centerY,
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
                     QString preciseCenterX,
                     QString preciseCenterY,
#endif
                     int minX,
                     int maxX,
                     int minY,
                     int maxY,
                     int fullHeight);

   ~RegionAttributes() = default;
   RegionAttributes(const RegionAttributes&) = default;
   RegionAttributes(RegionAttributes&&) = default;
   RegionAttributes& operator=(const RegionAttributes&) = default;
   RegionAttributes& operator=(RegionAttributes&&) = default;


    bool operator==(const RegionAttributes& other);

    double getScaleFactor() const;
    double getCenterX() const;
    double getCenterY() const;
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
    QString getPreciseCenterX() const;
    QString getPreciseCenterY() const;
#endif
    int getMinX() const;
    int getMaxX() const;
    int getMinY() const;
    int getMaxY() const;
    int getFullHeight() const;
    int computeRawDataSize() const;
    void adjustYValues(int currentYPos, bool isLowerHalf);

private:
    double scaleFactor;
    MandelBrotRenderer::CoordValue centerX;
    MandelBrotRenderer::CoordValue centerY;
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
    QString preciseCenterX;
    QString preciseCenterY;
#endif
    int minX;
    int maxX;
    int minY;
    int maxY;
    int fullHeight;

    static constexpr int defaultDimension = -1;
};

#endif // REGIONATTRIBUTES_H
