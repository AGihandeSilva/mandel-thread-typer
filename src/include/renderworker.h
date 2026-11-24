#ifndef RENDER_WORKER_H
#define RENDER_WORKER_H

#include <QThread>
#include <QMutex>
#include <QVector>

#include "computeddatasegment.h"
#include "workerthreaddata.h"
#include "mandelbrotrenderer.h"
#include "MandelParams.h"


QT_BEGIN_NAMESPACE
class QImage;
QT_END_NAMESPACE

class MandelbrotWidget;
class RenderThread;


class RenderWorker : public QObject
{
    Q_OBJECT

public:
    explicit RenderWorker(
                       RenderThread* parentThread,
                       MandelbrotWidget* owner,
                       uint currentPassValue,
                       uint finalPassValue,
                       MandelBrotRenderer::colorMapStore& colormap,
                       bool restart,
                       const bool& abort,
                       int threadIndex,
                       ComputedDataSegment&& segment,
                       MandelBrotRenderer::haltChecker abortChecker,
                       QMutex&  pauseMutex);

    ~RenderWorker() override = default;
    RenderWorker(const RenderWorker&) = delete;
    RenderWorker(RenderWorker&&) = delete;
    RenderWorker& operator=(const RenderWorker&) = delete;
    RenderWorker& operator=(RenderWorker&&) = delete;

    using computeFunction = std::function<void (const ComputedDataSegment &, const bool &, int &, MandelBrotRenderer::ComputeTaskResults &, int)>;

    uint getPassValue() const { return pass; }

    void setCleanedUp() { cleanedUp = true; }
    bool isCleanedUp() const { return cleanedUp; }

    ComputedDataSegment& getComputedData();
    WorkerThreadData&& transferWorkerThreadData() { return (std::move(internalData)); }

    int getThreadIndex() const;

    void setSegment(const ComputedDataSegment &value);
    void publishState(MandelBrotRenderer::threadState state);
    void shareTask();

    static uint calcMaxIterations(uint pass) { return ((1 << (2 * pass + 6)) + 32); }

    MandelBrotRenderer::setType getSetToGenerate() const;

    MandelBrotRenderer::colorMapStore &getColormap() const;

    double getIterationColourScale() const;

public slots:
    void setRestart(bool restart);

signals:
void renderedSubImage(const QImage *image, QString scaleFactor);
void computationDone(ComputedDataSegment& data);
void taskDone();
void finished();
void writeToLog(QString outputString, bool show = false) const;

private:
    RenderThread* parentThread;
    WorkerThreadData internalData;

    std::future<bool> m_computeResult;

    bool execute(const computeFunction& computeTask);

    bool getComputeResult() ;


#ifdef MANDELBROT_USE_SFINAE
    template <typename T>
    typename std::enable_if <!std::is_same<T, MandelBrotRenderer::Int128>::value && 
            MandelParams::needs_scale_shift<T>::value, bool>::type
    executeTask();
#else
    template <typename T> requires NeedsScaleShift<T>
    bool executeTask();
#endif 

#ifdef MANDELBROT_USE_SFINAE
    template <typename T, int Scaling = 0>
    typename std::enable_if<!MandelParams::needs_scale_shift<T>::value&&
        MandelParams::multiply_by_float_supported<T>::value, bool>::type
    executeTask();
#else
    template <typename T, int Scaling = 0> requires IsFloatLikeType<T>
    bool executeTask();
#endif

#ifdef MANDELBROT_USE_SFINAE
    template <typename T>
    typename std::enable_if<std::is_same<T, MandelBrotRenderer::Int128>::value, bool>::type
    executeTask();
#endif

    uint pass;
    const uint finalPassValue;
    const uint MaxMaxIterations;
    const double iterationColourScale;

    MandelBrotRenderer::colorMapStore& colormap;

    bool restart;
    const bool& abort;
    int threadIndex;
    ComputedDataSegment segment;

    int currentYPosition;

    QMutex mutex;
    QMutex GUImutex;
    QMutex& pauseMutex;

    int pointsDone;
    bool cleanedUp;
    bool computationCompleted;

    MandelBrotRenderer::setType setToGenerate;

    MandelbrotWidget* m_ownerWidget;
    MandelBrotRenderer::haltChecker abortChecker;
    void handleSegmentDone();

    static int count;

    static constexpr int MIN_REALLOCATION_SIZE_IN_PIXELS = 4;

    static constexpr int MAGNITUDE_BITS = 3;
};

#if !defined(MANDELBROT_USE_SFINAE)
template <>
bool RenderWorker::executeTask< MandelBrotRenderer::Int128 >();
#endif

#endif // RENDER_WORKER_H
