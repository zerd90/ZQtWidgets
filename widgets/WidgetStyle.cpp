#include "WidgetStyle.h"

#include "WidgetColors.h"

#include <QApplication>
#include <QFile>

void initQtCustomWidgetsResource()
{
    Q_INIT_RESOURCE(qt_custom_widgets);
}

namespace
{
    const char kLoadedProperty[] = "_qtCustomWidgetsStyleLoaded";
    const char kStyleBegin[]     = "/* qt-custom-widgets-style-begin */";
    const char kStyleEnd[]       = "/* qt-custom-widgets-style-end */";

    QString &widgetStyleTemplate()
    {
        static QString templ;
        return templ;
    }

    QString widgetStyleBlock()
    {
        initQtCustomWidgetsResource();

        QString &templ = widgetStyleTemplate();
        if (templ.isNull())
        {
            QFile file(QStringLiteral(":/qt-custom-widgets/widgets.qss"));
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                return {};
            templ = QString::fromUtf8(file.readAll());
        }

        return QString::fromLatin1(kStyleBegin) + QLatin1Char('\n') + applyWidgetColorTokens(templ) + QLatin1Char('\n')
             + QString::fromLatin1(kStyleEnd) + QLatin1Char('\n');
    }

    void installWidgetStyle(bool notify)
    {
        QApplication *app = qApp;
        if (!app)
            return;

        const QString block = widgetStyleBlock();
        if (block.isEmpty())
            return;

        QString   current = app->styleSheet();
        const int begin   = current.indexOf(QLatin1String(kStyleBegin));
        const int end     = current.indexOf(QLatin1String(kStyleEnd));
        if (begin >= 0 && end > begin)
        {
            const int tail = end + static_cast<int>(qstrlen(kStyleEnd));
            current.replace(begin, tail - begin, block);
        }
        else
        {
            current = block + current;
        }

        app->setStyleSheet(current);
        app->setProperty(kLoadedProperty, true);
        if (notify)
            WidgetPaletteNotifier::instance()->notify();
    }
} // namespace

WidgetPaletteNotifier *WidgetPaletteNotifier::instance()
{
    static WidgetPaletteNotifier notifier;
    return &notifier;
}

void WidgetPaletteNotifier::notify()
{
    emit changed();
}

QString applyWidgetColorTokens(QString styleSheet)
{
    const struct
    {
        const char *token;
        QColor      color;
    } tokens[] = {
        {"@surfaceMuted", WidgetColors::color(WidgetColors::Role::SurfaceMuted)},
        {"@textDisabled", WidgetColors::color(WidgetColors::Role::TextDisabled)},
        {"@accentHover",  WidgetColors::color(WidgetColors::Role::AccentHover) },
        {"@textMuted",    WidgetColors::color(WidgetColors::Role::TextMuted)   },
        {"@surface",      WidgetColors::color(WidgetColors::Role::Surface)     },
        {"@border",       WidgetColors::color(WidgetColors::Role::Border)      },
        {"@accent",       WidgetColors::color(WidgetColors::Role::Accent)      },
        {"@text",         WidgetColors::color(WidgetColors::Role::Text)        },
    };

    for (const auto &token : tokens)
        styleSheet.replace(QLatin1String(token.token), WidgetColors::hex(token.color));
    return styleSheet;
}

void ensureQtCustomWidgetsStyle()
{
    if (qApp && qApp->property(kLoadedProperty).toBool())
        return;
    installWidgetStyle(false);
}

void refreshQtCustomWidgetsStyle()
{
    installWidgetStyle(true);
}
