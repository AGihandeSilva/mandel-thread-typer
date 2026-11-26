#include "computeddatasegment.h"
#include <QImage>
#include <iostream>
#include <QMutex>
#include <memory_resource>

using MandelBrotRenderer::notYetInitializedValue;

ComputedDataSegment::ComputedDataSegment(MandelBrotRenderer::uintVectorAllocator& allocator) :
        image(nullptr), rawDataSize(notYetInitializedValue), taskResults {nullptr, 0 },
        segmentIndex(MandelBrotRenderer::nonExistentSegmentIndex), consumed(false), m_allocator(allocator)
{
    QMutexLocker locker(getMutex());
    ++count;
}

int ComputedDataSegment::count = 0;

ComputedDataSegment::ComputedDataSegment(RegionAttributes attributes,
                    QImage *image,
                    int segmentIndex,
                    MandelBrotRenderer::PooledUintVector& bufferedResultData,
                    const RegionAttributes& bufferedAttributes,
    MandelBrotRenderer::uintVectorAllocator& allocator)
    :    image(image),

         rawDataSize(attributes.computeRawDataSize()),
         taskResults {nullptr, 0},
         segmentIndex(segmentIndex),
         consumed(false),
         attributes(attributes),
         m_allocator(allocator)
{
    QMutexLocker locker(getMutex());
    ++count;

    //TODO : check this for efficiency / memory leaks
    if (bufferedResultData.size() == rawDataSize &&
        attributes == bufferedAttributes)
    {
        //std::cout << "reusing raw results!" << std::endl;
        m_rawResultData =  bufferedResultData;
    }
    else
    {
        //std::cout << "cannot reuse raw results!" << std::endl;
        m_rawResultData = MandelBrotRenderer::PooledUintVector(rawDataSize, qRgb(0, 0, 0), m_allocator);
#if defined(DEBUG_RAW_DATA)
        m_rawIterationsData = MandelBrotRenderer::PooledUintVector(attributes.getMaxX() + 1 - attributes.getMinX(), 0, m_allocator);
#endif
    }

    taskResults.m_rawResultData = &m_rawResultData;
}

ComputedDataSegment::~ComputedDataSegment()
{
    QMutexLocker locker(getMutex());
    if (count-- == 0)
    {
        std::cout << "data segment delete error!" << std::endl;
    }
    m_rawResultData.clear();
}

ComputedDataSegment::ComputedDataSegment(ComputedDataSegment&& other) noexcept
    : image{ other.image },
    rawDataSize{ other.rawDataSize },
      m_rawResultData {std::move(other.m_rawResultData)},
#if defined(DEBUG_RAW_DATA)
    m_rawIterationsData{ std::move(other.m_rawIterationsData) },
#endif
      taskResults { &m_rawResultData, 
#if defined(DEBUG_RAW_DATA)
        &m_rawIterationsData,
#endif   
    other.taskResults.m_iterationSum},
      segmentIndex(other.segmentIndex),

      consumed(other.consumed),
      attributes(std::move(other.attributes)),
      m_allocator(other.m_allocator)
{
    QMutexLocker locker(getMutex());
    ++count;
}

ComputedDataSegment::ComputedDataSegment(const ComputedDataSegment& other)
    : image{ other.image },
    rawDataSize{ other.rawDataSize },
      m_rawResultData(other.m_rawResultData),
#if defined(DEBUG_RAW_DATA)
    m_rawIterationsData(other.m_rawIterationsData),
#endif
      taskResults { &m_rawResultData, 
 #if defined(DEBUG_RAW_DATA)
        &m_rawIterationsData,
#endif
        other.taskResults.m_iterationSum},
      segmentIndex(other.segmentIndex),
      consumed(other.consumed),
      attributes(other.attributes),
      m_allocator(other.m_allocator)
{
    QMutexLocker locker(getMutex());
    ++count;
}

ComputedDataSegment& ComputedDataSegment::operator=(const ComputedDataSegment& other)
{
    QMutexLocker locker(getMutex());
    image = other.image;
    rawDataSize = other.rawDataSize;
    m_rawResultData = other.m_rawResultData;
    taskResults.m_rawResultData = &m_rawResultData;
    taskResults.m_iterationSum = other.taskResults.m_iterationSum;
    segmentIndex = other.segmentIndex;
    consumed = other.consumed;
    attributes = other.attributes;

    return *this;
}

ComputedDataSegment& ComputedDataSegment::operator=(ComputedDataSegment&& other) noexcept
{
    QMutexLocker locker(getMutex());
    image = other.image;
    rawDataSize = other.rawDataSize;
    m_rawResultData = MandelBrotRenderer::PooledUintVector(std::move(other.m_rawResultData));
    taskResults.m_rawResultData = &m_rawResultData;
    taskResults.m_iterationSum = other.taskResults.m_iterationSum;
    segmentIndex = other.segmentIndex;
    consumed = other.consumed;
    attributes = other.attributes;

    return *this;
}

QMutex* ComputedDataSegment::getMutex()
{
    static QMutex mutex;

    return &mutex;
}

void ComputedDataSegment::clearRawData()
{
   QMutexLocker locker(getMutex());
   m_rawResultData.assign(m_rawResultData.size(), qRgb(0, 0, 0));
   taskResults.m_iterationSum = 0;
}

const RegionAttributes& ComputedDataSegment::getAttributes() const
{
    return attributes;
}

void ComputedDataSegment::ChangeRegionAttributes(const RegionAttributes& newRegionAttributes)
{
    QMutexLocker locker(getMutex());
    attributes = newRegionAttributes;
    rawDataSize =  newRegionAttributes.computeRawDataSize();
    m_rawResultData.resize(rawDataSize);
    taskResults.m_rawResultData = &m_rawResultData;
}

