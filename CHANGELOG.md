## 2026-07-07~2026-07-08

### 社交按钮图标集成（Icon 系统 + magic_enum 反射）

- 社交按钮从纯文字改为 SVG 图标，图标配置在 `Profile.json` 中通过 `"iconStyle"` 和 `"icon"` 字段指定，`magic_enum::enum_cast` 反射到对应枚举并加载
- 图标文件已迁移至 `resource/icons/`（AntDesignIcons / BootstrapIcons / IconPark 三套，共约 2400+ SVG）
- 图标尺寸响应式：按钮 `2.5rem × 2.5rem`，图标 `1.5rem × 1.5rem`，收口到 `MizukiButton::setRemSize(qreal rem)`

### 社交按钮图标颜色策略（对齐前端 .btn-regular）

- 深入分析参考项目（Mizuki Astro 博客）的图标颜色体系：
  - 浅色模式：`text-(--btn-content)` = `oklch(0.55 0.12 240)` = `#1479B0` → `ColorRole::BtnContent`
  - 深色模式：`dark:text-white/75` = `rgba(255,255,255,0.75)` ≈ `#BFBFBF`（**覆盖掉** `--btn-content` 的 oklch 计算值）
- 实现 `ProfileCard::socialBtnIconColor()`：浅色取 `Palette[BtnContent]`，深色返回 `#BFBFBF`
- 主题切换时通过 `Palette::appColorChange` → `updateButtonIcons()` 重建所有图标颜色
- 图标构造时用 `addIconEnum(id, QIcon::Normal, QIcon::Off, color)` 显式指定颜色，避免 `useAccentTextColor` 在深色下偏蓝

### Icon 系统移植（来自 QWidget-FancyUI）

- 从 `E:\CODE\nearest\QWidget-FancyUI` 移植以下模块，移除 `fancy::` 命名空间：

  | 文件 | 说明 |
  |---|---|
  | `src/Core/SvgParsing.h` | SVG 属性替换工具（XML DOM 解析，支持单属性/多标签递归替换） |
  | `src/Icon/IconEnums.h` | AntDesignIcons (682) / BootstrapIcons (2078) / IconPark 枚举 |
  | `src/Icon/IconEngine.h/.cpp` | 自定义 QIconEngine，支持 SVG 渲染 + 颜色替换 + 三级缓存 |
  | `src/Icon/Icon.h/.cpp` | 图标封装类，封装 IconEngine + QIcon，提供着色开关 |
  | `src/include/magic_enum/` | magic_enum 头文件库（枚举 ↔ 字符串互转） |

- 修复移植过程中的问题：
  - `IconEngine.h`：删除与 `Defs.h` 重复的 `MAGIC_ENUM_RANGE` 宏定义和 `magic_enum.hpp` include，删除冗余的前向声明
  - `IconEngine.cpp`：`ColorRole::AppAccent` → `Primary`，`AppAccentText` → `BtnContent`，`Text` → `Text`（新增）
  - `IconEngine.cpp::cacheAll()`：修复 `return cache` 在 for 循环体内的 bug（仅缓存一个图标 → 移到循环后），IconPark 分支枚举类型 `BootstrapIcons` → `IconPark`
  - `Defs.h`：新增 `ColorRole::Text`（`#404040` / `#BFBFBF`），新增 `qHash(QColor)` 以支持 `QHash<QColor, QSvgRenderer*>`

### ProfileCard 社交链接 JSON 配置化

- 新增 `resource/Profiles/Profile.json`：`name` / `bio` / `socialLinks[]`（含 `name` / `url` / `iconStyle` / `icon`），可自定义增减
- `ResourcePath.h`：新增 `Local::Profile = ":/Profiles/Profile.json"`
- `rcc.qrc`：新增 `<file>Profiles/Profile.json</file>`
- `ProfileCard.cpp`：删除硬编码 `kSocialLinks[]` 静态数组和日文默认文本，改为 `loadProfileConfig()` 从 JSON 解析，解析失败保留占位文本 `"名字"` / `"签名"` 并打 warning
- 按钮点击从 lambda 重构为成员槽 `onSocialButtonClicked()`，URL 通过 `btn->setProperty("url", url)` 传递
- 删除了 `goto` 语句和 `TRY_CAST_ICON` 宏，改用 switch + 模板辅助函数 `tryCastIcon<E>` 和 if-else 链

### ColorProfile.json 调色板补充

- `fixedColors` 新增 `"Text"` 角色：`{ "light": "#404040", "dark": "#BFBFBF" }`——对齐参考项目正文文本色 `text-black/75 dark:text-white/75`

## 2026-06-26

### ProfileCard 构造函数拆解 + 社交按钮

- 子组件初始化封装为 4 个函数：`setupAvatar()` / `setupNameLabel()` / `setupBioLabel()` / `setupSocialButtons()`
- 新增 4 个 MizukiButton 社交按钮（Bilibili / Gitee / GitHub / Codeberg），水平居中排列
- 按钮尺寸响应式：`h-10 w-10 = 2.5rem`，间距 `gap-2 = 0.5rem`，均跟随 `currentRem()` 缩放
- 链接配置 `kSocialLinks[]` 静态数组，clicked → `QDesktopServices::openUrl`
- 暂不处理图标

### MizukiButton 绘制分离到 MizukiButtonStyle（QProxyStyle）

- 新增 `MizukiButtonStyle`（`QProxyStyle` 子类），参照 QWidget-FancyUI PushButtonStyle 模式
- `drawControl(CE_PushButtonBevel)` 拦截按钮斜面绘制，改为圆角背景
- ControlState（isDown / isHover）直接在代码中展开，不引入额外类
- `MizukiButton` 简化为构造 + `setStyle(MizukiButtonStyle)` + `setCornerRadius` 委托
- 移除 `paintEvent` override、`setFlat`、`setAutoFillBackground`

### ProfileCard 名字 + 签名 QLabel

- 头像下方新增两个 QLabel：
  - 名字 `_nameLabel`：**"まつざか ゆき"**，bold，居中，颜色 `ColorRole::DeepText`
  - 签名 `_bioLabel`：**"世界は大きい、君は行かなければならない"**，居中，颜色 `ColorRole::ContentMeta`
