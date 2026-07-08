# Qt for WebAssembly 中集成 OpenCV.js 进行图像处理：完整指南

> **技术栈**：Qt 6.7+ / Emscripten 3.x / OpenCV.js 4.x / C++17  
> **运行环境**：浏览器（WebAssembly single-threaded）  
> **前置知识**：Qt C++ 基础、CMake、Emscripten 交叉编译

---

## 目录

1. [概述：为什么要在 WASM 中用 OpenCV.js](#1-概述为什么要在-wasm-中用-opencvjs)
2. [环境准备与 OpenCV.js 加载](#2-环境准备与-opencvjs-加载)
3. [核心原理：EM_JS 桥接机制](#3-核心原理em_js-桥接机制)
4. [类型系统：QImage 与 OpenCV Mat 的映射](#4-类型系统qimage-与-opencv-mat-的映射)
5. [图像处理函数库：9 种模糊算法实现](#5-图像处理函数库9-种模糊算法实现)
6. [封装设计：构建可复用的 FImage 类](#6-封装设计构建可复用的-fimage-类)
7. [使用指南与最佳实践](#7-使用指南与最佳实践)
8. [内存模型与性能调优](#8-内存模型与性能调优)
9. [常见问题与注意事项](#9-常见问题与注意事项)

---

## 1. 概述：为什么要在 WASM 中用 OpenCV.js

### 1.1 背景

当你把一个 Qt 桌面应用编译为 WebAssembly 在浏览器中运行时，所有依赖本地动态库的功能都会失效——包括 OpenCV。传统的 `cv::GaussianBlur()` 调用链路（C++ → OpenCV DLL → SIMD 加速）在 WASM 环境中完全不可用。

**OpenCV.js** 是 OpenCV 官方提供的 WebAssembly 版本，它将 `libopencv` 编译为 `.wasm` + `.js` 胶水代码，并在浏览器中以 `window.cv` 全局对象的形式暴露完整的 OpenCV API。

### 1.2 整体架构

```
┌─────────────────────────────────────────────┐
│              浏览器                          │
│                                             │
│  ┌─────────────────────────────────────┐    │
│  │          Qt/WASM 应用                 │    │
│  │                                     │    │
│  │  FImage (C++ 封装层)                 │    │
│  │    │                                │    │
│  │    │ EM_JS 调用                      │    │
│  │    ▼                                │    │
│  │  内联 JavaScript                     │    │
│  │    │                                │    │
│  │    │ cv.matFromArray() / cv.blur()   │    │
│  │    ▼                                │    │
│  │  OpenCV.js (window.cv)              │    │
│  │    │                                │    │
│  │    │ Rust/JS 胶水代码                │    │
│  │    ▼                                │    │
│  │  opencv_js.wasm                     │    │
│  └─────────────────────────────────────┘    │
│                                             │
│  ┌─────────────────────────────────────┐    │
│  │  Qt WASM 线性内存                    │    │
│  │  ┌─────────────────────────────┐    │    │
│  │  │ QImage 像素缓冲区             │    │    │
│  │  │  ↕ HEAPU8.set() / subarray()│    │    │
│  │  │ OpenCV WASM Heap            │    │    │
│  │  └─────────────────────────────┘    │    │
│  └─────────────────────────────────────┘    │
└─────────────────────────────────────────────┘
```

### 1.3 本文将教你什么

- 使用 **EM_JS** 在 C++ 中内联 JavaScript，调用 OpenCV.js API
- 在 QImage 和 `cv.Mat` 之间正确传递像素数据
- 实现高斯模糊、方框模糊、中值滤波等 9 种图像处理算法
- 构建一个类型安全、支持链式调用的 C++ 图像处理工具类
- 理解 WASM 内存模型，避免常见性能陷阱

---

## 2. 环境准备与 OpenCV.js 加载

### 2.1 获取 OpenCV.js

从 OpenCV 官方 [Releases](https://github.com/opencv/opencv/releases) 下载预编译的 `opencv-{version}-docs-js.zip`，解压后得到 `opencv.js`。

### 2.2 在 HTML 中加载

在你的 Qt WASM 入口 HTML 中，**在 Qt 加载脚本之前**引入 OpenCV.js：

```html
<!doctype html>
<html lang="zh-CN">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>My App</title>
</head>
<body>
    <!-- ① OpenCV.js 必须最先加载 -->
    <script src="js/opencv.js" type="text/javascript"></script>

    <!-- ② Qt WASM 加载器 -->
    <script src="qtloader.js" type="module"></script>
</body>
</html>
```

> **⚠️ 重要**：OpenCV.js 的 `<script>` 不要加 `async` 属性。Qt 应用启动时会假设 `window.cv` 已经可用。

### 2.3 静态文件托管

将 `opencv.js` 放在你的 Web 服务器静态文件目录下（例如 `www/js/opencv.js`）。Qt WASM 应用本身不负责加载 OpenCV.js——它假设页面中已经存在 `window.cv`。

### 2.4 CMake 配置

CMake 不需要链接任何 OpenCV 库。只需配置标准的 Qt for WebAssembly 构建：

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

# Emscripten 特定设置
if(EMSCRIPTEN)
    set(QT_WASM_INITIAL_MEMORY "80MB")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

qt_add_executable(MyApp
    main.cpp
    fimage.cpp
)

target_link_libraries(MyApp PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)
```

---

## 3. 核心原理：EM_JS 桥接机制

### 3.1 什么是 EM_JS

`EM_JS` 是 Emscripten 提供的宏，允许在 C++ 源代码中直接嵌入 JavaScript。编译器将 JS 代码原封不动地输出到生成的胶水文件中，同时在 C++ 侧生成对应的函数声明。

```cpp
EM_JS(return_type, function_name, (param_type param, ...), {
    // 这里是 JavaScript 代码，可以直接访问 window、document、cv 等
});
```

### 3.2 数据传递流程

理解 QImage 像素数据如何流入 OpenCV.js 再流回，是整个方案的核心：

```
步骤 ① C++ → JS 传址
─────────────────────────
QImage::bits()  →  reinterpret_cast<uintptr_t>  →  EM_JS 的 bitsPtr 参数
                                                      │
步骤 ② 创建零拷贝视图                                    │
─────────────────────────                               ▼
                                          HEAPU8.subarray(bitsPtr, bitsPtr + size)
                                          返回一个直接映射到 QImage 内存的 Uint8Array
                                                      │
步骤 ③ 复制到 OpenCV Wasm Heap                           │
─────────────────────────                               ▼
                                          cv.matFromArray(h, w, type, srcView)
                                          ⚠ 内部会复制数据到 OpenCV 管理的 HEAP
                                                      │
步骤 ④ OpenCV 计算                                      │
─────────────────────────                               ▼
                                          cv.GaussianBlur(mat, mat, ...)
                                          （原地修改，mat 同时作为 src 和 dst）
                                                      │
步骤 ⑤ 写回 QImage                                     │
─────────────────────────                               ▼
                                          HEAPU8.set(mat.data, bitsPtr)
                                          ⚠ 将处理结果逐字节复制回 QImage 内存
```

**每次处理发生 2 次 N 字节的数据复制**，其中 N = width × height × 4（RGBA 图像）。这是预编译版 OpenCV.js 的固有开销，详见[第 8 章](#8-内存模型与性能调优)。

### 3.3 通用代码模板

所有图像处理函数遵循相同模式。以下是一个可直接复用为起点的模板：

```cpp
#include <emscripten.h>

EM_JS(void, myFilter_EMJS, (uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine), {
    // ① 计算缓冲区大小
    var size = bytesPerLine * h;

    // ② 在 QImage 内存上创建零拷贝 Uint8Array
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);

    // ③ 创建 OpenCV Mat（数据被复制到 OpenCV 的 WASM Heap）
    var mat = cv.matFromArray(h, w, type, srcView);

    // ④ 执行 OpenCV 操作（原地处理）
    cv.yourFunction(mat, mat, /* 参数 */);

    // ⑤ 将处理结果写回 QImage 内存
    HEAPU8.set(mat.data, bitsPtr);

    // ⑥ 释放 Mat（必须手动管理内存！）
    mat.delete();
});
```

### 3.4 参数说明

| 参数 | C++ 类型 | JS 类型 | 含义 |
|------|---------|---------|------|
| `bitsPtr` | `uintptr_t` | `number` | QImage 像素数据在 WASM 线性内存中的首地址 |
| `type` | `int` | `number` | OpenCV Mat 类型常量（如 `CV_8UC4 = 24`） |
| `w` | `int` | `number` | 图像宽度（像素） |
| `h` | `int` | `number` | 图像高度（像素） |
| `bytesPerLine` | `int` | `number` | QImage 每行字节数（用于计算缓冲区总大小） |
| `kernel` | `int` | `number` | 卷积核尺寸（各算法含义不同，见下文） |

---

## 4. 类型系统：QImage 与 OpenCV Mat 的映射

### 4.1 OpenCV Mat 类型枚举

在 OpenCV 中，Mat 的类型用一个 `int` 编码，规则为：

```
CV_{depth}{channels}  →  type = (channels - 1) × 8 + depth_code
```

为了在 C++ 侧有类型安全的映射，可以定义以下枚举：

```cpp
namespace WasmCV {
    namespace cv {
        enum class Type : int {
            // ── 8-bit unsigned ──
            CV_8UC1 = 0,   // 灰度图
            CV_8UC2 = 8,
            CV_8UC3 = 16,  // RGB
            CV_8UC4 = 24,  // RGBA — 最常用

            // ── 8-bit signed ──
            CV_8SC1 = 1,
            CV_8SC2 = 9,
            CV_8SC3 = 17,
            CV_8SC4 = 25,

            // ── 16-bit unsigned ──
            CV_16UC1 = 2,
            CV_16UC2 = 10,
            CV_16UC3 = 18,
            CV_16UC4 = 26,

            // ── 16-bit signed ──
            CV_16SC1 = 3,
            CV_16SC2 = 11,
            CV_16SC3 = 19,
            CV_16SC4 = 27,

            // ── 32-bit signed ──
            CV_32SC1 = 4,
            CV_32SC2 = 12,
            CV_32SC3 = 20,
            CV_32SC4 = 28,

            // ── 32-bit float ──
            CV_32FC1 = 5,
            CV_32FC2 = 13,
            CV_32FC3 = 21,
            CV_32FC4 = 29,

            // ── 64-bit float ──
            CV_64FC1 = 6,
            CV_64FC2 = 14,
            CV_64FC3 = 22,
            CV_64FC4 = 30,
        };

        // 提取深度（低 3 位）
        constexpr int depthOf(Type t) { return static_cast<int>(t) & 7; }

        // 提取通道数
        constexpr int channelsOf(Type t) { return (static_cast<int>(t) >> 3) + 1; }
    }
}
```

### 4.2 提取方法

| 操作 | 公式 | 示例（CV_8UC4 = 24） |
|------|------|----------------------|
| 深度 | `type & 7` | `24 & 7 = 0`（CV_8U） |
| 通道数 | `(type >> 3) + 1` | `(24 >> 3) + 1 = 4` |

### 4.3 QImage → OpenCV 类型映射函数

```cpp
static WasmCV::cv::Type toCvType(QImage::Format fmt) {
    switch (fmt) {
        case QImage::Format_Grayscale8:  return WasmCV::cv::Type::CV_8UC1;
        case QImage::Format_Grayscale16: return WasmCV::cv::Type::CV_16UC1;
        case QImage::Format_RGB888:      return WasmCV::cv::Type::CV_8UC3;
        case QImage::Format_RGBA64:      return WasmCV::cv::Type::CV_16UC4;
        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32_Premultiplied:
            return WasmCV::cv::Type::CV_8UC4;
        default:
            // 不支持的格式统一回退到 8-bit 4 通道
            qWarning() << "Unsupported QImage format, fallback to CV_8UC4";
            return WasmCV::cv::Type::CV_8UC4;
    }
}
```

> **提示**：OpenCV 完全支持 RGBA 四通道图像的高斯模糊等滤波操作，**不需要**做 RGBA→RGB 的颜色空间转换——省去两次昂贵的逐像素转换。

---

## 5. 图像处理函数库：9 种模糊算法实现

### 5.1 高斯模糊 — gaussianBlur

**数学原理**：使用二维高斯核进行加权平均卷积，权重服从正态分布。视觉效果最柔和自然。

```cpp
EM_JS(void, gaussianBlur_EMJS,
    (uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel), {
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.GaussianBlur(mat, mat, new cv.Size(kernel, kernel), 0);
    HEAPU8.set(mat.data, bitsPtr);
    mat.delete();
});
```

**参数**：`kernel = radius × 2 + 1`（必须为奇数，OpenCV 要求）  
**复杂度**：O(W × H × K²)  
**适用场景**：背景虚化、图像柔化、作为其他处理的预处理步骤

---

### 5.2 水平 / 垂直方向高斯模糊

利用高斯核的**可分离性**，二维卷积可拆为两个一维卷积，复杂度从 O(K²) 降至 O(2K)：

```
GaussianBlur(img, Size(K,K))  ≡  GaussianBlur(img, Size(K,1))  ∘  GaussianBlur(img, Size(1,K))
```

**水平高斯模糊**：

```cpp
EM_JS(void, horizontalGaussianBlur_EMJS,
    (uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel), {
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.GaussianBlur(mat, mat, new cv.Size(kernel, 1), 0);  // ← 注意 Size(K, 1)
    HEAPU8.set(mat.data, bitsPtr);
    mat.delete();
});
```

**垂直高斯模糊**：

```cpp
EM_JS(void, verticalGaussianBlur_EMJS,
    (uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel), {
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.GaussianBlur(mat, mat, new cv.Size(1, kernel), 0);  // ← 注意 Size(1, K)
    HEAPU8.set(mat.data, bitsPtr);
    mat.delete();
});
```

**适用场景**：方向性运动模糊特效（水平 = 横向动感，垂直 = 纵向动感）

---

### 5.3 均匀模糊 — uniformBlur（cv.blur）

核内所有像素权重相等，与高斯模糊的区别在于没有衰减：

```
输出 = ∑(邻域像素) / (K × K)
```

```cpp
EM_JS(void, uniformBlur_EMJS,
    (uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel), {
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.blur(mat, mat, new cv.Size(kernel, kernel));
    HEAPU8.set(mat.data, bitsPtr);
    mat.delete();
});
```

**参数**：`kernel` 直接传入（不经过 `×2+1` 转换）  
**特点**：简单快速，但边缘有较明显的"阶梯"感

---

### 5.4 方向性均匀模糊

与均匀模糊同理，只在一个方向操作：

```cpp
// 水平
EM_JS(void, horizontalUniforBlur_EMJS, (...), {
    cv.blur(mat, mat, new cv.Size(kernel, 1));
});

// 垂直
EM_JS(void, verticalUniforBlur_EMJS, (...), {
    cv.blur(mat, mat, new cv.Size(1, kernel));
});
```

---

### 5.5 方框模糊 — boxBlur（cv.boxFilter）

`cv.boxFilter` 与 `cv.blur` 类似，但支持额外的 `ddepth` 参数控制输出深度：

```cpp
EM_JS(void, boxBlur_EMJS,
    (uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel), {
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.boxFilter(mat, mat, -1, new cv.Size(kernel, kernel));  // ddepth=-1: 与输入相同
    HEAPU8.set(mat.data, bitsPtr);
    mat.delete();
});
```

**参数**：`kernel = radius × 2 + 1`，`ddepth = -1` 表示输出深度与输入一致

---

### 5.6 近似高斯模糊 — approxGaussianBlur

**核心思想**：根据中心极限定理，多次方框滤波叠加可逼近高斯滤波。3 次 `boxFilter` 的效果与高斯模糊视觉上几乎无法区分。

```cpp
EM_JS(void, approxGaussianBlur_EMJS,
    (uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel), {
    var ksize = new cv.Size(kernel, kernel);
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);

    cv.boxFilter(mat, mat, -1, ksize);
    cv.boxFilter(mat, mat, -1, ksize);
    cv.boxFilter(mat, mat, -1, ksize);

    HEAPU8.set(mat.data, bitsPtr);
    mat.delete();
});
```

**性能优势**：boxFilter 可使用滑动窗口优化至 O(W×H)，在大核场景下（K > 15）显著快于标准高斯模糊。

---

### 5.7 中值滤波 — medianBlur

将每个像素替换为其邻域内所有像素的**中位数**。对椒盐噪声有极好的去除效果，且能较好地保留边缘。

```cpp
EM_JS(void, medianBlur_EMJS,
    (uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel), {
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.medianBlur(mat, mat, kernel);
    HEAPU8.set(mat.data, bitsPtr);
    mat.delete();
});
```

**约束条件**（OpenCV 限制）：
- 不支持 2 通道图像
- `ksize > 5` 时要求输入为 `CV_8U` 深度

在 C++ 封装层应加入校验：

```cpp
// C++ 侧预处理校验
const int ksize = radius * 2 + 1;
const int channels = (type >> 3) + 1;
const int depth = type & 7;

if (channels == 2) {
    qWarning() << "medianBlur does not support 2-channel images";
    return *this;
}
if (ksize > 5 && depth != 0) {  // 0 = CV_8U
    qWarning() << "medianBlur ksize>5 requires CV_8U";
    return *this;
}
```

---

### 5.8 算法速查表

| 方法 | Kernel 含义 | OpenCV 函数 | 复杂度 | 视觉效果 |
|------|:---:|------|:---:|------|
| `gaussianBlur` | `radius × 2 + 1` | `cv.GaussianBlur` | O(WH·K²) | 柔和自然 |
| `approxGaussianBlur` | `radius × 2 + 1` | `cv.boxFilter × 3` | O(WH) | ≈ 高斯 |
| `boxBlur` | `radius × 2 + 1` | `cv.boxFilter` | O(WH·K²) | 均匀感 |
| `uniformBlur` | 直接传入 | `cv.blur` | O(WH·K²) | 阶梯感 |
| `medianBlur` | `radius × 2 + 1` | `cv.medianBlur` | O(WH·K²·logK) | 保边缘 |
| 水平方向 | `radius × 2 + 1` | 对应的 blur × `Size(K,1)` | O(WH·K) | 横向动感 |
| 垂直方向 | `radius × 2 + 1` | 对应的 blur × `Size(1,K)` | O(WH·K) | 纵向动感 |

---

## 6. 封装设计：构建可复用的 FImage 类

### 6.1 完整头文件

```cpp
#ifndef FIMAGE_H
#define FIMAGE_H
#include <QImage>
#include <QtMath>

// 前置声明
class QString;
class QPixmap;

// ── WASM 环境下的 OpenCV 类型与 EM_JS 声明 ──
#ifdef __EMSCRIPTEN__
#include <emscripten.h>

namespace WasmCV {
    namespace cv {
        enum class Type : int {
            CV_8UC1 = 0, CV_8UC2 = 8, CV_8UC3 = 16, CV_8UC4 = 24,
            CV_8SC1 = 1, CV_8SC2 = 9, CV_8SC3 = 17, CV_8SC4 = 25,
            CV_16UC1 = 2, CV_16UC2 = 10, CV_16UC3 = 18, CV_16UC4 = 26,
            CV_16SC1 = 3, CV_16SC2 = 11, CV_16SC3 = 19, CV_16SC4 = 27,
            CV_32SC1 = 4, CV_32SC2 = 12, CV_32SC3 = 20, CV_32SC4 = 28,
            CV_32FC1 = 5, CV_32FC2 = 13, CV_32FC3 = 21, CV_32FC4 = 29,
            CV_64FC1 = 6, CV_64FC2 = 14, CV_64FC3 = 22, CV_64FC4 = 30,
        };
        constexpr int depthOf(Type t) { return static_cast<int>(t) & 7; }
        constexpr int channelsOf(Type t) { return (static_cast<int>(t) >> 3) + 1; }
    }

    extern "C" {
        void gaussianBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);
        void horizontalGaussianBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);
        void verticalGaussianBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);
        void uniformBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);
        void horizontalUniforBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);
        void verticalUniforBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);
        void boxBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);
        void approxGaussianBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);
        void medianBlur_EMJS(uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int kernel);
    }
}
#endif  // __EMSCRIPTEN__

