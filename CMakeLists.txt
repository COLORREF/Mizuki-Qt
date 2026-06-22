cmake_minimum_required(VERSION 3.16)                                    # CMake 版本要求 — 最低 3.16

project(Mizuki-Qt VERSION 0.1 LANGUAGES CXX)                            # 项目名称、版本、语言

set(CMAKE_AUTOUIC ON)                                                   # 自动编译 .ui 文件
set(CMAKE_AUTOMOC ON)                                                   # 自动调用 moc 处理 Q_OBJECT 宏
set(CMAKE_AUTORCC ON)                                                   # 自动编译 .qrc 资源文件

set(CMAKE_CXX_STANDARD 17)                                              # 使用 C++17 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)                                     # 编译器不支持则报错

# ── Qt 依赖（所有平台通用） ───────────────────────────
find_package(Qt6 REQUIRED COMPONENTS                                    # 查找 Qt6 依赖
        Core                                                            # 核心库 (信号槽、事件循环)
        Gui                                                             # 图形界面基础
        Widgets                                                         # 桌面控件 (按钮、文本框等)
        Xml                                                             # XML 读写
        Svg                                                             # SVG 渲染
        SvgWidgets                                                      # SVG 控件集成
        Network
)

# ── 源文件列表 ──────────────────────────────────────
file(GLOB_RECURSE PROJECT_SOURCES
        "${PROJECT_SOURCE_DIR}/main.cpp"
        "${PROJECT_SOURCE_DIR}/widget.h"
        "${PROJECT_SOURCE_DIR}/widget.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.qrc"
        "${PROJECT_SOURCE_DIR}/src/*.ui"
        "${PROJECT_SOURCE_DIR}/resource/*.qrc"
)

qt_add_executable(GithubPage                                            # 构建可执行文件 (Qt6 推荐方式)
        MANUAL_FINALIZATION                                             # 手动控制最终化，稍后统一调用
        ${PROJECT_SOURCES}
)

if(EMSCRIPTEN)
    set_target_properties(GithubPage PROPERTIES
            QT_WASM_INITIAL_MEMORY "80MB"
    )
endif()


target_link_libraries(GithubPage PRIVATE                                # 链接 Qt6 库
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Xml
        Qt6::Svg
        Qt6::SvgWidgets
        Qt6::Network
)

set(PROJECT_SRC_INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/src
)
include_directories(${PROJECT_SRC_INCLUDE_DIRS})

include(GNUInstallDirs)                                                 # 引入标准安装目录变量

qt_finalize_executable(GithubPage)                                      # Qt6 最终化 (MOC/UIC/RCC)，放在所有链接之后

# 构建后复制产物到 run/server/www（仅 WASM）
if(EMSCRIPTEN)
    add_custom_command(TARGET GithubPage POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "${PROJECT_SOURCE_DIR}/run/server/www"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/GithubPage.js"
            "${CMAKE_BINARY_DIR}/GithubPage.wasm"
            "${CMAKE_BINARY_DIR}/qtloader.js"
            "${PROJECT_SOURCE_DIR}/run/server/www/"
            COMMENT "Copying WASM artifacts to server directory"
    )
endif()