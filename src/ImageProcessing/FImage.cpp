#include "FImage.h"

#include <QDebug>
#include <QPixmap>
#include <QRandomGenerator>

// ══════════════════════════════════════════════════════════════
//  WASM 实现 — OpenCV.js（EM_JS 桥接）— 已弃用，改为手动实现
// ══════════════════════════════════════════════════════════════
#if 0 // was: #ifdef __EMSCRIPTEN__ — OpenCV.js EM_JS（已禁用）
#include <emscripten.h>

// ── gaussianBlur — 高斯模糊 ──
EM_JS(void,
      gaussianBlur_EMJS,
    (uintptr_t bitsPtr,
      int type,
      int w,
      int h,
      int bytesPerLine,
      int kernel),
{
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.GaussianBlur(mat, mat, new cv.Size(kernel, kernel), 0);
    HEAPU8.set(mat.data, bitsPtr);
    mat.
    delete();
});

// ── horizontalGaussianBlur — 水平高斯模糊 ──
EM_JS(void,
      horizontalGaussianBlur_EMJS,
    (uintptr_t bitsPtr,
      int type,
      int w,
      int h,
      int bytesPerLine,
      int kernel),
{
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.GaussianBlur(mat, mat, new cv.Size(kernel, 1), 0);
    HEAPU8.set(mat.data, bitsPtr);
    mat.
    delete();
});

// ── verticalGaussianBlur — 垂直高斯模糊 ──
EM_JS(void,
      verticalGaussianBlur_EMJS,
    (uintptr_t bitsPtr,
      int type,
      int w,
      int h,
      int bytesPerLine,
      int kernel),

{
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.GaussianBlur(mat, mat, new cv.Size(1, kernel), 0);
    HEAPU8.set(mat.data, bitsPtr);
    mat.
    delete();
});

// ── uniformBlur — 均匀模糊（cv.blur = Box 平均） ──
EM_JS(void,
      uniformBlur_EMJS,
    (uintptr_t bitsPtr,
      int type,
      int w,
      int h,
      int bytesPerLine,
      int kernel),
{
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.blur(mat, mat, new cv.Size(kernel, kernel));
    HEAPU8.set(mat.data, bitsPtr);
    mat.
    delete();
});

// ── horizontalUniforBlur — 水平均匀模糊 ──
EM_JS(void,
      horizontalUniforBlur_EMJS,
    (uintptr_t bitsPtr,
      int type,
      int w,
      int h,
      int bytesPerLine,
      int kernel),
{
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.blur(mat, mat, new cv.Size(kernel, 1));
    HEAPU8.set(mat.data, bitsPtr);
    mat.
    delete();
});

// ── verticalUniforBlur — 垂直均匀模糊 ──
EM_JS(void,
      verticalUniforBlur_EMJS,
    (uintptr_t bitsPtr,
      int type,
      int w,
      int h,
      int bytesPerLine,
      int kernel),
{
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.blur(mat, mat, new cv.Size(1, kernel));
    HEAPU8.set(mat.data, bitsPtr);
    mat.
    delete();
});

// ── boxBlur — 方框模糊（cv.boxFilter） ──
EM_JS(void,
      boxBlur_EMJS,
    (uintptr_t bitsPtr,
      int type,
      int w,
      int h,
      int bytesPerLine,
      int kernel),
{
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.boxFilter(mat, mat, -1, new cv.Size(kernel, kernel));
    HEAPU8.set(mat.data, bitsPtr);
    mat.
    delete();
});

// ── approxGaussianBlur — 近似高斯模糊：3 趟 boxFilter 叠加 ──
EM_JS(void,
      approxGaussianBlur_EMJS,
    (uintptr_t bitsPtr,
      int type,
      int w,
      int h,
      int bytesPerLine,
      int kernel),
{
    var ksize = new cv
    .
    Size(kernel, kernel);
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);

    cv.boxFilter(mat, mat, -1, ksize);
    cv.boxFilter(mat, mat, -1, ksize);
    cv.boxFilter(mat, mat, -1, ksize);
    HEAPU8.set(mat.data, bitsPtr);
    mat.
    delete();
});

// ── medianBlur — 中值滤波 ──
EM_JS(void,
      medianBlur_EMJS,
    (uintptr_t bitsPtr,
      int type,
      int w,
      int h,
      int bytesPerLine,
      int kernel),
{
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.medianBlur(mat, mat, kernel);
    HEAPU8.set(mat.data, bitsPtr);
    mat.
    delete();
});