class FImage {
public:
    // ── 生命周期 ──
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

    // 隐式转换为 QImage
    operator QImage() const;

    // ── 图像处理接口（所有方法返回 *this 支持链式调用） ──
    FImage &gaussianBlur(int radius = 30);
    FImage &boxBlur(int radius = 30);
    FImage &approxGaussianBlur(int radius = 30);
    FImage &impulseNoise(double noiseRatio = 0.3);
    FImage &greyScale();

#ifdef __EMSCRIPTEN__
    // OpenCV.js 独占接口
    FImage &horizontalGaussianBlur(int radius = 30);
    FImage &verticalGaussianBlur(int radius = 30);
    FImage &uniformBlur(int radius = 30);
    FImage &horizontalUniforBlur(int radius = 30);
    FImage &verticalUniforBlur(int radius = 30);
    FImage &medianBlur(int radius = 30);
#endif

    // ── 工具方法 ──
    [[nodiscard]] QPixmap toQPixmap() const;
    QImage &qImage() { return _qimage; }

private:
    QImage _qimage;
#ifdef __EMSCRIPTEN__
    static WasmCV::cv::Type toCvType(QImage::Format fmt);
#endif
    bool ensureProcessable();
};

#endif // FIMAGE_H
```

### 6.2 关键实现：参数校验与调用分发

以 `gaussianBlur` 为例，展示一个典型的 C++ 封装方法：

```cpp
#ifdef __EMSCRIPTEN__

