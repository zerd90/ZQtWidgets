#ifndef _WIDGET_STYLE_H_
#define _WIDGET_STYLE_H_

#include <QObject>
#include <QString>

class WidgetPaletteNotifier : public QObject
{
    Q_OBJECT

public:
    static WidgetPaletteNotifier *instance();
    void                          notify();

signals:
    void changed();
};

// Inserts widgets.qss ahead of the application stylesheet on the first widget construction.
void ensureQtCustomWidgetsStyle();

// Rebuilds the widget stylesheet block after WidgetColors changes.
void refreshQtCustomWidgetsStyle();

// Replaces @color tokens in a widget stylesheet with WidgetColors values.
QString applyWidgetColorTokens(QString styleSheet);

#endif // WIDGETSTYLE_H