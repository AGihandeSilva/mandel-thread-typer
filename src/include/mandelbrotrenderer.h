#ifndef MANDELBROTRENDERER_H
#define MANDELBROTRENDERER_H

#include <QtGlobal>
#include <functional>
#include <QSize>
#include <QPoint>
#include <QDataStream>
#include <QString>

#include <vector>
#include <map>

#ifdef __GNUC__
#include <quadmath.h>
#endif

#if defined(_WIN32)
#include <intrin.h>
#endif

#include <ios>
#include <sstream>

#if (USE_BOOST_MULTIPRECISION == 1)
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#endif



namespace MandelBrotRenderer
{
    uint rgbFromWaveLength(double wave);

    enum class computeState { computeIdle, computeRunning, computeHalting };
    enum class threadState { disabled = 0, starting = 1, idle = 2, waiting = 3, busy = 4, shared = 5, restarted = 6, finishing = 7 };
    enum class threadAlgorithm { uniform, dynamic };
    enum class dynamicThreadAction { requestTask, splitTask };

    enum class internalDataType { singlePrecisionFloat = 0, doublePrecisionFloat = 1, customFloat20 =  2,
                                  float20dd = 3, float30dd = 4, float50dd = 5,

                                 float80 = 6, float128 = 7, int32 = 8, int64 = 9, int128 = 10, unknownType = -1 };

    enum class setType { mandelbrot = 0, julia = 1 };

    using haltChecker   = std::function<bool(void)>;
    using atomicInt     = std::atomic<uint>;
    using MQuintVector  = QVector<uint>;
    using MQintVector   = QVector<int>;
#if defined(_WIN32)
#if (USE_BOOST_MULTIPRECISION == 1)
    using Int128    = boost::multiprecision::int128_t;
    using Float80   = boost::multiprecision::cpp_bin_float_double_extended;
    using Float128  = boost::multiprecision::cpp_bin_float_quad;
#endif //USE_BOOST_MULTIPRECISION
#else
    using Int128   = __int128_t;
    using Float80  = __float80;
#if (USE_BOOST_MULTIPRECISION == 1)
    using Float128 = boost::multiprecision::cpp_bin_float_quad;
#else
    using Float128 = __float128;
#endif
#endif //_WIN32

using CoordValue = QString;

using CoordResult = std::pair<QString, bool>;
using DoubleResult = std::pair<double, bool>;
#if (USE_BOOST_MULTIPRECISION == 1) || defined(__GNUC__)
using PreciseFloatResult = std::pair<Float128, bool>;
#endif

#if (USE_BOOST_MULTIPRECISION == 1)
    using CustomFloat = boost::multiprecision::number<boost::multiprecision::cpp_bin_float<18, boost::multiprecision::backends::digit_base_10, void, boost::int16_t, -63, 64>>;
    using Float20dd = boost::multiprecision::number<boost::multiprecision::cpp_bin_float<20>>;
    using Float30dd = boost::multiprecision::number<boost::multiprecision::cpp_bin_float<30>>;
    using Float50dd = boost::multiprecision::cpp_bin_float_50;
#endif //USE_BOOST_MULTIPRECISION

    using TypePrecisonMap = std::map<internalDataType, int>;

    constexpr double    notYetInitializedDouble = -1.0;
    constexpr int       notYetInitializedInt    = -1;
    constexpr int64_t   notYetInitializedInt64  = -1;

    constexpr int       detailedValueStringLength = 100;

    static constexpr int nonExistentThreadIndex = -1;
    static constexpr int nonExistentPixelLinePosition = -1;
    static constexpr int notYetInitializedValue = 0x0BADCAFE;
    static constexpr int MIN_NUM_UNUSED_THREADS = 2;
    static constexpr int MIN_NUM_WORKER_THREADS = 1;
    static constexpr int MAX_NUM_WORKER_THREADS = 10;
    static constexpr int MAX_PASSES = 8;
    static constexpr int defaultNumPassesValue = MAX_PASSES - 2;
    static constexpr int TRANSIENT_MESSAGE_DURATION_IN_MS = 1500;

    static constexpr size_t numStates = 8;
    static constexpr uint DefaultColormapSize = 4096;

    static constexpr int MAX_RENDER_SIZE_X = 4096;
    static constexpr int MAX_RENDER_SIZE_Y = 3072;

    //misc Qt GUI parameters
    static constexpr int ThreadIndicatorFrameWidth = 4;

    static constexpr internalDataType defaultRendererType = internalDataType::doublePrecisionFloat;

    //using colorMapStore = MQuintVector;
    using colorMapStore = std::vector<uint>;

#ifdef _WIN32
    using std::enable_if_t;
#else
 template <bool Condition, typename T = void>
    using enable_if_t = typename std::enable_if<Condition, T>::type;
#endif

    template <typename E>
    constexpr decltype(auto) toUnderlyingType(E enumerator) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(enumerator);
    }

    enum class boolDescriptionMode { on_off = 0, true_false = 1 };

    const QString& getBoolValueAsString(bool value, boolDescriptionMode mode = boolDescriptionMode::on_off);

    struct RendererData
    {
        int pendingNumWorkerThreads;
        int currentNumPassValue;
        int nextNumPassValue;
        bool threadMediatorEnabled;
        int colorMapSize;
        internalDataType numericType;
        int64_t iterationSumCount;
    };

    struct RenderState
    {
        /* InformationDisplay settings */
            bool detailedDisplayEnabled;

        /* MainWindow settings */
            QSize size;
            QPoint pos;

        /* RenderParameters settings */
            CoordValue originX;
            CoordValue originY;
            double curScale;
            double pixmapScale;

        /* Renderer settings */
            bool threadMediatorEnabled;
            int colorMapSize;
            internalDataType numericType;
    };

    struct ComputeTaskResults
    {
        MQuintVector*   rawResultData;
        int64_t         iterationSum;
    };

    QDataStream& operator << (QDataStream &outputStream, const RenderState& state);
    QDataStream& operator >> (QDataStream &inputStream, RenderState& state);

    using CoordinateListenerConfig = int;
    static constexpr int CoordinateListenerfullPrecision = -1;

    class CoordinateListener
    {
    public:
        CoordinateListener() = default;
        virtual ~CoordinateListener() = default;
        virtual void updateCoordData(const QString& originX, const QString& originY,
                                     const QString& width, const QString& height) = 0;
        CoordinateListener(const CoordinateListener&) = delete;
        CoordinateListener(CoordinateListener&&) = delete;
        CoordinateListener& operator= (const CoordinateListener&) = delete;
        CoordinateListener& operator= (CoordinateListener&&) = delete;
    };

    using ListenerGroup = std::vector<std::pair<MandelBrotRenderer::CoordinateListener*,
        MandelBrotRenderer::CoordinateListenerConfig>>;

    struct RegionLimits
    {
        explicit RegionLimits(double xMin, double xMax, double yMin, double yMax) : xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax) {}

        const double xMin;
        const double xMax;
        const double yMin;
        const double yMax;
    };

}

#endif // MANDELBROTRENDERER_H
