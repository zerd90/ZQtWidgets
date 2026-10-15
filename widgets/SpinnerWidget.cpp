#include "WidgetStyle.h"
#include "WidgetColors.h"

#include "SpinnerWidget.h"

#include <QPainter>
#include <QTimer>

class SpinnerWidgetPrivate
{
public:
    QTimer timer;
    int    angle = 0;
};

SpinnerWidget::SpinnerWidget(QWidget *parent) : QWidget(parent), d_ptr(std::make_unique<SpinnerWidgetPrivate>())
{
    Q_D(SpinnerWidget);
    ensureQtCustomWidgetsStyle();
    setFixedSize(QSize(16, 16));
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    hide();

    d->timer.setInterval(80);
    connect(&d->timer, &QTimer::timeout, this,
            [this]()
            {
                Q_D(SpinnerWidget);
                d->angle = (d->angle + 30) % 360;
                update();
            });
}

SpinnerWidget::~SpinnerWidget() = default;

void SpinnerWidget::start()
{
    Q_D(SpinnerWidget);
    if (!d->timer.isActive())
        d->timer.start();
    show();
}

void SpinnerWidget::stop()
{
    Q_D(SpinnerWidget);
    d->timer.stop();
    hide();
}

void SpinnerWidget::paintEvent(QPaintEvent *)
{
    Q_D(SpinnerWidget);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int     w = width();
    const int     h = height();
    const QPointF center(w / 2.0, h / 2.0);

    painter.translate(center);
    painter.rotate(d->angle);

    const int   spokes = 12;
    const qreal inner  = 4.5;
    const qreal outer  = 7.0;
    const QPen  basePen(WidgetColors::color(WidgetColors::Role::Text), 1.6, Qt::SolidLine, Qt::RoundCap);

    for (int i = 0; i < spokes; ++i)
    {
        QColor c = basePen.color();
        c.setAlphaF((i + 1) / static_cast<qreal>(spokes));
        QPen pen = basePen;
        pen.setColor(c);
        painter.setPen(pen);
        painter.drawLine(QPointF(0, -inner), QPointF(0, -outer));
        painter.rotate(360.0 / spokes);
    }
}
