//
// Created by TX on 2026/6/25.
//

#include "Palette.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMetaEnum>

#include "Core/Theme/ThemeModeController.h"
#include "Core/ResourcePath.h"

// ─────────────────────────────────────────────
// 辅助：JSON key → ColorRole（Qt 元枚举反射）
// ─────────────────────────────────────────────
static ColorRole keyToRole(const QString &key)
{
    const QMetaEnum me = QMetaEnum::fromType<ColorRole>();
    bool ok = false;
    const int val = me.keyToValue(key.toUtf8().constData(), &ok);
    return ok ? static_cast<ColorRole>(val) : ColorRole::Last;
}

// ─────────────────────────────────────────────
// 构造：加载 Default 方案 → 连接主题信号
// ─────────────────────────────────────────────
Palette::Palette(QObject *parent) :
    QObject(parent)
{
    _groupTheme = {ColorGroups::Default, ThemeModeController::controller().appTheme()};
    loadSchemes();

    connect(&ThemeModeController::controller(), &ThemeModeController::appThemeChange, this, &Palette::onAppThemeChanged);
}

Palette &Palette::instance()
{
    static auto *p = new Palette(qApp);
    return *p;
}

// ─────────────────────────────────────────────
// 颜色查询
// ─────────────────────────────────────────────
QColor Palette::color(const ColorRole role) const
{
    return _schemes[_groupTheme][role];
}

QColor Palette::color(const ColorRole role, const Theme theme) const
{
    return _schemes[{_groupTheme.first, theme}][role];
}

QColor Palette::operator[](const ColorRole role) const
{
    return _schemes[_groupTheme][role];
}

// ─────────────────────────────────────────────
// 主题切换（Light ↔ Dark）
// ─────────────────────────────────────────────
void Palette::onAppThemeChanged(const Theme theme)
{
    _groupTheme.second = theme;
    emit appColorChange();
}

// ─────────────────────────────────────────────
// 设置配色组
// ─────────────────────────────────────────────
void Palette::setColorGroups(const ColorGroups group, const int hue)
{
    _groupTheme = {group, ThemeModeController::controller().appTheme()};
    if (group == ColorGroups::Custom)
        generateCustomScheme(hue);
    emit appColorChange();
}

void Palette::setCustomColor(const int hue)
{
    if (_groupTheme.first != ColorGroups::Custom)
        return;
    generateCustomScheme(hue);
    emit appColorChange();
}

// ─────────────────────────────────────────────
// 从 JSON 加载 oklch 参数、固定色，生成 Default 方案
// ─────────────────────────────────────────────
void Palette::loadSchemes()
{
    QFile file(ResourcePath::Local::ColorProfile);
    if (!file.open(QFile::ReadOnly))
    {
        qCritical() << "Palette: Failed to open color profile:" << file.errorString();
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError)
    {
        qCritical() << "Palette: JSON parse error:" << parseError.errorString();
        return;
    }
    if (!doc.isObject())
    {
        qCritical() << "Palette: Color profile root is not an object";
        return;
    }

    const QJsonObject root = doc.object();
    const QJsonObject defaultObj = root["Default"].toObject();
    if (defaultObj.isEmpty())
    {
        qCritical() << "Palette: 'Default' object not found in color profile";
        return;
    }

    // ── 1. 解析 oklchParams ──
    const QJsonObject oklchParams = defaultObj["oklchParams"].toObject();
    for (auto it = oklchParams.begin(); it != oklchParams.end(); ++it)
    {
        const ColorRole role = keyToRole(it.key());
        if (role == ColorRole::Last)
        {
            qWarning() << "Palette: Unknown ColorRole in oklchParams:" << it.key();
            continue;
        }

        const QJsonObject obj = it.value().toObject();
        const QJsonObject lightObj = obj["light"].toObject();
        const QJsonObject darkObj = obj["dark"].toObject();

        OklchRoleParams p;
        p.lightL = lightObj["l"].toDouble();
        p.lightC = lightObj["c"].toDouble();
        p.isFixed = false;

        if (!darkObj.isEmpty())
        {
            // 有深色变体
            p.darkL = darkObj["l"].toDouble();
            p.darkC = darkObj["c"].toDouble();
        }
        else
        {
            // 无深色变体（如 DeepText、TitleActive 在参考项目中只有一个值）
            // → 深色模式沿用浅色的 L/C
            p.darkL = p.lightL;
            p.darkC = p.lightC;
        }

        _oklchParams[role] = p;
    }

    // ── 2. 解析 fixedColors，写 Default hex 并标记 isFixed ──
    const QJsonObject fixedColors = defaultObj["fixedColors"].toObject();
    for (auto it = fixedColors.begin(); it != fixedColors.end(); ++it)
    {
        const ColorRole role = keyToRole(it.key());
        if (role == ColorRole::Last)
        {
            qWarning() << "Palette: Unknown ColorRole in fixedColors:" << it.key();
            continue;
        }

        const QJsonObject obj = it.value().toObject();
        _oklchParams[role] = OklchRoleParams{0, 0, 0, 0, true};

        _schemes[{ColorGroups::Default, Theme::Light}][role] = parseColorFromJsonStr(obj["light"].toString());
        _schemes[{ColorGroups::Default, Theme::Dark}][role] = parseColorFromJsonStr(obj["dark"].toString());
    }

    // ── 3. 生成 Default 方案的参数色（hue = 240°） ──
    for (auto it = _oklchParams.begin(); it != _oklchParams.end(); ++it)
    {
        if (it.value().isFixed)
            continue;

        const ColorRole role = it.key();
        const OklchRoleParams &p = it.value();
        _schemes[{ColorGroups::Default, Theme::Light}][role] =
                oklchToColor(p.lightL, p.lightC, 240.0);
        _schemes[{ColorGroups::Default, Theme::Dark}][role] =
                oklchToColor(p.darkL, p.darkC, 240.0);
    }
}