FImage &FImage::gaussianBlur(int radius) {
    // ① 边界条件检查
    if (radius < 0) {
        qWarning() << "gaussianBlur: radius must be >= 0";
        return *this;
    }
    if (radius == 0)
        return *this;  // 零半径 = 无操作

    // ② 格式兼容性保障
    if (!ensureProcessable())
        return *this;

    // ③ 调用 EM_JS 函数
    gaussianBlur_EMJS(
        reinterpret_cast<uintptr_t>(_qimage.bits()),  // 像素数据指针
        static_cast<int>(toCvType(_qimage.format())), // OpenCV 类型
        _qimage.width(),
        _qimage.height(),
        _qimage.bytesPerLine(),
        radius * 2 + 1                                // 核尺寸
    );

    return *this;  // 链式调用
}

#endif
```

### 6.3 格式兼容性保障

```cpp
bool FImage::ensureProcessable() {
    switch (_qimage.format()) {
        case QImage::Format_Mono:
            // 单色位图自动转换为 32-bit ARGB
            _qimage = _qimage.convertToFormat(QImage::Format_ARGB32);
            return true;
        case QImage::Format_Invalid:
            qWarning() << "FImage: invalid image, cannot process";
            return false;
        default:
            return true;
    }
}
```

### 6.4 生成与转换

```cpp
// ── 构造 ──
FImage::FImage(const QString &fileName, const char *format)
    : _qimage(fileName, format) {}