- 字体统一使用 **Zen Maru Gothic**（`f.setFamily("Zen Maru Gothic")`）
- `reSetLayout()` 中实现响应式字号，对齐 Mizuki 的 JS 动态 rem 缩放：

  | 窗口 CSS 像素 | base | scale | 名字 (1.25rem) | 签名 (1rem) |
  |:---|---:|---:|---:|---:|
  | < 768 | 14px | 1.0 | 17.5px | 14px |
  | 768–1280 | 16px | 1.0 | 20px | 16px |
  | 1280–1700 | 16px | 0.85 | 17px | 13.6px |
  | 1700–2000 | 16px | 0.85→1.0 线性 | 17→20px | 13.6→16px |
  | > 2000 | 16px | 1.0 | 20px | 16px |

- 字体族（Zen Maru Gothic）和加粗（名字 bold）移入构造函数，`reSetLayout()` 仅设 `setPixelSize`
- 颜色硬编码（QPalette）：名字白色 `Qt::white`，签名 `QColor(163,163,163)`（`text-neutral-400`，亮/深模式同值）
- 移除 `updateLabelColors()` 方法和 `Palette::appColorChange` 连接

### ProfileCard 分隔线（paintEvent 绘制）

- 新增 `paintEvent`：先调 `MizukiCard::paintEvent` 绘制背景，再绘制蓝色横向分隔线
- 分隔线对齐前端元素：`<div class="h-1 w-5 bg-[var(--primary)] mx-auto rounded-full mb-2">`
  - `w-5` = 1.25rem 宽，`h-1` = 0.25rem 高，`rounded-full` 两端圆弧（r = h/2）
  - 颜色 `ColorRole::Primary`（跟随主题 hue）
  - 水平居中，垂直位于名字底部 + `mb-1`(0.25rem) 间距
- 不缓存坐标：`paintEvent` 中从 `_nameLabel->font().pixelSize() / 1.25` 实时反推 `rem`，避免 reSetLayout 100ms debounce 导致的绘制延迟

### MizukiCard 通用圆角卡片基类提取

- 从 `ProfileCard` 中提取纯绘制职责，创建独立通用组件 `src/Widgets/MizukiCard/MizukiCard.h/.cpp`
- **MizukiCard 职责**：仅负责 `paintEvent` 绘制圆角背景（`CardBg`）与边框（`LineDivider`），颜色由 `Palette` + `ThemeModeController` 信号自动驱动
- 暴露出 `setCornerRadius(int)` / `cornerRadius()` 接口，默认 16px
- **ProfileCard 重构**：继承由 `QWidget` 改为 `MizukiCard`，移除 paintEvent、颜色信号连接、`WA_TranslucentBackground` / `setAutoFillBackground` 设置（均下沉至 MizukiCard）
- ProfileCard 仅保留布局管理、AvatarButton 异步加载、`reSetLayout()` 响应式边距计算
- **职责分离**：MizukiCard 负责外观（背景+圆角+边框），ProfileCard 负责内容布局，后续任何需要圆角卡片的地方直接继承 MizukiCard 即可

## 2026-06-25

### Palette 重构：JSON 单一色彩来源 + oklch 参数化

- **移除双源头**：删除 C++ 硬编码 `oklchDefs[]` 数组（~60行）和 `static_assert`
- **ColorProfile.json 重构**：拆分为 `oklchParams`（参数色 L/C，不含 hue）和 `fixedColors`（固定色 hex），JSON 成为唯一色彩数据源
- Default 方案由 `oklchParams + hue=240°` 在运行时通过 `oklchToColor()` 实时计算，不再存储预计算 hex 值
- `generateCustomScheme(hue)` 改为遍历 `_oklchParams` QHash 驱动，固定色从 Default 复制，参数色实时计算
- 新增 `OklchRoleParams` 结构体（含 `isFixed` 标记）和 `QHash<ColorRole, OklchRoleParams> _oklchParams` 成员
- 删除 `OklchParam`、`oklchParam()`、`loadColorProfile()` 等旧接口

#### 修复与参考项目 Mizuki 的不一致

| ColorRole | 旧值 | 新值（对齐 Mizuki variables.styl） |
|:--|:--|:--|
| FloatPanelBg Dark L | 0.23 | **0.19** |
| DeepText Dark | L=0.85 C=0.02（多余深色变体） | 删除，仅保留 L=0.25 C=0.02 |
| TitleActive Dark | L=0.70 C=0.10（多余深色变体） | 删除，仅保留 L=0.60 C=0.10 |

### ColorRole 改用 QMetaEnum 反射

- `ColorRole` 枚举移入 `ColorRoles` 命名空间，添加 `Q_NAMESPACE` + `Q_ENUM_NS(ColorRole)` + `using ColorRoles::ColorRole`
- 删除 `colorRoleNames[]` 字符串数组（~24行）和对应 `static_assert`
- `keyToRole()` 从手工循环改为 `QMetaEnum::fromType<ColorRole>().keyToValue()`，一行搞定
- 新增 `#include <QMetaEnum>` 和 `#include <QObject>`（Defs.h 中）

### 组件颜色统一迁移到 Palette

#### ContentWidget — PageBg
- `paintEvent` 硬编码 `0xE9F0F5` / `0x080E13` → `Palette::instance()[ColorRole::PageBg]`
- 删除 `_middleBar` / `_rightBar` 的调试 `setStyleSheet("background-color: ...")`
- 连接 `Palette::appColorChange` → `update()`

#### ProfileCard — CardBg + LineDivider
- `paintEvent` 新增圆角卡片背景绘制：`QPainterPath::addRoundedRect(16px)` + `fillPath(CardBg)` + `drawRoundedRect(LineDivider, 1px)`
- 删除硬编码 `Qt::red` 填充
- 连接 `Palette::appColorChange` → `update()`

#### 颜色缓存移除
- ContentWidget 删除 `_pageBg` 成员 + `updatePageBg()` 槽，paintEvent 直接读取 Palette
- ProfileCard 删除 `_cardBg` / `_lineDivider` 成员 + `updateCardColors()` 槽，paintEvent 直接读取
- 性能分析：QHash 查找 + QColor 隐式共享返回 ~25ns，vs QPainter::fillRect ~500μs，差 20000 倍，缓存无意义
- WaveWidget 保留缓存（60fps + 4层 alpha 变体 + Pixmap 预渲染，需缓存）

### 资源路径统一管理：ResourcePath.h

- 新建 `src/Core/ResourcePath.h`，所有后端 URL / 本地 qrc 路径集中一处维护
- 结构：
  ```
  ResourcePath::Server::Fonts/FontUrls/CategoryHomepage/CategoryProfile/
                       apiMaxImageId()/imageUrl()/avatarUrl()
  ResourcePath::Local:: fallbackHomeImage()/FallbackAvatar/ColorProfile
  ResourcePath::serverBase()   // Win: "http://localhost:8000/"  WASM: ""
  ```
