#ifndef REGIONATTRIBUTES_H
#define REGIONATTRIBUTES_H


class RegionAttributes
{
public:
   RegionAttributes();

   explicit RegionAttributes(double scaleFactor,
                     double centerX,
                     double centerY,
                     int minX,
                     int maxX,
                     int minY,
                     int maxY,
                     int fullHeight);


    bool operator==(const RegionAttributes& other);

    double getScaleFactor() const;
    double getCenterX() const;
    double getCenterY() const;
    int getMinX() const;
    int getMaxX() const;
    int getMinY() const;
    int getMaxY() const;
    int getFullHeight() const;
    int computeRawDataSize() const;
    void adjustYValues(int currentYPos, bool isLowerHalf);

private:
    double scaleFactor;
    double centerX;
    double centerY;
    int minX;
    int maxX;
    int minY;
    int maxY;
    int fullHeight;

    static constexpr int defaultDimension = -1;
};

#endif // REGIONATTRIBUTES_H