FImage::FImage(const QImage &image) : _qimage(image) {}
FImage::FImage(QImage &&img) noexcept : _qimage(std::move(img)) {}
FImage::FImage(const FImage &other) : _qimage(other._qimage) {}
FImage::FImage(FImage &&other) noexcept : _qimage(std::move(other._qimage)) {}

// ── 赋值 ──
FImage &FImage::operator=(const FImage &other) { _qimage = other._qimage; return *this; }
FImage &FImage::operator=(FImage &&other) noexcept { _qimage = std::move(other._qimage); return *this; }
FImage &FImage::operator=(const QImage &img) { _qimage = img; return *this; }
FImage &FImage::operator=(QImage &&img) noexcept { _qimage = std::move(img); return *this; }

// ── 隐式转换 ──
FImage::operator QImage() const { return _qimage; }

// ── 导出 ──
QPixmap FImage::toQPixmap() const { return QPixmap::fromImage(_qimage); }

// ── 灰度转换 ──
FImage &FImage::greyScale() {
    _qimage = _qimage.convertToFormat(QImage::Format_Grayscale8);
    return *this;
}
```

### 6.5 椒盐噪声生成

```cpp
FImage &FImage::impulseNoise(double noiseRatio) {
    constexpr QRgb white = qRgba(255, 255, 255, 255);
    constexpr QRgb black = qRgba(0, 0, 0, 255);

    if (noiseRatio > 0.0 && noiseRatio < 1.0) {
        for (int x = 0; x < _qimage.width(); x++)
            for (int y = 0; y < _qimage.height(); y++)
                if (QRandomGenerator::global()->bounded(1.0) <= noiseRatio)
                    _qimage.setPixel(x, y, QRandomGenerator::global()->bounded(0, 2) ? black : white);
    } else if (noiseRatio >= 1.0) {
        for (int x = 0; x < _qimage.width(); x++)
            for (int y = 0; y < _qimage.height(); y++)
                _qimage.setPixel(x, y, QRandomGenerator::global()->bounded(0, 2) ? black : white);
    }
    return *this;
}
```

> **提示**：椒盐噪声 + 中值滤波是一对黄金组合。先 `impulseNoise(0.3)` 再 `medianBlur(1)`，可以看到中值滤波出色的去噪效果。

### 6.6 其他方法的封装

其他 WASM-exclusive 方法遵循与 `gaussianBlur` 完全相同的封装模式。以 `medianBlur` 为例，它需要在 C++ 侧做额外的 OpenCV 约束校验：

```cpp
FImage &FImage::medianBlur(int radius) {
    if (radius < 0) { qWarning() << "..."; return *this; }
    if (radius == 0) return *this;
    if (!ensureProcessable()) return *this;

    const int ksize = radius * 2 + 1;
    const int type = static_cast<int>(toCvType(_qimage.format()));
    const int depth = type & 7;
    const int channels = (type >> 3) + 1;

    // OpenCV 约束检查
    if (channels == 2) {
        qWarning() << "medianBlur does not support 2-channel images";
        return *this;
    }
    if (ksize > 5 && depth != 0) {  // 0 = CV_8U
        qWarning() << "medianBlur ksize>5 requires CV_8U";
        return *this;
    }

    medianBlur_EMJS(
        reinterpret_cast<uintptr_t>(_qimage.bits()),
        type, _qimage.width(), _qimage.height(),
        _qimage.bytesPerLine(), ksize
    );
    return *this;
}
```

**其他方法**（`uniformBlur`、`horizontalUniforBlur` 等）的封装更简单——不同之处仅在于传入的 `kernel` 值是否需要 `×2+1` 变换，以及调用对应的 EM_JS 函数名。核心校验逻辑完全一致。

---

## 7. 使用指南与最佳实践

### 7.1 基础用法

```cpp
#include "fimage.h"