- `ResourceManager.cpp` 所有硬编码路径替换为 ResourcePath 常量/函数
- `Palette.cpp` `":/Profiles/..."` → `ResourcePath::Local::ColorProfile`

### 头像异步加载

- **ResourceManager** 新增 `loadAvatar()` slot + `avatarReady(QPixmap)` 信号 + `onAvatarReplyFinished()` 回调
- 请求 `Images/Profile/avatar.jpg`（Django 兜底静态路由覆盖），失败回退 `:/image/avatar.jpg`
- **ProfileCard** 构造中删除 `_avatar->setAvatarPixmap(...)` 硬编码，改为 `connect(avatarReady, _avatar, setAvatarPixmap)` + `loadAvatar()`

### 自定义滚动条移植（来自 QWidget-FancyUI）

- 新建 `src/Widgets/ScrollBar/` 目录：
  - `ScrollBar.h/.cpp` — QScrollBar 子类，构造自动安装 ScrollBarStyle
  - `ScrollBarStyle.h/.cpp` — QProxyStyle 子类，自定义 `drawComplexControl` + `pixelMetric` + `drawArrow`
- **硬编码颜色**（取自 FancyUI ColorProfile.json）：
  | | Light | Dark |
  |:--|:--|:--|
  | 轨道 | `#F9F9F9` | `#2C2C2C` |
  | 滑块/箭头 | `#8A8A8A` | `#9F9F9F` |
- **绘制逻辑**：轨道直角 `drawRect`，滑块圆角 9px，三角箭头 `drawArrow()`（仅悬停时 / 始终），PM_ScrollBarExtent=15，SliderMin=18
- **ScrollArea.cpp** 构造中 `setVerticalScrollBar(new ScrollBar)` + `setHorizontalScrollBar(new ScrollBar)`
- Direction 枚举（Up/Down/Left/Right）供箭头方向判定
- 箭头区域通过 `QRectF{...}.adjusted()` 精细控制尺寸和位置，多次迭代调整至视觉满意
- 后期移除平台 `#ifdef __EMSCRIPTEN__` 分支，统一双端绘制逻辑

### WASM index.html 修复

- `<script src="GithubPage.js">` → `Mizuki-Qt.js`
- `entryFunction: window.GithubPage_entry` → `window.Mizuki_Qt_entry`
- 加载副标题 "GithubPage" → "Mizuki-Qt"

### 技术沉淀

- oklch vs HSV 色彩空间：oklch 感知均匀，旋转 H 时人眼亮度/鲜艳度不变；HSV 在极端亮度下色相变化不可察觉
- sRGB→oklch 逆变换数学成立（精度 <1/255），可作为替代 oklchDefs 的方案（未采用）
- QColor 隐式共享 + 按值返回零额外开销（4×ushort），适合 paintEvent 直接读取
- Qt6 QHash const operator[] 返回临时拷贝非引用，需注意链式调用的悬垂问题
- Q_NAMESPACE + Q_ENUM_NS 可在命名空间内注册枚举，AUTOMOC 自动识别
- `QHash::operator[] const` (Qt6) 与 `QHash::value(const Key &)` (Qt5) 语义差异：前者返回临时拷贝非引用
- `QWidget::setFixedHeight/Width` 的内部机制：`setMinimumXXX` + `setMaximumXXX` + `updateGeometry()` → post `LayoutRequest`（异步），`QWidget::size()` 在函数调用后仍返回旧值
- Qt Resource System 中 `<qresource prefix>` + `<file>` 路径拼接为 `prefix/file_relative_path`，需用 `alias` 覆盖文件名部分
- HSL 色相旋转的局限：L>90% 或 L<10% 时色相变化肉眼不可分辨，不适合做主题色变换
- CSS 变量 + oklch 调色盘架构：一个 `--hue` 变量驱动全站所有组件，L/C 由设计师锁死保证明暗关系不崩

### 项目结构变更

- 新建 `src/Widgets/ScrollBar/`（ScrollBar + ScrollBarStyle）
- 新建 `src/Core/ResourcePath.h`
- 删除 `src/Core/Paltette/Palette.cpp` 中 `oklchDefs[]`、`OklchDef`、`oklchParam()`、`loadColorProfile()`
- 删除 `src/Widgets/ScrollArea/CustomScrollBar.h/.cpp`（已被 ScrollBar/ 替代）
- 创建 `resource/Profiles/Palette/` 目录，存放 `ColorProfile.json`
- `src/Core/Paltette/` — Palette 颜色管理系统

## 2026-06-23 ~ 2026-06-24

### 响应式布局尺寸系统

- **ContentWidget** — 三栏布局响应式参数：
  | 窗口宽度 | 侧边栏宽 | parcel 左右边距 | 卡片间距 | Profile 卡片边距 |
  |:---|:---|:---|:---|:---|
  | ≤1280 | 280 固定 | 16 | 15~17 线性 | 12 |
  | 1280~1715 | 238 固定 | 13 | 同上 | 10 |
  | 1715~2015 | 线性 0.140x-1.66 (238→280) | 阶梯 14→13→14→15 | 同上 | 阶梯 10→11→12 |
  | ≥2015 | 280 固定 | 16 | 同上 | 12 |
- **ProfileCard** 自身 padding/spacing 阶梯：`≤1280=12 / 1280~1765=10 / 1765~1946=11 / >1946=12`
- 各组件独立连接 `SizeManager::sizeAdjustmentCompleted`，自行管理尺寸计算 + `std::clamp` 上下限

### SizeManager 信号驱动模型的时序 Bug 修复

**问题**：最大化窗口时 `WaveWidget` 停死，垂直调整时 `restartCurrentAnimation` 先于 `resizeEvent` 触发。

**根因**：`BasicFramework::setSize()` 中 `_wave->setFixedHeight(waveH)` 调 `updateGeometry()` → post 异步 `LayoutRequest` → 在 `restartCurrentAnimation()` 启动 tickTimer 后被下一事件循环处理 → 触发 `WaveWidget::resizeEvent` → `_tickTimer.stop()` → debounce 已耗尽无人重启。

