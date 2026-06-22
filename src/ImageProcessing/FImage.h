#ifndef FIMAGE_H
#define FIMAGE_H
#include <QImage>
#include <QtMath>

// ══════════════════════════════════════════════════════════════
//  WASM-only：OpenCV 类型枚举 + EM_JS 函数声明（已弃用 OpenCV.js，保留代码以备恢复）
// ══════════════════════════════════════════════════════════════
#if 0 // was: #ifdef __EMSCRIPTEN__ — OpenCV.js 声明（已禁用）
namespace WasmCV
{
    namespace cv
    {
        enum class Type : int
        {
            CV_8UC1 = 0, // 8-bit unsigned,  1 channel
            CV_8UC2 = 8, // 8-bit unsigned,  2 channels
            CV_8UC3 = 16, // 8-bit unsigned,  3 channels
            CV_8UC4 = 24, // 8-bit unsigned,  4 channels (RGBA)

            CV_8SC1 = 1, // 8-bit signed,    1 channel
            CV_8SC2 = 9, // 8-bit signed,    2 channels
            CV_8SC3 = 17, // 8-bit signed,    3 channels
            CV_8SC4 = 25, // 8-bit signed,    4 channels

            CV_16UC1 = 2, // 16-bit unsigned, 1 channel
            CV_16UC2 = 10, // 16-bit unsigned, 2 channels
            CV_16UC3 = 18, // 16-bit unsigned, 3 channels
            CV_16UC4 = 26, // 16-bit unsigned, 4 channels

            CV_16SC1 = 3, // 16-bit signed,   1 channel
            CV_16SC2 = 11, // 16-bit signed,   2 channels
            CV_16SC3 = 19, // 16-bit signed,   3 channels
            CV_16SC4 = 27, // 16-bit signed,   4 channels

            CV_32SC1 = 4, // 32-bit signed,   1 channel
            CV_32SC2 = 12, // 32-bit signed,   2 channels
            CV_32SC3 = 20, // 32-bit signed,   3 channels
            CV_32SC4 = 28, // 32-bit signed,   4 channels

            CV_32FC1 = 5, // 32-bit float,    1 channel
            CV_32FC2 = 13, // 32-bit float,    2 channels
            CV_32FC3 = 21, // 32-bit float,    3 channels
            CV_32FC4 = 29, // 32-bit float,    4 channels

            CV_64FC1 = 6, // 64-bit float,    1 channel
            CV_64FC2 = 14, // 64-bit float,    2 channels
            CV_64FC3 = 22, // 64-bit float,    3 channels
            CV_64FC4 = 30, // 64-bit float,    4 channels
        };

        constexpr int depthOf(Type t) { return static_cast<int>(t) & 7; }
        constexpr int channelsOf(Type t) { return (static_cast<int>(t) >> 3) + 1; }
    } // namespace cv

    extern "C"
    {
    void gaussianBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);

    void horizontalGaussianBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);

    void verticalGaussianBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);

    void uniformBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);

    void horizontalUniforBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);

    void verticalUniforBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);

    void boxBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);

    void approxGaussianBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);

    void medianBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);
    } // extern "C"
} // namespace WasmCV
#endif // was: __EMSCRIPTEN__ — OpenCV.js 声明（已禁用）

class QString;
class QPixmap;

class FImage
{
public:
    FImage() = default;

    explicit FImage(const QString &fileName, const char *format = nullptr);

    explicit FImage(const QImage &image);

    explicit FImage(QImage &&other) noexcept;

    explicit FImage(const FImage &fimage);

    explicit FImage(FImage &&fimage) noexcept;

    FImage &operator=(const FImage &fimage);

    FImage &operator=(FImage &&fimage) noexcept;

    FImage &operator=(const QImage &qimage);

    FImage &operator=(QImage &&qimage) noexcept;

    operator QImage() const;

    // ── 跨平台接口 ──
    FImage &gaussianBlur(int radius = 30); // 高斯模糊
    FImage &boxBlur(int radius = 30); // 方框模糊（cv.boxFilter / 手动 box filter）
    FImage &approxGaussianBlur(int radius = 30); // 近似高斯模糊（3 趟 box filter 叠加）
    FImage &impulseNoise(double noiseRatio = 0.3); // 椒盐噪声
    FImage &greyScale(); // 8bit 单通道灰度图

// #ifdef __EMSCRIPTEN__
    // ── 方向性模糊接口（全平台使用手动实现，不依赖 OpenCV.js） ──
    FImage &horizontalGaussianBlur(int radius = 30);
    FImage &verticalGaussianBlur(int radius = 30);
    FImage &uniformBlur(int radius = 30);
    FImage &horizontalUniforBlur(int radius = 30);
    FImage &verticalUniforBlur(int radius = 30);
    FImage &medianBlur(int radius = 30);
// #endif

    [[nodiscard]] QPixmap toQPixmap() const;

    QImage &qImage() { return _qimage; }

private:
    QImage _qimage;

// #ifdef __EMSCRIPTEN__
//     static WasmCV::cv::Type toCvType(QImage::Format fmt);
// #endif
    bool ensureProcessable();
};

#endif // FIMAGE_H