// 从文件加载并进行高斯模糊
FImage img("photo.jpg");
img.gaussianBlur(30);
QPixmap result = img.toQPixmap();

// 链式调用：加载 → 模糊 → 灰度 → 导出
QPixmap out = FImage("banner.jpg")
    .gaussianBlur(20)
    .greyScale()
    .toQPixmap();
```

### 7.2 在 Qt Widget 中集成

```cpp
void MyWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    FImage bg("background.webp");
    bg.gaussianBlur(50);

    painter.drawPixmap(0, 0, bg.toQPixmap());
}
```

### 7.3 高性能模糊（大核场景）

当核半径较大（> 15）时，`approxGaussianBlur` 比 `gaussianBlur` 快得多：

```cpp
// 大核模糊 — approxGaussianBlur 更高效
FImage largeBg("4k_wallpaper.jpg");
largeBg.approxGaussianBlur(60);  // 60px 半径的近似高斯模糊
```

### 7.4 去噪处理

```cpp
// 对扫描文档进行中值滤波去噪
FImage scan("document_scan.jpg");
scan.medianBlur(2);  // 5×5 中值滤波
```

### 7.5 运动模糊特效

```cpp
#ifdef __EMSCRIPTEN__
FImage city("cityscape.jpg");

// 水平方向模糊 = 横向运动感
city.horizontalGaussianBlur(10);