**修复**：移除 `BasicFramework` 对外部组件尺寸的跨对象设定，将 `_wave->setFixedHeight` / `_bannerCarousel->setFixedHeight` 移入各自 `restartCurrentAnimation` 内部，与 render/start 同函数栈执行。`resizeEvent` 清空（仅依赖 SizeManager 信号驱动），切断异步 LayoutRequest 对动画的不当干扰。

**BannerCarouselWidget** 同样修复：抽取 `restartZoomForCurrent()`（不顾停旧动画，仅供 crossfade 路径使用）与 `restartCurrentAnimation()`（全停全重建，供 resize 使用）两条分离路径。

### QScrollArea 滚动修复

- `BasicFramework` 原 `QVBoxLayout(this)` 直接设在 QScrollArea 本体上，覆盖了 Qt 内部 viewport/滚动条布局 → 退化普通 QFrame，永不出现滚动条
- 修复：新增 `QWidget *_scrollContent`，`_vboxLayout` 挂载其下，`setWidget(_scrollContent)` + `setWidgetResizable(true)` → 标准 QScrollArea 模式

### AvatarButton 头像组件

- 创建 `src/Widgets/ProfileCard/AvatarButton.h/.cpp`，继承 `QPushButton`
- 自定义 `paintEvent`：不绘制按钮基类样式，圆角裁剪 + 白色背景 + 等比缩放居中
- 图片缩放由 `SizeManager` 信号驱动，`recalcSizeAndPixmap()` 先 `setFixedSize` 再用 `side` 变量缩放（避免 `size()` 异步滞后）
- `updateFillColors` 收口到 `Palette::instance()[ColorRole::PageBg]`（由 `Palette::appColorChange` 驱动，替代原 ThemeModeController）

### CMake 构建系统

- `qt_add_resources()` 替代 GLOB_RECURSE 中的 qrc 通配，保证 qrc 内部 `<file>` 变更自动触发 rcc
- 移除 `"${PROJECT_SOURCE_DIR}/resource/*.qrc"` 通配项

### 弃用 OpenCV.js，全平台改用纯 Qt/C++ 手动实现

- **`FImage.h`**：
  - `#ifdef __EMSCRIPTEN__` → `#if 0`，注释掉 WasmCV 命名空间 + EM_JS 函数声明（保留可恢复）
  - 方向性模糊 6 个接口 `#ifdef __EMSCRIPTEN__`/`#endif` 改为 `//` 注释，全平台可用
  - `toCvType` 声明的 `#ifdef __EMSCRIPTEN__`/`#endif` 改为 `//` 注释
- **`FImage.cpp`**：
  - `#ifdef __EMSCRIPTEN__` → `#if 0`，注释掉所有 EM_JS 实现和 WASM 版 FImage 方法
  - `#ifdef Q_OS_WIN` → `#if 1`，原手动实现改为全平台通用
  - 新增 5 个手动辅助函数：horizontalGaussianBlurInternal / verticalGaussianBlurInternal / horizontalBoxBlurInternal / verticalBoxBlurInternal / medianBlurInternal
  - 新增 6 个全平台方法实现：horizontalGaussianBlur / verticalGaussianBlur / uniformBlur / horizontalUniforBlur / verticalUniforBlur / medianBlur
- **`index.html`**：`<script src="js/opencv.js">` → HTML 注释（保留可恢复）
- **保留 `opencv.js` 文件不删除**

### 图片请求简化：去掉随机，改为顺序循环 + 图片静态化

- **Django `urls.py`**：移除 `/images/homepage/<int:img_id>/` 路由
- **Django `views.py`**：删除 `homepage_image()` 视图，移除 `mimetypes` 导入
- **Qt `ResourceManager`**：`requestImageByRandomId()` → `requestNextImageById()`，1→max→1 顺序循环；移除 `QRandomGenerator` include
- **`www/Images/Homepage/`**：`6.png` → `6.jpg`，删除 `1 - 副本.jpg`

## 2026-06-20

### 跨平台改造：WASM → WASM + Windows 桌面

为项目添加 Windows MSVC 桌面编译支持，通过 `#ifdef Q_OS_WIN` / `#ifdef __EMSCRIPTEN__` 条件分支实现双平台共存。

#### CMakeLists.txt — 构建系统

- `QT_WASM_INITIAL_MEMORY "80MB"` 用 `if(EMSCRIPTEN)` 包裹，仅在 Emscripten 工具链下生效
- POST_BUILD 产物复制（.js/.wasm/qtloader.js → run/server/www）用 `if(EMSCRIPTEN)` 包裹
- 非 WASM 构建时两者跳过不执行，MSVC 正常生成 .exe

#### SystemThemeMonitor.h/.cpp — 系统主题检测

| 平台 | 基类 | 实现方式 |
|:--:|:--:|------|
| WASM | `QObject` | `EM_ASM_INT(matchMedia)` 同步查询 + `EM_ASM(addEventListener)` 事件监听 + `EMSCRIPTEN_KEEPALIVE` JS→C++ 桥接 |
| Win | `QThread` | `RegOpenKeyEx(HKCU, Personalize, AppsUseLightTheme)` 读注册表 + `RegNotifyChangeKeyValue` + `WaitForMultipleObjects(3)` 线程阻塞轮询 |

- 公共接口不变：`SystemTheme()` / `start()` / `stop()` / `systemThemeChanged(Theme)`
- Win 额外提供 `systemAccentColorsChanged(QMap<SysAccentPalette, QColor>)` 信号及 `SysAccentPalette` 枚举，`run()` 中同时监听主题注册表 + DWM 强调色注册表

#### ResourceManager.cpp — 资源加载

- 新增 `SERVER_BASE` 条件定义：Win = `http://localhost:8000/`，WASM = 空串（浏览器自动以页面 origin 解析相对 URL）
- 3 处 URL 构造（字体 / API-max_id / 图片）拼上 `SERVER_BASE` 前缀
- 其余逻辑（异步请求、max_id 状态机、fallback 兜底、信号链 `homeImageReady`）零改动
- 移除 `#include <QJsonDocument>` / `#include <QJsonObject>`（后端接口简化后不再返回 JSON）

#### FImage.h/.cpp — 图像处理