// ══════════════════════════════════════════════════════════════
//  WASM — FImage 方法实现
// ══════════════════════════════════════════════════════════════

WasmCV::cv::Type FImage::toCvType(const QImage::Format fmt)
{
    switch (fmt)
    {
        case QImage::Format_Grayscale8 :
            return WasmCV::cv::Type::CV_8UC1;
        case QImage::Format_Grayscale16 :
            return WasmCV::cv::Type::CV_16UC1;
        case QImage::Format_RGB888 :
            return WasmCV::cv::Type::CV_8UC3;
        case QImage::Format_RGBA64 :
            return WasmCV::cv::Type::CV_16UC4;
        case QImage::Format_ARGB32 :
        case QImage::Format_RGB32 :
        case QImage::Format_ARGB32_Premultiplied :
            return WasmCV::cv::Type::CV_8UC4;
        default :
            qWarning() << "FImage: unsupported format" << static_cast<int>(fmt)
                    << ", fallback to CV_8UC4";
            return WasmCV::cv::Type::CV_8UC4;
    }
}

FImage &FImage::gaussianBlur(int radius)
{
    if (radius < 0)
    {
        qWarning() << "The blur radius must be >= 0";
        return *this;
    }
    if (radius == 0)
        return *this;
    if (!ensureProcessable())
        return *this;

    gaussianBlur_EMJS(reinterpret_cast<uintptr_t>(_qimage.bits()),
                      static_cast<int>(toCvType(_qimage.format())),
                      _qimage.width(),
                      _qimage.height(),
                      _qimage.bytesPerLine(),
                      radius * 2 + 1
    );
    return *this;
}

FImage &FImage::horizontalGaussianBlur(int radius)
{
    if (radius <= 0)
    {
        if (radius < 0)
            qWarning() << "The blur radius must be > 0";
        return *this;
    }
    if (!ensureProcessable())
        return *this;
    horizontalGaussianBlur_EMJS(reinterpret_cast<uintptr_t>(_qimage.bits()),
                                static_cast<int>(toCvType(_qimage.format())),
                                _qimage.width(),
                                _qimage.height(),
                                _qimage.bytesPerLine(),
                                radius * 2 + 1
    );
    return *this;
}

FImage &FImage::verticalGaussianBlur(int radius)
{
    if (radius <= 0)
    {
        if (radius < 0)
            qWarning() << "The blur radius must be >= 0";
        return *this;
    }
    if (!ensureProcessable())
        return *this;
    verticalGaussianBlur_EMJS(reinterpret_cast<uintptr_t>(_qimage.bits()),
                              static_cast<int>(toCvType(_qimage.format())),
                              _qimage.width(),
                              _qimage.height(),
                              _qimage.bytesPerLine(),
                              radius * 2 + 1
    );
    return *this;
}

FImage &FImage::uniformBlur(int radius)
{
    if (radius <= 0)
    {
        if (radius < 0)
            qWarning() << "The blur radius must be > 0";
        return *this;
    }
    if (!ensureProcessable())
        return *this;
    uniformBlur_EMJS(reinterpret_cast<uintptr_t>(_qimage.bits()),
                     static_cast<int>(toCvType(_qimage.format())),
                     _qimage.width(),
                     _qimage.height(),
                     _qimage.bytesPerLine(),
                     radius
    );
    return *this;
}

FImage &FImage::horizontalUniforBlur(int radius)
{
    if (radius <= 0)
    {
        if (radius < 0)
            qWarning() << "The blur radius must be > 0";
        return *this;
    }
    if (!ensureProcessable())
        return *this;
    horizontalUniforBlur_EMJS(reinterpret_cast<uintptr_t>(_qimage.bits()),
                              static_cast<int>(toCvType(_qimage.format())),
                              _qimage.width(),
                              _qimage.height(),
                              _qimage.bytesPerLine(),
                              radius
    );
    return *this;
}

FImage &FImage::verticalUniforBlur(int radius)
{
    if (radius <= 0)
    {
        if (radius < 0)
            qWarning() << "The blur radius must be > 0";
        return *this;
    }
    if (!ensureProcessable())
        return *this;
    verticalUniforBlur_EMJS(reinterpret_cast<uintptr_t>(_qimage.bits()),
                            static_cast<int>(toCvType(_qimage.format())),
                            _qimage.width(),
                            _qimage.height(),
                            _qimage.bytesPerLine(),
                            radius
    );
    return *this;
}