// 或者用均匀模糊得到更锐利的运动线条
FImage sports("race.jpg");
sports.horizontalUniforBlur(15);
#endif
```

### 7.6 多步骤流水线

```cpp
FImage photo("portrait.jpg");

// 加噪 → 中值滤波去噪 → 轻微高斯柔化
photo.impulseNoise(0.1)
     .medianBlur(1)
     .gaussianBlur(3);

// 导出到 QPixmap 显示
QPixmap final = photo.toQPixmap();
```

### 7.7 与 Qt 信号槽结合

FImage 是纯同步的栈对象，不与 Qt 事件循环交互。在 WASM single-threaded 环境中，图像处理会阻塞当前帧。对于大图（> 4K），建议：

```cpp
// 如果处理耗时较长，可以先显示占位图再异步处理
void MyWidget::loadAndBlur(const QString &path) {
    QPixmap placeholder = QPixmap::fromImage(QImage(path).scaled(100, 100));
    setPixmap(placeholder);  // 快速显示低分辨率预览

    FImage img(path);
    img.gaussianBlur(30);
    setPixmap(img.toQPixmap());  // 显示模糊后的高分辨率图
}
```

---

## 8. 内存模型与性能调优

### 8.1 一次操作的内存流

以 1920×1080 RGBA 图像为例，完整的内存轨迹：

```
阶段             方向           数据量
─────────────────────────────────────────
QImage 内存      静止          1920×1080×4 = 8,294,400 字节

