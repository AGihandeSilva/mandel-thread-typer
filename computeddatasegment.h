#ifndef COMPUTEDDATASEGMENT_H
#define COMPUTEDDATASEGMENT_H

#include <QVector>
#include "mandelbrotrenderer.h"
#include "regionattributes.h"

QT_BEGIN_NAMESPACE
class QImage;
QT_END_NAMESPACE

using namespace MandelBrotRenderer;

class ComputedDataSegment
{
public:
    ComputedDataSegment();

    explicit ComputedDataSegment(RegionAttributes attributes,
                        QImage *image,
                        int segmentIndex,
                        MQuintVector& bufferedResultData,
                        RegionAttributes bufferedAttributes);

    virtual ~ComputedDataSegment();

    ComputedDataSegment(const ComputedDataSegment& other);

    ComputedDataSegment(ComputedDataSegment&& other) noexcept;

    ComputedDataSegment& operator=(const ComputedDataSegment& other);

    double getScaleFactor() const { return attributes.getScaleFactor(); }
    double getCenterX() const { return attributes.getCenterX(); }
    double getCenterY() const { return attributes.getCenterY(); }
    int getMinX() const { return attributes.getMinX(); }
    int getMaxX() const { return attributes.getMaxX(); }
    int getMinY() const { return attributes.getMinY(); }
    int getMaxY() const { return attributes.getMaxY(); }
    QImage * getImage() const { return image; }
    int getFullHeight() const { return attributes.getFullHeight(); }
    ComputeTaskResults& getFullResultData() { return taskResults; }
    MQuintVector& getRawResultData() { return rawResultData; }
    MQuintVector&& extractRawResultData() { consumed = true; return std::move(rawResultData); }
    void clearRawData();

    int getSegmentIndex() const { return segmentIndex; }
    bool hasBeenUsed() const { return consumed; }
    void clearSegmentIndex() { segmentIndex = nonExistentThreadIndex; }

    const RegionAttributes& getAttributes() const;

    void ChangeRegionAttributes(const RegionAttributes& newRegionAttributes);

private:
    QImage *image;
    int rawDataSize;
    MQuintVector rawResultData;
    ComputeTaskResults taskResults;
    int segmentIndex;
    bool consumed;

    RegionAttributes attributes;

    static int count;
};

#endif // COMPUTEDDATASEGMENT_H