FImage &FImage::boxBlur(int radius)
{
    if (radius < 0)
    {
        qWarning() << "The blur radius must be >= 0";
        return *this;
    }
    if (radius == 0)
        return *this;
    if (!ensureProcessable())
        return *this;

    boxBlur_EMJS(reinterpret_cast<uintptr_t>(_qimage.bits()),
                 static_cast<int>(toCvType(_qimage.format())),
                 _qimage.width(),
                 _qimage.height(),
                 _qimage.bytesPerLine(),
                 radius * 2 + 1
    );
    return *this;
}

FImage &FImage::approxGaussianBlur(int radius)
{
    if (radius < 0)
    {
        qWarning() << "approxGaussianBlur: radius must be >= 0";
        return *this;
    }
    if (radius == 0)
        return *this;
    if (!ensureProcessable())
        return *this;

    approxGaussianBlur_EMJS(reinterpret_cast<uintptr_t>(_qimage.bits()),
                            static_cast<int>(toCvType(_qimage.format())),
                            _qimage.width(),
                            _qimage.height(),
                            _qimage.bytesPerLine(),
                            radius * 2 + 1
    );
    return *this;
}

FImage &FImage::medianBlur(int radius)
{
    if (radius < 0)
    {
        qWarning() << "The blur radius must be >= 0";
        return *this;
    }
    if (radius == 0)
        return *this;
    if (!ensureProcessable())
        return *this;

    const int ksize = radius * 2 + 1;
    const int type = static_cast<int>(toCvType(_qimage.format()));
    const int depth = type & 7;
    const int channels = (type >> 3) + 1;

    if (channels == 2)
    {
        qWarning() << "FImage: medianBlur does not support 2-channel images";
        return *this;
    }
    if (ksize > 5 && depth != 0)
    {
        qWarning() << "FImage: medianBlur ksize>5 requires CV_8U, got depth=" << depth;
        return *this;
    }

    medianBlur_EMJS(reinterpret_cast<uintptr_t>(_qimage.bits()),
                    type,
                    _qimage.width(),
                    _qimage.height(),
                    _qimage.bytesPerLine(),
                    ksize
    );
    return *this;
}

#endif // was: __EMSCRIPTEN__ — OpenCV.js EM_JS（已禁用）


#if 1 // was: #ifdef Q_OS_WIN — 手动实现，全平台通用

// ── 内部辅助：高斯模糊（横向 + 纵向分离卷积） ──
static QImage gaussianBlurInternal(const QImage &src, const int radius)
{
    if (radius <= 0 || src.isNull())
        return src;

    const int width = src.width();
    const int height = src.height();
    const int kernelSize = 2 * radius + 1;
    const qreal sigma = 0.3 * ((kernelSize - 1) * 0.5 - 1) + 0.8;

    // 生成高斯核
    QVector<qreal> kernel(kernelSize);
    qreal sum = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        const qreal x = i - radius;
        const qreal g = qExp(-(x * x) / (2.0 * sigma * sigma));
        kernel[i] = g;
        sum += g;
    }
    for (auto &v: kernel)
        v /= sum;

    QImage temp(src.size(), QImage::Format_ARGB32);
    QImage dst(src.size(), QImage::Format_ARGB32);

    const int bytesPerLine = static_cast<int>(src.bytesPerLine());
    const uchar *srcBits = src.constBits();
    uchar *tempBits = temp.bits();
    uchar *dstBits = dst.bits();

    // ── 横向模糊 ──
    for (int y = 0; y < height; ++y)
    {
        const uchar *srcLine = srcBits + y * bytesPerLine;
        uchar *tempLine = tempBits + y * bytesPerLine;

        for (int x = 0; x < width; ++x)
        {
            double rSum = 0, gSum = 0, bSum = 0, aSum = 0;

            for (int k = -radius; k <= radius; ++k)
            {
                const int px = std::clamp(x + k, 0, width - 1);
                const uchar *p = srcLine + px * 4;
                const double w = kernel[k + radius];

                bSum += p[0] * w;
                gSum += p[1] * w;
                rSum += p[2] * w;
                aSum += p[3] * w;
            }

            uchar *d = tempLine + x * 4;
            d[0] = static_cast<uchar>(std::lround(bSum));
            d[1] = static_cast<uchar>(std::lround(gSum));
            d[2] = static_cast<uchar>(std::lround(rSum));
            d[3] = static_cast<uchar>(std::lround(aSum));
        }
    }

    // ── 纵向模糊 ──
    for (int y = 0; y < height; ++y)
    {
        uchar *dstLine = dstBits + y * bytesPerLine;

        for (int x = 0; x < width; ++x)
        {
            double rSum = 0, gSum = 0, bSum = 0, aSum = 0;

            for (int k = -radius; k <= radius; ++k)
            {
                const int py = std::clamp(y + k, 0, height - 1);
                const uchar *p = tempBits + py * bytesPerLine + x * 4;
                const double w = kernel[k + radius];

                bSum += p[0] * w;
                gSum += p[1] * w;
                rSum += p[2] * w;
                aSum += p[3] * w;
            }

            uchar *d = dstLine + x * 4;
            d[0] = static_cast<uchar>(std::lround(bSum));
            d[1] = static_cast<uchar>(std::lround(gSum));
            d[2] = static_cast<uchar>(std::lround(rSum));
            d[3] = static_cast<uchar>(std::lround(aSum));
        }
    }

    return dst;
}