- 头文件：`WasmCV` 命名空间 + 6 个 WASM-only 接口（horizontalGaussianBlur / verticalGaussianBlur / uniformBlur / horizontalUniforBlur / verticalUniforBlur / medianBlur）用 `#ifdef __EMSCRIPTEN__` 包裹
- 3 个跨平台接口：`gaussianBlur` / `boxBlur` / `approxGaussianBlur`
- .cpp 三层结构：
  1. `#ifdef __EMSCRIPTEN__`：9 个 `EM_JS(...)` + `toCvType` + 全部 9 个方法（调用 OpenCV.js，不变）
  2. `#ifdef Q_OS_WIN`：`gaussianBlurInternal`（用户提供的 Qt 纯手写两遍分离卷积函数，原样保留）+ `boxBlurInternal`（参照高斯模糊风格手写）+ 3 个方法
  3. 共享：构造/赋值/`ensureProcessable`/`impulseNoise`/`greyScale`

#### boxBlurInternal 滑动窗口优化

- 初版为朴素 O(W×H×K) 实现，与高斯模糊同复杂度
- 优化为滑动窗口 O(W×H)：均值核所有权重相等，相邻像素仅左/右边界变化
  ```
  sum_new = sum_old - src[clamp(x-radius)] + src[clamp(x+radius+1)]
  ```
- 横向/纵向各一遍，norm 在写入时乘入（避免浮点漂移累积）
- 高斯模糊因核权重非均匀，无法使用此优化

### Django API 接口简化

- `/api/images/max_id/<str:category>/` 从返回 JSON `{"max_id": 4}` → 纯文本 `"4"`（`HttpResponse(str(max_id), content_type="text/plain")`）
- Qt 前端 `onMaxIdReplyFinished()` 从 `QJsonDocument::fromJson()` → `reply->readAll().trimmed().toInt()`
- 移除 `views.py` 中 `JsonResponse` 导入

### 技术沉淀

- `QStringLiteral` vs `QString`：编译器期 UTF-16 → 只读数据段 vs 运行时堆分配+解码；适用于编译期已知字面量，动态拼接只能用 `QString`
- 静态局部变量 + qApp 父对象 = 双重析构定时炸弹：
  - 指针语义（`static auto *p = new T(qApp)`）：程序结束时不析构指针，但 qApp 析构后 p 悬空
  - 值语义（`static T inst(qApp)`）：qApp 析构 + C++ static 析构 → 双重析构 → 堆损坏
- Windows 注册表主题路径：`HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme`（DWORD：0=深色 1=浅色）
- 方块模糊可用滑动窗口降复杂度至 O(W×H)；高斯模糊因核权重非均匀无法使用此优化

## 2026-06-19 

### TypewriterLabel 打字机组件

- 创建 `src/Widgets/TypewriterLabel/TypewriterLabel.h/.cpp`，QLabel 子类 + QTimer + 状态机
- 状态机：Type(打字 100ms/字) → Pause(暂停 2000ms) → Delete(删除 50ms/字) → Next(下一条)，无限循环
- 单条文本打完即停不删除；多条文本才走删除循环
- 字号默认 30px（`setPixelSize(30)`，对齐 Mizuki `lg:text-3xl = 1.875rem`）
- 21 句轮播古诗/词，带出处破折号标注
- 文本列表支持任意数量扩展；`setTexts()`

### 字体栈

- 全局字体从思源黑体切换为梦源黑体：W5-Regular(400) / W7-Medium(500) / W9-Bold(700)
- ResourceManager 异步加载 5 种字体：
  - `font/DreamHanSansCN/DreamHanSansCN-W5.ttf`
  - `font/DreamHanSansCN/DreamHanSansCN-W7.ttf`
  - `font/DreamHanSansCN/DreamHanSansCN-W9.ttf`
  - `font/loli.ttf`（Heiti TC / 萝莉体 第二版）
  - `font/ZenMaruGothic-Medium.ttf`（Zen Maru Gothic）
- 默认字体 `QApplication::setFont(QFont("Dream Han Sans CN", 10))` 不变
- 打字机使用 Zen Maru Gothic；标题同样使用 Zen Maru Gothic（后因单字重问题未解）

### 响应式字号对齐 Mizuki 博客

- 修改 `BasicFramework::setSize()` 和 `Widget::initMainUI()`
- 四级断点对齐 Mizuki 的 `banner.css` 覆盖规则 + Tailwind 类 + JS 页面缩放：
  | 断点 | 标题 (banner-title) | 打字机 (banner-subtitle) |
  |:----:|:-------------------:|:------------------------:|
  | ≤479px | 45px | 14px |
  | 480-767px | 53px | 16px |
  | 768-1279px | 64 × s | 24 × s |
  | ≥1280px | 96 × s | 30 × s |
- s = clamp(0.85, vw/2000, 1.0)，模拟 JS `document.documentElement.style.fontSize = scale*100 + '%'`
- vw/vh 归一化到 CSS 像素：`qRound(window()->width() / window()->devicePixelRatio())`
- `s` 用 `(vw > 1280)` 对齐 JS `window.innerWidth > 1280` 判断（区别于 CSS `min-width: 1280px`）
- 三目运算符压缩代码行数，注释一行概括映射关系

### Banner 标题与阴影

- 标题文字 "COLORREF"，白色，居中对齐
- `QGraphicsDropShadowEffect` 文字阴影，参数对齐 Mizuki CSS：
  - 标题：offset(1,1) blur=6 alpha=50%（`text-shadow: 2px 2px 4px rgba(0,0,0,0.7)`）
  - 打字机：offset(1,1) blur=2 alpha=60%（`text-shadow: 1px 1px 2px rgba(0,0,0,0.6)`）
- QGraphicsDropShadowEffect 本质是盒子阴影非文字阴影，通过增大 blur + 降低 alpha 补偿

### Mizuki 博客参考数据

#### Banner 标题 "わたしの部屋"
- 字体族：ZenMaruGothic-Medium（asciiFont）→ 萝莉体 第二版（cjkFont 回退）→ system-ui
- 加粗：`font-weight: bold`
- 颜色：`text-white`（纯白）
- 字号规则（Tailwind + banner.css 覆盖）：
  - ≤479px: 3.2rem !important
  - 480-767px: 3.8rem !important
  - 768-1279px: 4rem !important
  - ≥1280px: 6rem（`lg:text-8xl`，banner.css 桌面段无覆盖）
- rem 基值：<768px=14px, ≥768px=16px, 桌面叠加页面缩放
- Tailwind lg: 断点被项目重写为 1280px（非默认 1024px）

#### 打字机效果（TypewriterText.astro）
- 5 句日文轮播：特别なことはないけど… / 今でもあなたは私の光 / 君ってさ… / 君と話すと… / 今日はなんでもない日…
- 参数：speed=100ms, deleteSpeed=50ms, pauseTime=2000ms
- 字号：≤479px=1rem, 480-767px=1.125rem, 768-1279px=1.5rem, ≥1280px=1.875rem