HEAPU8.subarray 创建视图      0 字节（零拷贝，仅创建引用）

matFromArray    QImage→Mat    8,294,400 字节（复制到 OpenCV WASM Heap）

OpenCV 计算     Mat 内部      8,294,400 字节读写（原地操作）

HEAPU8.set      Mat→QImage    8,294,400 字节（复制回 QImage 内存）

mat.delete()    释放 Mat      8,294,400 字节释放
─────────────────────────────────────────
总计                         2 次 × 8MB = ~16MB 数据搬运
```

### 8.2 为什么无法零拷贝

预编译版 OpenCV.js 存在三个根本限制：

| 限制 | 影响 | 根因 |
|------|------|------|
| `Mat(rows, cols, type, dataPtr)` 构造函数未导出 | 无法让 Mat 直接引用 QImage 内存 | Embind 未绑定该重载 |
| `mat.data` 是只读 Getter | 不能通过赋值改变 Mat 的数据源 | 反映底层的 const 指针 |
| `mat.step` 不可写 | 无法自定义行步长来匹配 QImage 的 bytesPerLine | Embind 属性映射限制 |

**结论**：在当前预编译版 OpenCV.js 中，2 次数据复制是不可避免的固定开销。如果需要零拷贝，唯一的路径是自己从源码编译 OpenCV.js 并修改 Embind 绑定导出。

### 8.3 性能基准

| 操作 | 1920×1080 RGBA | 3840×2160 RGBA |
|------|:---:|:---:|
| gaussianBlur(30) | ~45ms | ~190ms |
| approxGaussianBlur(30) | ~25ms | ~105ms |
| boxBlur(30) | ~18ms | ~75ms |
| medianBlur(3) | ~35ms | ~150ms |
| uniformBlur(30) | ~16ms | ~68ms |

> *以上数据在 Edge 浏览器 WASM single-threaded Release 模式下测得，仅供参考。*

### 8.4 内存管理注意事项

**必须手动释放的对象**：
- `cv.Mat` — 底层在 OpenCV 的 WASM C++ Heap 中分配
- `cv.MatVector` — 同理

```javascript
var mat = cv.matFromArray(h, w, type, data);
// ... 使用 mat ...
mat.delete();  // ← 必须！不受 JavaScript GC 管理
```

**无需释放的对象**（纯 JS 对象，受 GC 管理）：
- `cv.Size` / `cv.Point` / `cv.Rect` / `cv.Scalar`

```javascript
var ksize = new cv.Size(31, 31);  // 只是 {width: 31, height: 31}
// ksize 不需要 delete() — JS GC 会处理
```

> ⚠️ **教训**：对 `cv.Size` 调用 `.delete()` 会导致 TypeError，因为它只是普通 JS 对象，没有 `delete` 方法。

### 8.5 EM_JS vs emscripten::val 性能对比

| 方案 | 单次调用开销 | 代码复杂度 | 推荐场景 |
|------|:---:|:---:|------|
| `EM_JS` | ~1μs | 低（JS 在 C++ 文件中直接可见） | 固定流程的少量 API 调用 |
| `emscripten::val` | ~1.2μs | 高（需要多次 `.call()` 链式调用） | 动态组合大量 API |

实测 20000 次操作，`EM_JS` 版仅比 `val` 版快约 2.6%。选择 `EM_JS` 的主要原因是代码可维护性——内联 JS 比 `val.call()` 层层转发更直观易读。

---

## 9. 常见问题与注意事项

### Q1：OpenCV.js 什么时候加载完成？

OpenCV.js 是异步加载的。本章方案假设在 HTML 中通过同步 `<script>` 标签**在 Qt 之前**加载 OpenCV.js。如果你使用异步加载（`<script async>` 或动态 `import()`），需要在 Qt 应用启动前手动轮询 `window.cv` 是否就绪。

### Q2：为什么不在 EM_JS 中使用 Canvas API？

CSS filter（`canvas.style.filter = 'blur(30px)'`）或 Canvas 2D `filter` 属性修改的是 GPU 合成层的视觉效果，不改变实际像素数据。Qt 的 QPainter 通过 Canvas 2D API 软件光栅化绘制，不会触发 GPU 合成路径，因此 CSS filter 对 `QImage::bits()` 无任何影响。

### Q3：图像处理会阻塞 UI 吗？

是的。WASM single-threaded 模式下，所有 EM_JS 调用在主线程同步执行。处理 4K 大图可能需要 100-200ms，期间浏览器 UI 完全冻结。对于交互式应用，建议在用户操作间隔（如轮播切换）中执行，或先用缩略图响应再逐步替换。

### Q4：为什么不使用 Web Worker？

Qt for WebAssembly 目前**不正式支持**多线程（pthread + SharedArrayBuffer 模式仍处于实验阶段且兼容性问题多）。如果未来 WASM 多线程成熟，可将 FImage 处理迁移到 Worker 中执行。

### Q5：如何扩展更多 OpenCV 功能？

只需遵循第 3.3 节的模板，增加新的 EM_JS 函数即可。例如添加 Canny 边缘检测：

```cpp
EM_JS(void, canny_EMJS,
    (uintptr_t bitsPtr, int type, int w, int h, int bytesPerLine, int lowThresh, int highThresh), {
    var size = bytesPerLine * h;
    var srcView = HEAPU8.subarray(bitsPtr, bitsPtr + size);
    var mat = cv.matFromArray(h, w, type, srcView);
    cv.Canny(mat, mat, lowThresh, highThresh);
    HEAPU8.set(mat.data, bitsPtr);
    mat.delete();
});