// ── 内部辅助：方块模糊 / 均值滤波（滑动窗口优化，O(W×H)） ──
//    每像素 O(1)：首次求窗口和，之后 sum_new = sum_old − left + right
//    边界处理: std::clamp
static QImage boxBlurInternal(const QImage &src, const int kernelSize)
{
    if (kernelSize <= 1 || src.isNull())
        return src;

    const int width = src.width();
    const int height = src.height();
    const int radius = (kernelSize - 1) / 2;

    QImage temp(src.size(), QImage::Format_ARGB32);
    QImage dst(src.size(), QImage::Format_ARGB32);

    const int srcBpl = static_cast<int>(src.bytesPerLine());
    const int tempBpl = static_cast<int>(temp.bytesPerLine());
    const int dstBpl = static_cast<int>(dst.bytesPerLine());
    const uchar *srcBits = src.constBits();
    uchar *tempBits = temp.bits();
    uchar *dstBits = dst.bits();

    const double norm = 1.0 / kernelSize;

    // ── 横向滑动窗口 ──
    for (int y = 0; y < height; ++y)
    {
        const uchar *srcLine = srcBits + y * srcBpl;
        uchar *tempLine = tempBits + y * tempBpl;

        // 初始化窗口：[−radius, radius] → clamp 到 [0, width−1]
        double rSum = 0, gSum = 0, bSum = 0, aSum = 0;
        for (int k = -radius; k <= radius; ++k)
        {
            const int px = std::clamp(k, 0, width - 1);
            const uchar *p = srcLine + px * 4;
            bSum += p[0];
            gSum += p[1];
            rSum += p[2];
            aSum += p[3];
        }

        for (int x = 0; x < width; ++x)
        {
            uchar *d = tempLine + x * 4;
            d[0] = static_cast<uchar>(std::lround(bSum * norm));
            d[1] = static_cast<uchar>(std::lround(gSum * norm));
            d[2] = static_cast<uchar>(std::lround(rSum * norm));
            d[3] = static_cast<uchar>(std::lround(aSum * norm));

            // 滑动：减去最左像素，加入最右新像素
            const int leftX = std::clamp(x - radius, 0, width - 1);
            const int rightX = std::clamp(x + radius + 1, 0, width - 1);
            const uchar *lp = srcLine + leftX * 4;
            const uchar *rp = srcLine + rightX * 4;
            bSum -= lp[0];
            bSum += rp[0];
            gSum -= lp[1];
            gSum += rp[1];
            rSum -= lp[2];
            rSum += rp[2];
            aSum -= lp[3];
            aSum += rp[3];
        }
    }

    // ── 纵向滑动窗口 ──
    for (int x = 0; x < width; ++x)
    {
        // 初始化窗口
        double rSum = 0, gSum = 0, bSum = 0, aSum = 0;
        for (int k = -radius; k <= radius; ++k)
        {
            const int py = std::clamp(k, 0, height - 1);
            const uchar *p = tempBits + py * tempBpl + x * 4;
            bSum += p[0];
            gSum += p[1];
            rSum += p[2];
            aSum += p[3];
        }

        for (int y = 0; y < height; ++y)
        {
            uchar *d = dstBits + y * dstBpl + x * 4;
            d[0] = static_cast<uchar>(std::lround(bSum * norm));
            d[1] = static_cast<uchar>(std::lround(gSum * norm));
            d[2] = static_cast<uchar>(std::lround(rSum * norm));
            d[3] = static_cast<uchar>(std::lround(aSum * norm));

            const int topY = std::clamp(y - radius, 0, height - 1);
            const int bottomY = std::clamp(y + radius + 1, 0, height - 1);
            const uchar *tp = tempBits + topY * tempBpl + x * 4;
            const uchar *bp = tempBits + bottomY * tempBpl + x * 4;
            bSum -= tp[0];
            bSum += bp[0];
            gSum -= tp[1];
            gSum += bp[1];
            rSum -= tp[2];
            rSum += bp[2];
            aSum -= tp[3];
            aSum += bp[3];
        }
    }

    return dst;
}

