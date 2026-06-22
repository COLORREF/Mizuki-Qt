# Mizuki-Qt

Mizuki-Qt 是一个使用 Qt/C++ 复刻 [LyraVoid/Mizuki](https://github.com/LyraVoid/Mizuki) 外观体验的个人博客项目。项目目标不是移植原项目源码，而是用完全不同的技术栈重新实现一个可在桌面端和 WebAssembly 环境运行的博客前端，并使用 Django 提供轻量后端与静态资源服务。

当前项目仍处于早期开发阶段，已经完成首页基础框架、轮播 Banner、打字机文案、波浪分隔、深浅色主题基础设施、字体/图片资源加载，以及 Django 静态文件和图片 API。

## 与 Mizuki 的关系

本项目仅在视觉和交互方向上参考 Mizuki，没有使用 Mizuki 的源代码。原 Mizuki 是基于 Astro/TypeScript 的静态博客主题，采用 Material Design 3 风格，并提供文章、标签、归档、评论、搜索、深浅色等完整博客能力；Mizuki-Qt 当前则是一个 Qt/C++ + Django 的重新实现版本，功能范围还在逐步补齐。

如果你想了解原项目，请访问：

- 原项目：<https://github.com/LyraVoid/Mizuki>

## 技术栈

- C++17
- Qt 6：Core、Gui、Widgets、Svg、Network等
- CMake 3.16+
- Qt for WebAssembly
- Python + Django

## 当前进度

已完成：

- 首页主体框架与响应式尺寸计算
- 首页 Banner 图片轮播、缩放与淡入淡出
- 打字机文字组件
- 波浪形视觉组件
- 系统深浅色监听与应用主题切换基础
- 字体和首页图片资源的异步加载
- Django 后端入口、静态文件托管、首页图片接口
- Windows 桌面端与 WebAssembly 运行路径的基础适配

计划中：

- 文章列表、文章详情与 Markdown 渲染
- 标签、分类、归档等博客核心页面
- 搜索、评论、友链等扩展功能
- 后端配置拆分与生产部署方案
- 更完整的构建脚本和发布流程

## 目录结构

```text
.
├── CMakeLists.txt
├── resource/                 # Qt 内置资源
├── run/
│   ├── start_server.bat      # Windows 下启动 Django 后端
│   └── server/
│       ├── blog_backend/     # Django 后端
│       ├── requirements.txt
│       └── www/              # 后端服务的前端入口、图片、字体、文章等资源
└── src/
    ├── Core/                 # 资源管理、主题控制等核心模块
    ├── ImageProcessing/      # 图像处理相关代码
    ├── Widgets/              # 自定义 Qt 组件
    └── index/                # 首页基础框架
```

## 本地开发

准备环境：

1. 安装 Qt 6，并确保包含 Widgets、Svg、Network 等模块。
2. 如需构建 WebAssembly 版本，安装 Qt WebAssembly kit 和 Emscripten 环境。
3. 安装 Python，并在 `run/server` 下安装后端依赖。

后端依赖安装示例：

```powershell
cd run/server
python -m venv .venv
.\.venv\Scripts\python -m pip install -r requirements.txt
```

启动后端：

```powershell
cd run
.\start_server.bat
```

或直接运行：

```powershell
cd run/server
python manage.py runserver 0.0.0.0:8000
```

构建 Qt 项目：
```text
WebAssembly：暂时只支持 single-threaded 编译，推荐在Qt Creator 中选择 WASM 构建套件进行编译，其他IDE配置极其繁琐，不做推荐
Windows：跟随个人喜好，在不同 IDE 中选择使用 MSVC（推荐）或 MinGW 编译
```

WebAssembly 构建产物会在构建后自动复制到 `run/server/www`，再由 Django 服务提供访问。当前仓库不会提交 Python 运行时、CMake 构建目录、本地数据库和 WebAssembly 生成产物。

## 开源协议

本项目未使用 Mizuki 源码，仅参考外观并重新实现。

仓库代码采用 MIT License 发布。项目中使用的字体、图片、Qt、Django 等第三方内容遵循其各自许可证。

## 致谢

- [LyraVoid/Mizuki](https://github.com/LyraVoid/Mizuki)：视觉与体验参考来源。
- Qt、Django 以及相关开源生态。
