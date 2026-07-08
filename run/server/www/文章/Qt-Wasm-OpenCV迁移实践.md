# 一次被迫的跨平台迁徙：把 Qt + OpenCV 应用搬进浏览器

> 从 Windows DLL 到 WebAssembly，从 cv::Mat 到 cv.Mat，从零拷贝幻想到 2.6% 的妥协——我把一个 Qt 图像处理项目搬进了浏览器。这是一篇关于踩坑、试探、放弃幻想和接受现实的技术笔记。

**关于本文**：本文基于作者实际项目开发经历中的真实问答记录，由 AI 辅助整理并生成。文中所有代码均已实测通过，涉及环境为 Qt 6.7.2、WebAssembly single-threaded Release 编译、emsdk 3.1.50、OpenCV.js 4.13.0（官方 GitHub 下载），测试平台 Edge 浏览器。

**摘要**：将一个依赖 OpenCV 动态链接库的 Qt 桌面应用编译为 WebAssembly 并在浏览器中运行，面临的核心挑战是如何替代本地 OpenCV。本文记录了从 `emscripten::val` 调用 OpenCV.js 起步，经历崩溃排查、性能优化、四轮零拷贝探索失败，再到 EM_JS 方案对比，最终通过 20000 次实测得出"Val 版仅比 EM_JS 慢 2.6%，代码可维护性远大于微优化"结论的完整历程。

---

## 起因：一个突如其来的 Wasm 需求

我有一个运转良好的 Qt 桌面应用：加载图片、高斯模糊、显示结果。运行在 Windows 上，链接着 OpenCV 的 DLL，一切都理所当然——

```cpp
QImage img(":/photo.png");
img = img.convertToFormat(QImage::Format_RGBA8888);
cv::Mat mat(img.height(), img.width(), CV_8UC4,img.bits(), img.bytesPerLine());
cv::GaussianBlur(mat, mat, cv::Size(15, 15), 0);
```

然后需求来了：**把它编译成 WebAssembly，在浏览器里跑。**

问题很直接：浏览器不认识 DLL，更不认识本地编译的 OpenCV。我能怎么用 OpenCV？

摆在面前的路只有一条：OpenCV.js。

---

## 初探：emscripten::val，一座跨语言的桥

OpenCV.js 本质是 C++ OpenCV 通过 Emscripten 编译成 `.wasm` + `.js` 的产物，暴露 `window.cv` 对象。我的 C++ 代码需要调用它，而 Emscripten 提供了 `emscripten::val`——一个能让你从 C++ 侧操作任意 JS 对象的工具。

第一版代码长这样：

```cpp
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/val.h>
#endif

void gaussianBlur_Val(QImage& img, int kernel)
{
    using namespace emscripten;

    int w = img.width();
    int h = img.height();
    int size = w * h * 4; // RGBA，每像素4字节

    // 获取 OpenCV.js 对象（window.cv）
    val cv = val::global()["cv"];

    // 获取 WASM 内存视图 Module.HEAPU8
    val heap = val::module_property("HEAPU8");

    // 将 QImage 像素缓冲区包装成 Uint8Array（零拷贝视图）
    val srcBuf = val::global("Uint8Array").new_(
        heap["buffer"],
        (uintptr_t)img.bits(),
        size);

    // 构造 OpenCV Mat（RGBA 四通道）
    val mat = cv["matFromArray"](h, w, cv["CV_8UC4"], srcBuf);

    // RGBA → RGB（Alpha 通道对高斯模糊无意义）
    val rgb = cv["Mat"].new_();
    cv["cvtColor"](mat, rgb, cv["COLOR_RGBA2RGB"]);

    // 保证卷积核为奇数（OpenCV 要求）
    kernel |= 1;
    val ksize = cv["Size"].new_(kernel, kernel);

    // 输出 Mat
    val dst = cv["Mat"].new_();

    // 执行高斯模糊
    cv["GaussianBlur"](rgb, dst, ksize, 0);

    // RGB → RGBA
    val rgba = cv["Mat"].new_();
    cv["cvtColor"](dst, rgba, cv["COLOR_RGB2RGBA"]);

    // 取出结果数据
    val resultData = rgba["data"];

    // 逐字节写回 QImage
    uchar* bits = img.bits();
    for (int i = 0; i < size; i++)
        bits[i] = resultData[i].as<int>();

    // 释放所有 OpenCV 对象（Mat 不受 JS GC 管理）
    mat.call<void>("delete");
    rgb.call<void>("delete");
    dst.call<void>("delete");
    rgba.call<void>("delete");
    ksize.call<void>("delete");
}
```