// ══════════════════════════════════════════════════════════════
//  Windows — FImage 方法实现（仅 3 个跨平台接口）
// ══════════════════════════════════════════════════════════════

FImage &FImage::gaussianBlur(int radius)
{
    if (radius < 0)
    {
        qWarning() << "The blur radius must be greater than or equal to 0";
        return *this;
    }
    if (radius == 0)
        return *this;
    if (!ensureProcessable())
        return *this;

    _qimage = gaussianBlurInternal(_qimage, radius);
    return *this;
}

FImage &FImage::boxBlur(int radius)
{
    if (radius < 0)
    {
        qWarning() << "The blur radius must be greater than or equal to 0";
        return *this;
    }
    if (radius == 0)
        return *this;
    if (!ensureProcessable())
        return *this;

    _qimage = boxBlurInternal(_qimage, radius * 2 + 1);
    return *this;
}

FImage &FImage::approxGaussianBlur(int radius)
{
    if (radius < 0)
    {
        qWarning() << "approxGaussianBlur: radius must be >= 0";
        return *this;
    }
    if (radius == 0)
        return *this;
    if (!ensureProcessable())
        return *this;

    const int ksize = radius * 2 + 1;

    // 3 趟 box filter 叠加 — 与 WASM 版 cv.boxFilter × 3 逻辑一致
    _qimage = boxBlurInternal(_qimage, ksize);
    _qimage = boxBlurInternal(_qimage, ksize);
    _qimage = boxBlurInternal(_qimage, ksize);

    return *this;
}

// ══════════════════════════════════════════════════════════════
//  方向性模糊 — 手动实现（水平 / 垂直分离）
// ══════════════════════════════════════════════════════════════

// ── 内部辅助：水平方向高斯模糊（仅横向卷积） ──
static QImage horizontalGaussianBlurInternal(const QImage &src, const int radius)
{
    if (radius <= 0 || src.isNull())
        return src;

    const int width = src.width();
    const int height = src.height();
    const int kernelSize = 2 * radius + 1;
    const qreal sigma = 0.3 * ((kernelSize - 1) * 0.5 - 1) + 0.8;

    QVector<qreal> kernel(kernelSize);
    qreal sum = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        const qreal x = i - radius;
        const qreal g = qExp(-(x * x) / (2.0 * sigma * sigma));
        kernel[i] = g;
        sum += g;
    }
    for (auto &v: kernel)
        v /= sum;

    QImage dst(src.size(), QImage::Format_ARGB32);
    const int srcBpl = static_cast<int>(src.bytesPerLine());
    const int dstBpl = static_cast<int>(dst.bytesPerLine());
    const uchar *srcBits = src.constBits();
    uchar *dstBits = dst.bits();

    for (int y = 0; y < height; ++y)
    {
        const uchar *srcLine = srcBits + y * srcBpl;
        uchar *dstLine = dstBits + y * dstBpl;

        for (int x = 0; x < width; ++x)
        {
            double rSum = 0, gSum = 0, bSum = 0, aSum = 0;
            for (int k = -radius; k <= radius; ++k)
            {
                const int px = std::clamp(x + k, 0, width - 1);
                const uchar *p = srcLine + px * 4;
                const double w = kernel[k + radius];
                bSum += p[0] * w;
                gSum += p[1] * w;
                rSum += p[2] * w;
                aSum += p[3] * w;
            }
            uchar *d = dstLine + x * 4;
            d[0] = static_cast<uchar>(std::lround(bSum));
            d[1] = static_cast<uchar>(std::lround(gSum));
            d[2] = static_cast<uchar>(std::lround(rSum));
            d[3] = static_cast<uchar>(std::lround(aSum));
        }
    }
    return dst;
}