// C++ 封装
FImage &FImage::cannyEdge(int lowThresh, int highThresh) {
    canny_EMJS(
        reinterpret_cast<uintptr_t>(_qimage.bits()),
        static_cast<int>(toCvType(_qimage.format())),
        _qimage.width(), _qimage.height(),
        _qimage.bytesPerLine(),
        lowThresh, highThresh
    );
    return *this;
}
```

### Q6：处理大图时 WASM 内存不足怎么办？

增加 CMake 中的初始内存：
```cmake
set(QT_WASM_INITIAL_MEMORY "256MB")
```

或启用内存增长（性能有惩罚）：
```cmake
target_link_options(MyApp PRIVATE -sALLOW_MEMORY_GROWTH=1)
```

---

## 附录

### A. 完整文件清单

一个最小化的 Qt WASM + OpenCV.js 图像处理模块包含以下文件：

| 文件 | 作用 |
|------|------|
| `fimage.h` | FImage 类声明、类型枚举、EM_JS 函数声明 |
| `fimage.cpp` | 全部实现代码（EM_JS 内联 JS + C++ 封装） |
| `opencv.js` | 预编译的 OpenCV.js（从 OpenCV Releases 下载） |
| `index.html` | 入口 HTML（加载 opencv.js + Qt WASM 应用） |

### B. 环境兼容性

| 组件 | 版本要求 |
|------|---------|
| Qt | 6.5+（推荐 6.7+） |
| Emscripten | 3.1.50+ |
| OpenCV.js | 4.8+（推荐 4.13+） |
| 浏览器 | Chrome 90+ / Edge 90+ / Firefox 90+ / Safari 15+ |
| C++ 标准 | C++17 |

---

> **文档版本**：v2.0  
> **最后更新**：2026-06-22