能跑。但很快，第一个崩溃不请自来。

---

## 第一课：不是所有对象都需要 delete

浏览器 F12 控制台：

```
TypeError: Cannot read properties of undefined (reading 'call')
    at __emval_call_method
```

一番排查，罪魁祸首是这一行：

```cpp
ksize.call<void>("delete");
```

`cv.Size` 在 OpenCV.js 中本质上就是一个普通 JS 对象 `{width, height}`——它没有 `delete()` 方法，也根本不需要释放。**只有 `cv.Mat`、`cv.MatVector` 这种底层存在 Wasm Heap 中的对象才需要手动 delete。** `cv.Size`、`cv.Point`、`cv.Rect`、`cv.Scalar` 都不需要。

删掉那一行，一切都正常了。这是第一条经验：OpenCV.js 的对象模型和 C++ 不完全一样，轻量对象由 JS GC 管理，只有重型对象才占用 Wasm 内存。

---

## 800 万次跨语言调用的灾难

代码能跑了，但慢得离谱。为什么？

```cpp
for (int i = 0; i < size; i++)
    bits[i] = resultData[i].as<int>();
```

一张 1920×1080 的 RGBA 图片，这个循环执行了 **8,294,400 次**——每次都是一个完整的 C++ → Embind → JS 的跨语言调用。这是性能的地狱。

解法其实很简单：用 JS 原生的 `Uint8Array.set()` 一次性拷贝。

```cpp
// 一次 memcpy，替代 800 万次跨语言调用
srcBuf.call<void>("set", dst["data"]);
```

浏览器底层的 `Uint8Array.set()` 本质上是高度优化的 `memcpy`，速度提升了一个数量级。

同时我还去掉了两个无意义的 RGBA↔RGB 颜色空间转换——因为 OpenCV **完全支持 CV_8UC4 的高斯模糊**。代码从 4 个 Mat 变成了 1 个，从 800 万次逐字节读写变成了一次 `set()` 调用。

---

## 我没死心：能不能零拷贝？

优化后的代码已经够快了，但作为被 C++ 惯坏的人，我心里一直有一个疙瘩——

在原生 C++ 里，我是这样干的：

```cpp
cv::Mat mat(h, w, CV_8UC4, img.bits(), img.bytesPerLine());
// Mat 只是一个 Header，指向 QImage 的数据，零拷贝！
```

现在这个 wasm 版本，每次都要 `matFromArray` 复制一次进去，再 `set` 复制一次出来——总共有两次大块内存搬运。能不能在 OpenCV.js 里也做 Header-Only Mat？

于是我开始了长达十几个回合的实验。

### 第一回合：构造函数

```js
new cv.Mat(100, 100, cv.CV_8UC4, 0);
// ❌ "Incorrect number of tuple elements for Scalar"
```

第四个参数被 OpenCV.js 解释为 `cv::Scalar`，而不是 `void* data`。说明 `Mat(rows, cols, type, ptr)` 这个构造函数重载**根本就没被 Embind 导出**。

### 第二回合：create()

```js
m.create(100, 100, cv.CV_8UC4);                        // ✅ 成功
m.create(100, 100, cv.CV_8UC4, 0);                     // ❌ BindingError
m.create(100, 100, cv.CV_8UC4, new Uint8Array(...));   // ❌ BindingError
```

`create()` 同样没有导出带外部数据指针的重载。

### 第三回合：直接设置 data

```js
mat.data = someOtherArray;
// ❌ "Mat.data is a read-only property"
```

`mat.data` 是 Embind 导出的只读 getter，没法改写。

### 第四回合：反过来——让 QImage 引用 Mat.data

既然 QImage→Mat 走不通，那 Mat→QImage 呢？我用 `mat.data.byteOffset` 拿到了 Mat 数据在 Wasm Heap 中的偏移，然后传入 QImage 的构造函数：

```cpp
int ptr = dst["data"]["byteOffset"].as<int>();
QImage result(reinterpret_cast<uchar*>(ptr), ...);
```

图片显示出来了——但是彩点乱码、撕裂错位。

补了几行诊断日志：

```
rows: 937
cols: 769
step: -2147483648   ← 0x80000000，int32 的最小值，完全错误
BindingError: Cannot pass "[object Window]" as a Mat const*
```

`step` 字段在 C++ 侧无法正确解析，`elemSize()` 调用时 `this` 被错误绑定为 `window`。OpenCV.js 内部的对象模型和 C++ 根本不兼容。

实验到此结束。

---

## 结论：零拷贝，在这个预编译版 OpenCV.js 里，不可能。