// ── 内部辅助：垂直方向高斯模糊（仅纵向卷积） ──
static QImage verticalGaussianBlurInternal(const QImage &src, const int radius)
{
    if (radius <= 0 || src.isNull())
        return src;

    const int width = src.width();
    const int height = src.height();
    const int kernelSize = 2 * radius + 1;
    const qreal sigma = 0.3 * ((kernelSize - 1) * 0.5 - 1) + 0.8;

    QVector<qreal> kernel(kernelSize);
    qreal sum = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        const qreal x = i - radius;
        const qreal g = qExp(-(x * x) / (2.0 * sigma * sigma));
        kernel[i] = g;
        sum += g;
    }
    for (auto &v: kernel)
        v /= sum;

    QImage dst(src.size(), QImage::Format_ARGB32);
    const int srcBpl = static_cast<int>(src.bytesPerLine());
    const int dstBpl = static_cast<int>(dst.bytesPerLine());
    const uchar *srcBits = src.constBits();
    uchar *dstBits = dst.bits();

    for (int y = 0; y < height; ++y)
    {
        uchar *dstLine = dstBits + y * dstBpl;
        for (int x = 0; x < width; ++x)
        {
            double rSum = 0, gSum = 0, bSum = 0, aSum = 0;
            for (int k = -radius; k <= radius; ++k)
            {
                const int py = std::clamp(y + k, 0, height - 1);
                const uchar *p = srcBits + py * srcBpl + x * 4;
                const double w = kernel[k + radius];
                bSum += p[0] * w;
                gSum += p[1] * w;
                rSum += p[2] * w;
                aSum += p[3] * w;
            }
            uchar *d = dstLine + x * 4;
            d[0] = static_cast<uchar>(std::lround(bSum));
            d[1] = static_cast<uchar>(std::lround(gSum));
            d[2] = static_cast<uchar>(std::lround(rSum));
            d[3] = static_cast<uchar>(std::lround(aSum));
        }
    }
    return dst;
}

// ── 内部辅助：水平方向方框模糊（仅横向滑动窗口） ──
static QImage horizontalBoxBlurInternal(const QImage &src, const int kernelSize)
{
    if (kernelSize <= 1 || src.isNull())
        return src;

    const int width = src.width();
    const int height = src.height();
    const int radius = (kernelSize - 1) / 2;
    const double norm = 1.0 / kernelSize;

    QImage dst(src.size(), QImage::Format_ARGB32);
    const int srcBpl = static_cast<int>(src.bytesPerLine());
    const int dstBpl = static_cast<int>(dst.bytesPerLine());
    const uchar *srcBits = src.constBits();
    uchar *dstBits = dst.bits();

    for (int y = 0; y < height; ++y)
    {
        const uchar *srcLine = srcBits + y * srcBpl;
        uchar *dstLine = dstBits + y * dstBpl;

        double rSum = 0, gSum = 0, bSum = 0, aSum = 0;
        for (int k = -radius; k <= radius; ++k)
        {
            const int px = std::clamp(k, 0, width - 1);
            const uchar *p = srcLine + px * 4;
            bSum += p[0]; gSum += p[1]; rSum += p[2]; aSum += p[3];
        }

        for (int x = 0; x < width; ++x)
        {
            uchar *d = dstLine + x * 4;
            d[0] = static_cast<uchar>(std::lround(bSum * norm));
            d[1] = static_cast<uchar>(std::lround(gSum * norm));
            d[2] = static_cast<uchar>(std::lround(rSum * norm));
            d[3] = static_cast<uchar>(std::lround(aSum * norm));

            const int leftX = std::clamp(x - radius, 0, width - 1);
            const int rightX = std::clamp(x + radius + 1, 0, width - 1);
            const uchar *lp = srcLine + leftX * 4;
            const uchar *rp = srcLine + rightX * 4;
            bSum -= lp[0]; bSum += rp[0];
            gSum -= lp[1]; gSum += rp[1];
            rSum -= lp[2]; rSum += rp[2];
            aSum -= lp[3]; aSum += rp[3];
        }
    }
    return dst;
}