### 技术讨论

- 字体族名/字重嵌入 ttf name 表（NameID=1/16）与 OS/2 表 weight class；Qt 按 OS/2 数值匹配，非文件名
- `QFont` 构造函数第二个参数是 pointSize 非 pixelSize（30pt ≈ 40px），`setPixelSize(30)` 才是 30px
- QFontDatabase::addApplicationFont 仅注册字体到池，QApplication::setFont 设置全局默认；字体可单独 setFont 到控件
- XHR responseType='arraybuffer' 时访问 .responseText 违反 W3C 规范抛 InvalidStateError。查明原因为浏览器扩展 inspector.js 拦截 XHR 导致误报（非 Qt/Django 代码问题）
- Qt/WASM 中 `window()->width()` 可能返回设备像素，需除以 `devicePixelRatio()` 归一化到 CSS 像素
- JS `window.innerWidth`（含滚动条）与 CSS `min-width`（视口宽度）差异导致缩放激活区间偏移

## 2026-06-17~2026-06-18

### WaveWidget: 4 层波浪叠加动画（SVG → Qt/C++ WASM 移植）

- 参考 Mizuki 博客（Astro + SVG `#gentle-wave`）的波浪效果，在轮番图下方实现 4 层正弦波叠加动画
- **层次参数**（从 SVG `viewBox="0 20 150 32"` 提取）：
  | 层 | opacity | duration | SVG y-offset | 波峰占 widget 高度 | 波谷 |
  |:--:|:-------:|:--------:|:------------:|:------------------:|:----:|
  | 1 | 0.25 | 7s | 0 | 0% | y=h |
  | 2 | 0.50 | 10s | 3 | h×3/32≈9.4% | y=h |
  | 3 | 0.75 | 13s | 5 | h×5/32≈15.6% | y=h |
  | 4 | 1.0 | 20s | 7 | h×7/32≈21.9% | y=h |
- 每层独立构建路径：amplitude = h × remainingRatio / 2, yOffset = h × offsetRatio + amplitude，保证波谷精确落在 y=h 不越界

### WaveWidget 动画驱动演进

- **初版**：4 个 `QVariantAnimation` 各自独立驱动 → 4 次 `update()`/帧，WASM 主线程超过 50ms
- **相位差方案一（有 Bug）**：平移起止值 `start = -w + w×phase` → offset 超出 `[-w, 0]` 越界，可视窗口落在路径 `[0, 2w]` 之外产生垂直黑边
- **相位差方案二**：sin 公式嵌入 `SinPhase = -2π × (delay/duration)`，动画统一 `-w → 0`
- **最终方案**：合并为 1 个 `QTimer`(28ms≈35fps) + `QElapsedTimer` 统一时间源，相位差通过 `elapsed + delayMs` 在时间域表达
  ```cpp
  offset = -w + w × fmod((elapsed + delayMs) / durationMs, 1.0)
  ```
- 4 层参数常量化：`Duration[]` `Delay[]` `OffsetRatio[]` `RemainingRatio[]` `Alpha[]`，所有函数改为 for 循环

### WaveWidget 渲染性能优化

#### 优化 A：单 Timer 驱动
- 移除 4 个 `QVariantAnimation`，每帧只触发 1 次 `paintEvent`，WASM 桥开销降 75%

#### 优化 B：采样精度调整
- `SamplingAccuracy: 1 → 3 → 1`（配合 Pixmap 缓存后改回 1 白嫖精度）

#### 优化 C：QPainterPath → QPixmap 预渲染缓存
- 路径仅在 resize/主题切换时构建并渲染到 `QPixmap(2w, h)`
- `paintEvent` 简化为 4 次 `drawPixmap`：无 save/restore、无抗锯齿、无 fillPath
- 内存开销：4 张 Pixmap 典型 1200×150 约 5.76MB
- 帧率从 55-60ms 降到 28ms 间隔以下

#### 颜色优化
- 移除 `painter.setOpacity()` / `painter.save()` / `painter.restore()`
- `_colors[4]` 数组 QColor 构造时直接嵌入 alpha（25%/50%/75%/100%）
- `updateFillColors()` 主题切换时重建 Pixmap（低频操作）

### WaveWidget 代码结构

- `WaveWidget.h`：类级文档注释，所有成员变量右侧用途说明，方法简要说明
- `WaveWidget.cpp`：5 个常量数组完整注释（SVG 对应关系），每个函数功能+关键行内注释
- 架构总结：paintEvent 纯纹理 blit → resize/theme 时一次重建 → 单 Timer 驱动全部 4 层

## 2026-06-16

### Django 图片 API

- 新增 `/images/homepage/<int:img_id>/` 路由：前端传数字，后端 `Path.glob(f"{img_id}.*")` 匹配文件，`mimetypes.guess_type` 自动设置 Content-Type
- 新增 `/api/images/max_id/<str:category>/` 路由：返回 JSON `{"category":"Homepage","max_id":4}`，支持 `Images/` 下任意分类目录扩展
- 全部 URL 补注释说明作用与示例访问格式

### ResourceManager: loadHomeImage 完整实现

- 新增成员：`m_homepageMaxId`（0=未获取）、`m_lastHomeImageId`（0=首次）、`m_maxIdRequestInFlight`（防并发）、`m_fallbackIdx`（兜底轮换）
- `loadHomeImage()` 设计为两阶段链式调用：若 max_id 未就绪 → 先异步获取 max_id → 回调中调用 `requestImageByRandomId()`
- 随机不重复算法（排除法，均匀分布）：max=1 永远选 1；首次全随机 `bounded(1,max+1)`；正常从 `[1,max]` 排除 `m_lastHomeImageId`（`range=max-1`，若 `chosen≥last` 则 +1 跳过）
- 兜底机制：max_id 请求失败时自动交替返回 `:/image/1.webp` ↔ `:/image/2.webp`，服务恢复后下次 loadHomeImage 自动重试 max_id 并切回网络图片

### ResourceManager 异步方案演进

