#pragma once

#include <QColor>
#include <QString>

// Shared widget palette. widgets.qss refers to these as @tokens.
namespace WidgetColors
{

    enum class Role
    {
        Surface,
        SurfaceMuted,
        Text,
        TextMuted,
        TextDisabled,
        Border,
        Accent,
        AccentHover
    };

    QColor color(Role role);
    void   setColor(Role role, const QColor &color);

    inline QString hex(const QColor &color)
    {
        return color.name(QColor::HexRgb);
    }

} // namespace WidgetColors