// ── 内部辅助：垂直方向方框模糊（仅纵向滑动窗口） ──
static QImage verticalBoxBlurInternal(const QImage &src, const int kernelSize)
{
    if (kernelSize <= 1 || src.isNull())
        return src;

    const int width = src.width();
    const int height = src.height();
    const int radius = (kernelSize - 1) / 2;
    const double norm = 1.0 / kernelSize;

    QImage dst(src.size(), QImage::Format_ARGB32);
    const int srcBpl = static_cast<int>(src.bytesPerLine());
    const int dstBpl = static_cast<int>(dst.bytesPerLine());
    const uchar *srcBits = src.constBits();
    uchar *dstBits = dst.bits();

    for (int x = 0; x < width; ++x)
    {
        double rSum = 0, gSum = 0, bSum = 0, aSum = 0;
        for (int k = -radius; k <= radius; ++k)
        {
            const int py = std::clamp(k, 0, height - 1);
            const uchar *p = srcBits + py * srcBpl + x * 4;
            bSum += p[0]; gSum += p[1]; rSum += p[2]; aSum += p[3];
        }

        for (int y = 0; y < height; ++y)
        {
            uchar *d = dstBits + y * dstBpl + x * 4;
            d[0] = static_cast<uchar>(std::lround(bSum * norm));
            d[1] = static_cast<uchar>(std::lround(gSum * norm));
            d[2] = static_cast<uchar>(std::lround(rSum * norm));
            d[3] = static_cast<uchar>(std::lround(aSum * norm));

            const int topY = std::clamp(y - radius, 0, height - 1);
            const int bottomY = std::clamp(y + radius + 1, 0, height - 1);
            const uchar *tp = srcBits + topY * srcBpl + x * 4;
            const uchar *bp = srcBits + bottomY * srcBpl + x * 4;
            bSum -= tp[0]; bSum += bp[0];
            gSum -= tp[1]; gSum += bp[1];
            rSum -= tp[2]; rSum += bp[2];
            aSum -= tp[3]; aSum += bp[3];
        }
    }
    return dst;
}

// ── 内部辅助：中值滤波 ──
static QImage medianBlurInternal(const QImage &src, int kernelSize)
{
    if (kernelSize <= 1 || src.isNull())
        return src;

    const int width = src.width();
    const int height = src.height();
    const int radius = (kernelSize - 1) / 2;
    const int total = kernelSize * kernelSize;

    QImage dst(src.size(), QImage::Format_ARGB32);
    const int srcBpl = static_cast<int>(src.bytesPerLine());
    const int dstBpl = static_cast<int>(dst.bytesPerLine());
    const uchar *srcBits = src.constBits();
    uchar *dstBits = dst.bits();

    // 每通道独立排序取中值
    QVector<uchar> rVals(total), gVals(total), bVals(total), aVals(total);

    for (int y = 0; y < height; ++y)
    {
        uchar *dstLine = dstBits + y * dstBpl;
        for (int x = 0; x < width; ++x)
        {
            int idx = 0;
            for (int ky = -radius; ky <= radius; ++ky)
            {
                const int py = std::clamp(y + ky, 0, height - 1);
                const uchar *srcLine = srcBits + py * srcBpl;
                for (int kx = -radius; kx <= radius; ++kx)
                {
                    const int px = std::clamp(x + kx, 0, width - 1);
                    const uchar *p = srcLine + px * 4;
                    bVals[idx] = p[0];
                    gVals[idx] = p[1];
                    rVals[idx] = p[2];
                    aVals[idx] = p[3];
                    ++idx;
                }
            }
            std::sort(bVals.begin(), bVals.end());
            std::sort(gVals.begin(), gVals.end());
            std::sort(rVals.begin(), rVals.end());
            std::sort(aVals.begin(), aVals.end());

            const int mid = total / 2;
            uchar *d = dstLine + x * 4;
            d[0] = bVals[mid];
            d[1] = gVals[mid];
            d[2] = rVals[mid];
            d[3] = aVals[mid];
        }
    }
    return dst;
}

// ══════════════════════════════════════════════════════════════
//  方向性模糊 FImage 方法（手动实现，全平台通用）
// ══════════════════════════════════════════════════════════════

FImage &FImage::horizontalGaussianBlur(const int radius)
{
    if (radius <= 0)
    {
        if (radius < 0)
            qWarning() << "horizontalGaussianBlur: radius must be > 0";
        return *this;
    }
    if (!ensureProcessable())
        return *this;
    _qimage = horizontalGaussianBlurInternal(_qimage, radius);
    return *this;
}

FImage &FImage::verticalGaussianBlur(int radius)
{
    if (radius <= 0)
    {
        if (radius < 0)
            qWarning() << "verticalGaussianBlur: radius must be > 0";
        return *this;
    }
    if (!ensureProcessable())
        return *this;
    _qimage = verticalGaussianBlurInternal(_qimage, radius);
    return *this;
}

