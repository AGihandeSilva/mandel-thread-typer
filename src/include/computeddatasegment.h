#ifndef COMPUTEDDATASEGMENT_H
#define COMPUTEDDATASEGMENT_H

#include <QVector>
#include "mandelbrotrenderer.h"
#include "regionattributes.h"

QT_BEGIN_NAMESPACE
class QImage;
QT_END_NAMESPACE

/*
 *A data object class containing compute task parameters
 * and containers for computed results
*/

class ComputedDataSegment
{
public:
    ComputedDataSegment();

    explicit ComputedDataSegment(RegionAttributes attributes,
                        QImage *image,
                        int segmentIndex,
                        MandelBrotRenderer::MQuintVector& bufferedResultData,
                        const RegionAttributes& bufferedAttributes);

    virtual ~ComputedDataSegment();

    ComputedDataSegment(const ComputedDataSegment& other);

    ComputedDataSegment(ComputedDataSegment&& other) noexcept;

    ComputedDataSegment& operator=(const ComputedDataSegment& other);

    ComputedDataSegment& operator=(ComputedDataSegment&& other) noexcept;

    double getScaleFactor() const { return attributes.getScaleFactor(); }
    double getCenterX() const { return attributes.getCenterX(); }
    double getCenterY() const { return attributes.getCenterY(); }
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
    QString getPreciseCenterX() const { return attributes.getPreciseCenterX(); }
    QString getPreciseCenterY() const { return attributes.getPreciseCenterY(); }
#endif
    int getMinX() const { return attributes.getMinX(); }
    int getMaxX() const { return attributes.getMaxX(); }
    int getMinY() const { return attributes.getMinY(); }
    int getMaxY() const { return attributes.getMaxY(); }
    QImage * getImage() const { return image; }
    int getFullHeight() const { return attributes.getFullHeight(); }
    MandelBrotRenderer::ComputeTaskResults& getFullResultData() { return taskResults; }
    MandelBrotRenderer::MQuintVector& getRawResultData() { return rawResultData; }
    MandelBrotRenderer::MQuintVector&& extractRawResultData() { consumed = true; return std::move(rawResultData); }
    void clearRawData();

    int getSegmentIndex() const { return segmentIndex; }
    bool hasBeenUsed() const { return consumed; }
    void clearSegmentIndex() { segmentIndex = MandelBrotRenderer::nonExistentThreadIndex; }

    const RegionAttributes& getAttributes() const;

    void ChangeRegionAttributes(const RegionAttributes& newRegionAttributes);

private:
    QImage *image;
    int rawDataSize;
    MandelBrotRenderer::MQuintVector rawResultData;
    MandelBrotRenderer::ComputeTaskResults taskResults;
    int segmentIndex;
    bool consumed;

    RegionAttributes attributes;

    static int count;
};

#endif // COMPUTEDDATASEGMENT_H
