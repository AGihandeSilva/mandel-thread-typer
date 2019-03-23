#include "computeddatasegment.h"
#include <QImage>
#include <iostream>

using MandelBrotRenderer::notYetInitializedValue;

ComputedDataSegment::ComputedDataSegment() :
        image(nullptr), rawDataSize(notYetInitializedValue), taskResults {nullptr, 0 },
        segmentIndex(MandelBrotRenderer::nonExistentThreadIndex), consumed(false)
{
    ++count;
}

int ComputedDataSegment::count = 0;

ComputedDataSegment::ComputedDataSegment(RegionAttributes attributes,
                    QImage *image,
                    int segmentIndex,
                    MandelBrotRenderer::MQuintVector& bufferedResultData,
                    const RegionAttributes& bufferedAttributes)
    :    image(image),

         rawDataSize(attributes.computeRawDataSize()),
         taskResults {nullptr, 0},
         segmentIndex(segmentIndex),
         consumed(false),
         attributes(attributes)
{
    ++count;

    //TODO : check this for efficiency / memory leaks
    if (bufferedResultData.size() == rawDataSize &&
        attributes == bufferedAttributes)
    {
        //std::cout << "reusing raw results!" << std::endl;
        rawResultData =  bufferedResultData;
    }
    else
    {
        //std::cout << "cannot reuse raw results!" << std::endl;
        rawResultData = MandelBrotRenderer::MQuintVector(rawDataSize, qRgb(0, 0, 0));
    }

    taskResults.rawResultData = &rawResultData;
}

ComputedDataSegment::~ComputedDataSegment()
{
    if (count-- == 0)
    {
        std::cout << "data segment delete error!" << std::endl;
    }
    rawResultData.clear();
}

ComputedDataSegment::ComputedDataSegment(ComputedDataSegment&& other) noexcept
    : image{ other.image },
    rawDataSize{ other.rawDataSize },
      rawResultData {std::move(other.rawResultData)},
      taskResults { &rawResultData, other.taskResults.iterationSum},
      segmentIndex(other.segmentIndex),

      consumed(other.consumed),
      attributes(std::move(other.attributes))
{
    ++count;
}

ComputedDataSegment::ComputedDataSegment(const ComputedDataSegment& other)
    : image{ other.image },
    rawDataSize{ other.rawDataSize },
      rawResultData(other.rawResultData),
      taskResults { &rawResultData, other.taskResults.iterationSum},
      segmentIndex(other.segmentIndex),
      consumed(other.consumed),
      attributes(other.attributes)
{
    ++count;
}

ComputedDataSegment& ComputedDataSegment::operator=(const ComputedDataSegment& other)
{
    image = other.image;
    rawDataSize = other.rawDataSize;
    rawResultData = other.rawResultData;
    taskResults.rawResultData = &rawResultData;
    taskResults.iterationSum = other.taskResults.iterationSum;
    segmentIndex = other.segmentIndex;
    consumed = other.consumed;
    attributes = other.attributes;

    return *this;
}

ComputedDataSegment& ComputedDataSegment::operator=(ComputedDataSegment&& other) noexcept
{
    image = other.image;
    rawDataSize = other.rawDataSize;
    rawResultData = MandelBrotRenderer::MQuintVector(std::move(other.rawResultData));
    taskResults.rawResultData = &rawResultData;
    taskResults.iterationSum = other.taskResults.iterationSum;
    segmentIndex = other.segmentIndex;
    consumed = other.consumed;
    attributes = other.attributes;

    return *this;
}

void ComputedDataSegment::clearRawData()
{
   rawResultData.fill(qRgb(0, 0, 0));
   taskResults.iterationSum = 0;
}

const RegionAttributes& ComputedDataSegment::getAttributes() const
{
    return attributes;
}

void ComputedDataSegment::ChangeRegionAttributes(const RegionAttributes& newRegionAttributes)
{
    attributes = newRegionAttributes;
    rawDataSize =  newRegionAttributes.computeRawDataSize();
    rawResultData.resize(rawDataSize);
    taskResults.rawResultData = &rawResultData;
}

