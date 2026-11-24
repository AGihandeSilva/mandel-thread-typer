#ifndef REGIONATTRIBUTES_H
#define REGIONATTRIBUTES_H

#include "mandelbrotrenderer.h"

#include <QString>

class RegionAttributes
{
public:
   RegionAttributes();

   explicit RegionAttributes(MandelBrotRenderer::CoordValue& scaleFactor,
                     MandelBrotRenderer::CoordValue& originX,
                     MandelBrotRenderer::CoordValue& originY,
                     QString preciseOriginX,
                     QString preciseOriginY,
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

    MandelBrotRenderer::CoordValue getScaleFactor() const { return scaleFactor; }
    MandelBrotRenderer::CoordValue getRawScaleFactor() const { return scaleFactor; }

    MandelBrotRenderer::CoordValue  getOriginX() const { return originX; }
    MandelBrotRenderer::CoordValue  getOriginY() const { return originY; }

    QString getPreciseOriginX() const;
    QString getPreciseOriginY() const;
    int getMinX() const;
    int getMaxX() const;
    int getMinY() const;
    int getMaxY() const;
    int getFullHeight() const;
    int computeRawDataSize() const;
    void adjustYValues(int currentYPos, bool isLowerHalf);

private:
    MandelBrotRenderer::CoordValue scaleFactor;
    MandelBrotRenderer::CoordValue originX;
    MandelBrotRenderer::CoordValue originY;
    QString preciseOriginX;
    QString preciseOriginY;
    int minX;
    int maxX;
    int minY;
    int maxY;
    int fullHeight;

    static constexpr int defaultDimension = -1;
};

#endif // REGIONATTRIBUTES_H