// ─────────────────────────────────────────────
// oklch → sRGB 转换（与参考网页浏览器内计算一致）
// ─────────────────────────────────────────────
QColor Palette::oklchToColor(const double l, const double c, const double hDeg)
{
    if (c <= 0.0)
        return QColor::fromRgbF(l, l, l); // 灰色：L 直接映射为等值 RGB

    // oklch → oklab
    const double h = qDegreesToRadians(hDeg);
    const double a = c * std::cos(h);
    const double b = c * std::sin(h);

    // oklab → LMS (linear)
    const double lb = l + 0.3963377774 * a + 0.2158037573 * b;
    const double mb = l - 0.1055613458 * a - 0.0638541728 * b;
    const double sb = l - 0.0894841775 * a - 1.2914855480 * b;

    // decode nonlinearity (cube)
    const double l3 = lb * lb * lb;
    const double m3 = mb * mb * mb;
    const double s3 = sb * sb * sb;

    // LMS → linear sRGB
    double r = +4.0767416621 * l3 - 3.3077115913 * m3 + 0.2309699292 * s3;
    double g = -1.2684380046 * l3 + 2.6097574011 * m3 - 0.3413193965 * s3;
    double bl = -0.0041960863 * l3 - 0.7034186147 * m3 + 1.7076147010 * s3;

    // sRGB gamma encode
    auto gamma = [](double v) -> double {
        if (v <= 0.0031308)
            return 12.92 * v;
        return 1.055 * std::pow(v, 1.0 / 2.4) - 0.055;
    };

    r = gamma(std::clamp(r, 0.0, 1.0));
    g = gamma(std::clamp(g, 0.0, 1.0));
    bl = gamma(std::clamp(bl, 0.0, 1.0));

    return QColor::fromRgbF(r, g, bl);
}

// ─────────────────────────────────────────────
// 根据 hue 生成 Custom 方案（oklch 实时计算）
// ─────────────────────────────────────────────
void Palette::generateCustomScheme(const int hue)
{
    for (const Theme theme: {Theme::Light, Theme::Dark})
    {
        ColorsHash &dst = _schemes[{ColorGroups::Custom, theme}];

        for (auto it = _oklchParams.begin(); it != _oklchParams.end(); ++it)
        {
            const ColorRole role = it.key();
            const OklchRoleParams &p = it.value();

            if (p.isFixed)
            {
                // 固定色不随 hue 变化 → 从 Default 方案复制
                dst[role] = _schemes[{ColorGroups::Default, theme}][role];
            }
            else
            {
                const double l = (theme == Theme::Light) ? p.lightL : p.darkL;
                const double c = (theme == Theme::Light) ? p.lightC : p.darkC;
                dst[role] = oklchToColor(l, c, static_cast<double>(hue));
            }
        }
    }
}

// ─────────────────────────────────────────────
// hex 字符串 → QColor
// ─────────────────────────────────────────────
QColor Palette::parseColorFromJsonStr(const QString &raw)
{
    if (raw.startsWith('#'))
    {
        const QString hex = raw.mid(1);
        bool ok = false;
        const int r = hex.mid(0, 2).toInt(&ok, 16);
        const int g = hex.mid(2, 2).toInt(&ok, 16);
        const int b = hex.mid(4, 2).toInt(&ok, 16);

        if (hex.length() == 8)
        {
            const int a = hex.mid(6, 2).toInt(&ok, 16);
            return {r, g, b, a};
        }
        return {r, g, b};
    }

    qWarning() << "Palette: Unrecognized value format:" << raw;
    return {};
}