- 初版 `QEventLoop::exec()` 同步等待 → WASM 报 `WaitForMoreEvents not supported without asyncify`（单线程协作式事件循环，阻塞导致网络回调永远无法递送）
- 改为异步 + `m_homeImageLoadPending` flag 暂挂 → 简化为两阶段链式（`loadHomeImage` 内部自行获取 max_id）
- QtFuture `.then()` 链式 → WASM 不支持 QFuture → 回归传统 `connect` + 命名成员槽（`onMaxIdReplyFinished` / `onImageReplyFinished`）
- `Qt::SingleShotConnection` 移除：WASM 中 `QNetworkReply::finished` 以排队方式递送，`SingleShotConnection` 可能在排队事件处理前断开，导致 `postEvent: Unexpected null receiver` → `getWasmTableEntry is not a function` 崩溃
- 删除构造时异步请求 max_id（`requestHomepageMaxId` 函数并入 `loadHomeImage` 内部），消除与 `QTimer::singleShot(0)` 的竞态窗口

### BannerCarouselWidget: 动画启动拆分 + 首图自愈

- `startZoomAnimation()` 拆分为两个函数：
  - `restartCurrentAnimation()` — 仅重置当前图预缩放 + zoom + fade timer，**不 emit nextImageNeeded**（resize 专用）
  - `startZoomAnimation()` — 调用 `restartCurrentAnimation()` + `emit nextImageNeeded()`（首图 / 轮播切换专用）
- `setPixmap()` 新增首图自愈：填入 `_current` 槽位且动画未运行时自动调用 `startZoomAnimation()`，解决首图异步延迟到达导致 debounce 过期吃掉启动的永久白屏问题
- `resizeEvent → _debounceTimer` 改绑 `restartCurrentAnimation`，窗口 resize 不再触发多余网络请求
- 调用点重新分配：`setPixmap`/`onFadeOutStart` → `startZoomAnimation`；`resizeEvent` → `restartCurrentAnimation`

### BasicFramework: 接入异步图片流水线

- 本地静态图片列表循环 → 两行信号直连：
  ```cpp
  connect(_centeredImageWidget, &BannerCarouselWidget::nextImageNeeded, &ResourceManager::manager(), &ResourceManager::loadHomeImage);
  connect(&ResourceManager::manager(), &ResourceManager::homeImageReady, _centeredImageWidget, &BannerCarouselWidget::setPixmap);
  ```
- 全链路零阻塞：`nextImageNeeded` → `loadHomeImage`（发射 HTTP 立刻返回）→ 事件循环照常运转 → HTTP 响应到达 → `homeImageReady` → `setPixmap`

### WASM 兼容性沉淀

- QEventLoop 同步阻塞在 WASM 完全不可用（主线程 WaitForMoreEvents 不支持，需 Asyncify 编译标志）
- QFuture/QtFuture 在 WASM 不支持
- `Qt::SingleShotConnection` 与 WASM 排队信号递送不兼容（信号排队 → 连接已断 → 空接收者崩溃）
- 正确模式：传统 `connect` + 成员槽，依赖 `QNetworkReply::finished` 仅发射一次 + `deleteLater()` 自动清理

### 技术讨论

- 网络延迟对轮播状态机影响分析：首图延迟→永久白屏（debounce 过期 ← 已修复）；循环延迟→同图多播一轮（onFadeOutStart 有重试机制）；重复 nextImageNeeded 浪费带宽；异步 setPixmap 纯被动写入致错过切换窗口
- `std::call_once` vs `m_maxIdRequestInFlight`：`call_once` 需同步语义抛异常才可重试，与 WASM 异步模型矛盾；手动 flag 可重置且完全兼容异步回调
- 随机排除法数学验证：`range=m-1`, `raw∈[0,m-2]`, `mapped=raw+1`, 若 `mapped≥last` 则 `++mapped` → 均匀分布

## 2026-06-15

### CenteredImageWidget 动画性能优化
- 预缩放策略：`scaled()` 从每帧调用改为仅 `startZoomAnimation` 时执行一次到最大尺寸（`size()*kZoomEnd`），paintEvent 用 `painter.translate()+scale()+drawPixmap()` 原地变换，消除每帧堆分配
- `_zoom` 动画驱动对象从 `QSize` 改为 `qreal`（kZoomStart=1.03 → kZoomEnd=1.13），`onAnimationRun` 简化为 `value.toReal()/kZoomEnd`，每帧避免 QSize 构造/解包
- 新增透明度动画：`_opacityAni`（4200ms, 0→1(1200ms)→1→0(3000ms)），通过 `setKeyValueAt()` 分段，与缩放放入 `QParallelAnimationGroup`
- crossfade 期间条件关闭 `QPainter::Antialiasing|SmoothPixmapTransform`：`_fadeOutIdx<0` 时全画质，两图重叠时 alpha 混合自然模糊边缘

### 两图轮播架构
- 数据结构 `_pixmaps[2]` / `_scaledMax[2]` / `_zoomScale[2]` / `_zoomAni[2]`
- `_cycleTimer`(3000ms) 触发轮播 + `_crossfadeAnim`(1200ms, 0→1) 控制交叠
- paintEvent 双图绘制，`_crossfadeProgress` 驱动互补透明度
- 重构为仅淡出方案：新图直接 `_opacity=1.0`，旧图 `_fadeOutAni`(1200ms, 1→0)；paintEvent 按新图→旧图顺序，旧图叠在上层淡出

### Bug 修复

#### 共享状态竞态（重构单组动画复用时）
- `_currentScale` 和 `_opacity` 为单变量，crossfade 期间两个 `_zoomAni[i]` 写入同一目标
- 修复：展开为 `_currentScale[2]` / `_opacity[2]`，信号分别连接 `onZoom0/1Changed` / `onOpacity0/1Changed`

#### 白屏（复用单例 `_opacityAni` 后）
- paintEvent 对两张图应用同一个 `_opacity`，淡出时两图一起透明化
- `onOpacityChanged` 缺 `update()`，淡出完成后 `_opacity` 未恢复 1.0，导致 1800ms 白屏
- 修复：新增 `_fadeOutIdx` 追踪淡出槽位，paintEvent 中仅淡出图用 `_opacity`，新图固定 `drawOpacity=1.0`；`onOpacityFinished` 清零并恢复

#### 图片始终在 1 和 2 之间循环（信号槽接口引入后）
- `startZoomAnimation` 中 `if (_pixmaps[备用].isNull()) emit` 守卫——旧图残留致条件恒假
- 修复：移除守卫，无条件 `emit nextImageNeeded()`；`setPixmap` 幂等覆盖旧图