FImage &FImage::uniformBlur(int radius)
{
    if (radius <= 0)
    {
        if (radius < 0)
            qWarning() << "uniformBlur: radius must be > 0";
        return *this;
    }
    if (!ensureProcessable())
        return *this;
    // uniformBlur 的 radius 参数即为 kernel size，不 ×2+1
    _qimage = boxBlurInternal(_qimage, radius);
    return *this;
}

FImage &FImage::horizontalUniforBlur(int radius)
{
    if (radius <= 0)
    {
        if (radius < 0)
            qWarning() << "horizontalUniforBlur: radius must be > 0";
        return *this;
    }
    if (!ensureProcessable())
        return *this;
    _qimage = horizontalBoxBlurInternal(_qimage, radius);
    return *this;
}

FImage &FImage::verticalUniforBlur(int radius)
{
    if (radius <= 0)
    {
        if (radius < 0)
            qWarning() << "verticalUniforBlur: radius must be > 0";
        return *this;
    }
    if (!ensureProcessable())
        return *this;
    _qimage = verticalBoxBlurInternal(_qimage, radius);
    return *this;
}

FImage &FImage::medianBlur(int radius)
{
    if (radius < 0)
    {
        qWarning() << "medianBlur: radius must be >= 0";
        return *this;
    }
    if (radius == 0)
        return *this;
    if (!ensureProcessable())
        return *this;

    const int ksize = radius * 2 + 1;
    _qimage = medianBlurInternal(_qimage, ksize);
    return *this;
}

#endif // was: Q_OS_WIN — 手动实现，全平台通用


// ══════════════════════════════════════════════════════════════
//  跨平台共享实现（构造/赋值/椒盐噪声/灰度）
// ══════════════════════════════════════════════════════════════

FImage::FImage(const QString &fileName, const char *format) :
    _qimage(fileName, format) {}

FImage::FImage(const QImage &image) :
    _qimage(image) {}

FImage::FImage(QImage &&other) noexcept :
    _qimage(std::move(other)) {}

FImage::FImage(const FImage &fimage) :
    _qimage(fimage._qimage) {}

FImage::FImage(FImage &&fimage) noexcept :
    _qimage(std::move(fimage._qimage)) {}

FImage &FImage::operator=(const FImage &fimage)
{
    _qimage = fimage._qimage;
    return *this;
}

FImage &FImage::operator=(FImage &&fimage) noexcept
{
    _qimage = std::move(fimage._qimage);
    return *this;
}

FImage &FImage::operator=(const QImage &qimage)
{
    _qimage = qimage;
    return *this;
}

FImage &FImage::operator=(QImage &&qimage) noexcept
{
    _qimage = std::move(qimage);
    return *this;
}

FImage::operator QImage() const { return _qimage; }

QPixmap FImage::toQPixmap() const { return QPixmap::fromImage(_qimage); }

bool FImage::ensureProcessable()
{
    switch (_qimage.format())
    {
        case QImage::Format_Mono :
            _qimage = _qimage.convertToFormat(QImage::Format_ARGB32);
            return true;
        case QImage::Format_Invalid :
            qWarning() << "FImage: invalid image, cannot process";
            return false;
        default :
            return true;
    }
}

FImage &FImage::impulseNoise(double noiseRatio)
{
    constexpr QRgb white = qRgba(255, 255, 255, 255);
    constexpr QRgb black = qRgba(0, 0, 0, 255);

    if (noiseRatio > 0.0 && noiseRatio < 1.0)
    {
        for (int x = 0; x < _qimage.width(); x++)
            for (int y = 0; y < _qimage.height(); y++)
                if (QRandomGenerator::global()->bounded(1.0) <= noiseRatio)
                    _qimage.setPixel(x, y, QRandomGenerator::global()->bounded(0, 2) ? black : white);
    }
    else if (noiseRatio >= 1.0)
    {
        for (int x = 0; x < _qimage.width(); x++)
            for (int y = 0; y < _qimage.height(); y++)
                _qimage.setPixel(x, y, QRandomGenerator::global()->bounded(0, 2) ? black : white);
    }
    else if (noiseRatio < 0.0)
        qWarning() << "Noise ratio must be greater than or equal to 0";

    return *this;
}

FImage &FImage::greyScale()
{
    _qimage = _qimage.convertToFormat(QImage::Format_Grayscale8);
    return *this;
}
