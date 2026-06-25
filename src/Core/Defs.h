//
// Created by TX on 2026/6/25.
//

#ifndef MIZUKI_QT_DEFS_H
#define MIZUKI_QT_DEFS_H

enum class Theme
{
    Light,
    Dark
};

enum class ColorGroups
{
    System, // 跟随系统的配色方案，默认情况下将使用此方案，仅用于部分突出强调颜色（如按钮背景色等），其余颜色由硬编码指定且三者均相同（如通用边框色等）
    Default, // 默认情况下的配色方案，当系统不支持查询主题色时将使用此方案，配色方案由本库的开发者@COLORREF提供
    Custom // 自定义的配色方案，由用户或开发者手动指定
};

enum class SysAccentPalette
{
    // 此处枚举为系统主题色及其衍生色，命名采用与WinRT中的命名方式一致（顺便吐槽一下这个构式命名）
    // 值从小到大的顺序对应颜色由浅到深
    // ！不要修改顺序，sysAccentColors 实现函数依赖此顺序
    AccentLight3, // 最浅的主题色  场景：1、深色模式下鼠标悬浮在控件（如右下角面板打开后的WiFi按钮）上时使用的颜色 2、深色模式下系统超链接标签文字颜色
    AccentLight2, // 较浅的主题色  场景：1、深色模式下系统控件的默认颜色（如系统设置界面侧边栏指示器的颜色、单选框内部背景色）2、浅色模式下系统超链接标签文字颜色
    AccentLight1, // 浅色主题色
    Accent, // 个性化设置中实际设置显示的颜色
    AccentDark1, // 深色主题色   场景：1、浅色模式下系统控件的默认颜色
    AccentDark2, // 较深的主题色  场景：1、系统底部任务栏颜色（需要开启深色模式以及“在开始和任务栏显示重点颜色”）
    // 2、浅色模式下鼠标悬浮在控件（如右下角面板打开后的WiFi按钮）上时使用的颜色
    // 3、浅色模式下用于突出重要内容的超链接标签颜色（如任务管理器设置中相关超链接标签）
    AccentDark3, // 最深的主题色  场景：深色模式下系统列表控件选中时的颜色（如任务管理器中内存或cpu消耗占比较高的单元格颜色与之非常接近）
};

enum class ColorRole
{
    Window, // 窗口背景色
    WindowText, // 窗口标题等窗口信息文本色
    WindowBright, // 窗口高度对比色（关闭按钮悬浮时基础色）
    Border, // 通用边框色
    Text, // 通用文本色
    Button, // 按钮背景色
    ButtonBorder, // 按钮边框色
    ButtonBottomLine, // 按钮底线色
    RadioButtonInnerCircle, // 单选按钮内圈颜色
    Link, // 超链接色
    ImportantLink, // 重要的超链接色
    LinkVisited, // 已访问超链接色
    DisEnabled, // 通用禁用色
    DisEnabledText, //禁用文本色
    AppAccent, // 当前应用使用的强调色
    SysAccent, // 系统个性化设置中的强调色
    AppAccentText, // 以AppAccent为背景色的合适的文本
    MaskNormal, // 默认透明遮罩色
    MaskHover, // 悬浮透明遮罩色
    MaskPressed, // 按下透明遮罩色
    MaskBrightPressed, // 高度对比按下透明遮罩色（关闭按钮）
    MaskSelected, // 选中透明遮罩色
    MaskSelectedHover, // 选中悬浮遮罩色
    MaskSelectedPressed, // 选中按下遮罩色
    MSWindow, // MS窗口主区域背景色
    MSWindowSplitLine, // MS窗口分割线颜色
    ScrollBarNormal, // 滚动条普通背景色
    ScrollBarFocus, // 滚动条焦点背景色
    ScrollBarEndArrow, // 滚动条末端箭头颜色
    ScrollBarSlider, // 滚动条滑块颜色
    TextEditPanelNormal, // 文本编辑框面板普通色
    TextEditPanelFocus, // 文本编辑框面板焦点色
    TextEditPanelHover, //  文本编辑框面板悬浮色
    TextEditBorder, // 文本编辑框边框色
    TextEditIndicatorLine, // 文本编辑框指示线通用色
    PlaceholderText, // 占位符文本通用色
    Last
};

#endif //MIZUKI_QT_DEFS_H