### 接口重构：信号槽解耦图片供应
- `QPixmap _pixmap` → `_pixmaps[2]`，新增信号 `nextImageNeeded()`
- `setPixmap()` 自动选槽位：当前空→填当前，已有图→填备用
- `onFadeOutStart()` 移除硬编码路径加载，仅做槽位切换
- 构造函数末尾 `QTimer::singleShot(0, this, &CenteredImageWidget::nextImageNeeded)` 延迟发射首信号，保证外部 `connect()` 先就位
- `BasicFramework` 中连接信号驱动静态列表 `{1,2,3,4}.webp` 循环

### 技术讨论
- QPainter::RenderHint 枚举：Antialiasing / TextAntialiasing / SmoothPixmapTransform / VerticalSubpixelPositioning / LosslessImageRendering / NonCosmeticBrushPatterns 效果与适用场景
- `QVariantAnimation::setKeyValueAt()` 关键帧实现缓入缓出、多阶段动画、非单调变化
- `QTimer::singleShot(true)+start()` 防抖模式在高频事件中的正确用法
- WASM 性能：Canvas 2D 软件光栅化 vs CSS GPU 合成器本质差距；WebGL 后端对 QPainter 无加速；OpenCV.js 跨 WASM 堆拷贝开销；QPainter 不允许非主线程绘制

### BannerCarouselWidget 类名重命名

- `CenteredImageWidget` → `BannerCarouselWidget`

## 2026-06-11

### FImage 新增 approxGaussianBlur（近似高斯模糊）
- 算法：3 趟 cv.boxFilter 叠加逼近高斯，与 Chromium Skia / CSS blur() 同款方案
- 参数语义与 gaussianBlur 对齐：int radius → kernel = 2r+1
- 等效关系：approxGaussianBlur(r) ≈ gaussianBlur(3.3r)；同核尺寸下 r ≈ floor(0.3×R + 0.5)
- 实现：专用 EM_JS（单次调用三趟 boxFilter，避免多次 C++↔JS 上下文切换）
- 命名演变：canvascssBlur → fastGaussianBlur → approxGaussianBlur

### Canvas/CSS blur 技术调研
- CSS backdrop-filter / filter 模糊在合成阶段自动扩展纹理 → 无边缘暗边
- Canvas ctx.filter blur 边界外采样透明像素 → 边缘变暗，需手动留 padding 解决
- CSS blur() 参数是高斯标准差 σ，非半径；Chromium 底层用 3-box 逼近实现

## 2026-06-08

### ResourceManager 重构
- 字体路径从 widget.cpp 内聚到 ResourceManager::loadFonts()，Widget 改为无参调用
- ResourceManager 改为全局单例：私有构造、static manager()、禁止拷贝/移动
- Widget 移除 ResourceManager 成员指针，改为 ResourceManager::manager() 直调

### 目录与命名
- src/Resource/ → src/ResourceManager/，避免与 resource/ 资源文件目录混淆
- ResourceManager::instance() → manager()，与类名直接对应

### 编码统一
- 全部项目源文件（.cpp/.h/CMakeLists.txt）统一为 UTF-8 without BOM
- CMakeLists.txt 删除 GB2312/UTF-8 冲突导致的乱码注释，重写中文注释

### ResourceManager 实现优化
- 移除 m_fontsLoaded / m_totalFonts 成员变量，改用 std::shared_ptr<int> 局部跟踪，生命周期绑定到单次 loadFonts() 调用

### ResourceManager 接口规划
- Eager 资源（启动时全量加载）：loadFonts() / loadArticles() / loadStyleSheet() / loadConfig() + loadAll()
- Lazy 资源（按需加载）：loadImage(url) / loadImages(urls) → imageReady(url) 信号
- 图片不设应用层缓存，WASM 场景下浏览器 HTTP 缓存自动处理重复请求
- 新增状态查询：isFontsLoaded() / isAllLoaded() 等

### JS 系统主题检测
- addOne-test.js → system-theme.js，实现浏览器深浅主题检测
- 挂载 window.isSystemDark() / getSystemTheme() / onThemeChange() 供 WASM 调用

### Theme 系统移植
- 参考 QWidget-FancyUI 桌面版接口，创建 src/Theme/ 模块（5 个文件）
- Theme.h — 枚举 Theme::Light / Dark
- SystemThemeMonitor — WASM 适配版系统主题监听（matchMedia 事件代替 QThread 轮询注册表）
- ThemeModeController — 主题控制器单例，接口与 fancy::ThemeModeController 一致
- 桌面版 SystemAccentPalette 不支持，省略

### Widget 主题测试界面
- initMainUI() 构建完整测试面板：状态标签 + 四个主题控制按钮
- 手动模式 / 跟随系统模式切换，label 实时刷新，背景色联动

### Bug 修复
- _systemThemeChangedCallback → onSystemThemeChange（Emscripten C 函数导出的前导下划线冲突，导致 Module._xxx 为 undefined 而 JS 回调静默失败）
- SystemThemeMonitor::start() 中 _listening 置位移入 #ifdef __EMSCRIPTEN__ 内
- 增加 console.log 调试输出便于追踪 matchMedia 事件链路

### 移除 Emscripten 冗余宏判断
- 项目始终 WASM 构建，移除 .cpp 实现代码中的 #ifdef __EMSCRIPTEN__ 包覆
- EM_ASM / EM_JS / EMSCRIPTEN_KEEPALIVE 代码直接暴露，不加条件编译
- #include <emscripten.h> 保留宏判断（头文件在非 WASM 环境不存在）
- 删除 SystemTheme() 中的桌面回退 return Theme::Light; 分支

## 2026-06-07

### Django 后端搭建
- 在 run/python/ 部署独立 Python 3.14.5 embeddable
- 安装 pip 并配置清华镜像源（pip.ini 本地化，缓存限制在 run/ 内）
- 创建 Django 6.0.6 后端项目 run/server/，仅保留 index 静态服务

### 启动脚本修复
- start_server.bat 路径硬编码 → %~dp0 自动定位
- 多次修复编码乱码（UTF-8/GB2312 冲突），最终用纯 ASCII
- 绑定地址改为 0.0.0.0:8000，终端输出可点击的 127.0.0.1 链接

### 目录结构整理
- run/js/ 统一存放外部 JS（opencv.js）
- 网页文件归入 server/，再细分到 server/www/
- CMakeLists.txt 部署路径同步更新为 server/www/

### 杂项修复
- 清理 Django 安装残留（~jango 目录）修复 ImportError
- requirements.txt 版本约束放宽至 <7.0
- 屏蔽 Chrome .well-known 探针 404
- 清理所有 __pycache__、pip 缓存、安装包残留