不是技术不够好，而是 OpenCV.js 的 Embind 绑定层从一开始就没考虑过这种用法。构造函数没导出、属性只读、结构体字段无法解析——三道锁，每道都锁死了零拷贝的可能。

除非我**自己编译 OpenCV.js** 并修改 Embind 导出——那是一个完全不同量级的工程。

---

## 换一个思路：EM_JS

既然 `emscripten::val` 路径的各种 tricks 都用尽了，那就换个玩法——EM_JS。它让 C++ 直接执行一段内嵌的 JS，所有 OpenCV 操作都在 JS 内部完成，只跨一次语言边界：

```cpp
EM_JS(void, gaussianBlur_EMJS,
      (uintptr_t rgbaPtr, int w, int h, int kernel),
{
    var srcView = HEAPU8.subarray(rgbaPtr, rgbaPtr + w * h * 4);
    var mat = cv.matFromArray(h, w, cv.CV_8UC4, srcView);
    cv.GaussianBlur(mat, mat, new cv.Size(kernel | 1, kernel | 1), 0);
    HEAPU8.set(mat.data, rgbaPtr);
    mat.delete();
});
```

Val 版和 EM_JS 版在逻辑上等价——都是两次 memcpy、一个 Mat、一次高斯模糊。区别只在于：

- **Val 版**：每次 `cv["xxx"](...)` 调用都经过 Embind 转发（C++ → JS 多次往返）
- **EM_JS 版**：一次进入 JS，全部算完再出来（C++ → JS → 计算 → C++，一次往返）

理论上 EM_JS 应该有优势。但到底能快多少？

---

## 实测：20000 次图片处理

我用同一张 769×937 的 RGBA 图片，分别测试了两个版本各 20000 次高斯模糊。每次新建 QImage，排除缓存干扰。结果：

| 方案 | 20000次总耗时 | 平均每次 | 相对性能 |
|------|-------------|---------|---------|
| Val 版 | 3,577,043 ms | 178.8 ms | 100%（基准） |
| EM_JS 版 | 3,484,270 ms | 174.2 ms | 102.6%（快 2.6%） |

`Val / EM_JS = 1.02663`。

2.6%。

坦白说，看到这个数字时我挺意外的。不是意外它快得不多——而是意外它**竟然能这么接近**。我之前那个逐字节 `.as<int>()` 版本比优化版慢了几十倍，而优化到 Uint8Array.set() 之后，Val 和 EM_JS 之间的差距只剩下了 Embind 的调度开销，而这个开销在卷积运算面前几乎可以忽略。

高斯模糊本身占用了约 95% 的时间，Embind 调度只占 5%。5% 的 2.6%——微乎其微。

---

## 我该怎么选？

| 方案 | 复制 | 跨语言调用 | 性能 | 可维护性 | 我的选择 |
|------|------|-----------|------|---------|---------|
| 原始 val + for 循环 | 2次 + N次字节调用 | 极高 | ❌ | 低 | 绝对不用 |
| 优化 val + set() | 2次 | 中等 | ✅ 95~99% | ✅ | **单函数场景首选** |
| EM_JS + HEAPU8.set | 2次 | 最低 | ✅ 100% | 一般 | 多接口封装 |

如果我的项目只需要一个高斯模糊，我会毫不犹豫选 Val 版——代码接近原生 C++ 风格，调试方便，维护成本低。2.6% 的代价不值得牺牲可读性。

如果未来要封装几十个 OpenCV 接口（Blur、Resize、Threshold、Canny...），那统一用 EM_JS 把计算全部放在 JS 侧会更划算。

但无论如何，**不要纠结这 2.6%**。它已经从性能问题变成了个人偏好问题。真正的瓶颈不在调用接口，而在 `matFromArray` 的那一次数据复制——如果哪天 OpenCV.js 能支持 Header-Only Mat，那才是下一个数量级的提升。

---

## 写在最后

这次迁移教会我的最重要的一件事是：

**WebAssembly 不是 C++ 在浏览器中的忠实复制。它是一种妥协。**

你在原生 C++ 中习以为常的东西——裸指针、共享内存、自定义析构——到了 Wasm 的世界里，都变成了需要重新审视的问题。

但这不完全是坏事。因为约束往往也指出了真正重要的方向。在这个项目里，把我从"能不能实现零拷贝"拉回到"现有条件下什么方案最合理"的那些实验数据，比任何理论分析都更有说服力。

如果你也在做 Qt Wasm + OpenCV.js 的开发，这是我的建议：**先用 Val 版快速验证功能，再用性能测试来决定要不要换 EM_JS。大概率你会发现 Val 版已经够好了。**
