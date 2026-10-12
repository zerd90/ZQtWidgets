
#include <QMap>

#include "WidgetColors.h"
#include "WidgetStyle.h"

namespace WidgetColors
{
    namespace
    {
        QMap<Role, QColor> g_colors = {
            {Role::Surface,      QColor(0xFF, 0xFF, 0xFF)},
            {Role::SurfaceMuted, QColor(0xF4, 0xF7, 0xFB)},
            {Role::Text,         QColor(0x1F, 0x2A, 0x37)},
            {Role::TextMuted,    QColor(0x64, 0x74, 0x8B)},
            {Role::TextDisabled, QColor(0x94, 0xA3, 0xB8)},
            {Role::Border,       QColor(0xC9, 0xD3, 0xE3)},
            {Role::Accent,       QColor(0x2A, 0x8D, 0xFF)},
            {Role::AccentHover,  QColor(0x3A, 0x9D, 0xFF)},
        };
    } // namespace

    QColor color(Role role)
    {
        if (g_colors.contains(role))
            return g_colors[role];
        return QColor();
    }

    void setColor(Role role, const QColor &value)
    {
        if (!g_colors.contains(role))
            return;
        QColor &current = g_colors[role];
        if (current == value)
            return;
        current = value;
        refreshQtCustomWidgetsStyle();
    }

} // namespace WidgetColors
