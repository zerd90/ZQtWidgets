#include "TimedLineChart.h"
#include "WidgetColors.h"
#include "WidgetStyle.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

#include <cmath>
#include <limits>

class TimedLineChartPrivate
{
public:
    QVector<TimedLineChart::Series> m_series;
    QVector<QVector<qreal>>         m_values;
    int                             m_visibleSampleCount = 60;
};

TimedLineChart::TimedLineChart(QWidget *parent) : QWidget(parent), d_ptr(std::make_unique<TimedLineChartPrivate>())
{
    ensureQtCustomWidgetsStyle();
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setChartSize(QSize(380, 88));
}

TimedLineChart::~TimedLineChart() = default;

void TimedLineChart::setChartSize(const QSize &size)
{
    setFixedSize(size.expandedTo(QSize(1, 1)));
}

void TimedLineChart::setVisibleSampleCount(int count)
{
    Q_D(TimedLineChart);
    d->m_visibleSampleCount = qMax(2, count);
    for (auto &values : d->m_values)
    {
        if (values.size() > d->m_visibleSampleCount)
            values.remove(0, values.size() - d->m_visibleSampleCount);
    }
    update();
}

void TimedLineChart::setSeries(const QVector<Series> &series)
{
    Q_D(TimedLineChart);
    d->m_series = series;
    d->m_values.clear();
    d->m_values.resize(series.size());
    update();
}

void TimedLineChart::setSeriesName(int index, const QString &name)
{
    Q_D(TimedLineChart);
    if (index < 0 || index >= d->m_series.size() || d->m_series[index].name == name)
        return;
    d->m_series[index].name = name;
    update();
}

void TimedLineChart::appendValues(const QVector<qreal> &values)
{
    Q_D(TimedLineChart);
    if (values.size() != d->m_series.size())
        return;

    for (int i = 0; i < values.size(); ++i)
    {
        d->m_values[i].append(values[i]);
        if (d->m_values[i].size() > d->m_visibleSampleCount)
            d->m_values[i].remove(0, d->m_values[i].size() - d->m_visibleSampleCount);
    }
    update();
}

void TimedLineChart::clear()
{
    Q_D(TimedLineChart);
    for (auto &values : d->m_values)
        values.clear();
    update();
}

void TimedLineChart::paintEvent(QPaintEvent *)
{
    Q_D(TimedLineChart);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor frameColor = WidgetColors::color(WidgetColors::Role::TextMuted);
    frameColor.setAlpha(70);
    painter.setPen(QPen(frameColor, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 5, 5);

    qreal minimum  = std::numeric_limits<qreal>::max();
    qreal maximum  = std::numeric_limits<qreal>::lowest();
    bool  hasValue = false;
    for (const auto &seriesValues : d->m_values)
    {
        for (qreal value : seriesValues)
        {
            if (!std::isfinite(value))
                continue;
            minimum  = qMin(minimum, value);
            maximum  = qMax(maximum, value);
            hasValue = true;
        }
    }

    const QFontMetrics metrics(font());
    const int          legendHeight = d->m_series.isEmpty() ? 0 : metrics.height() + 4;
    const int          labelWidth   = metrics.horizontalAdvance(QStringLiteral("000.00")) + 6;
    const QRectF       plotRect(7, legendHeight + 4, qMax(1, width() - labelWidth - 14),
                                qMax(1, height() - legendHeight - metrics.height() - 9));

    qreal lower = 0.0;
    qreal upper = 1.0;
    if (hasValue)
    {
        lower = minimum * 0.95;
        upper = maximum * 1.05;
        if (lower >= upper || qFuzzyCompare(lower, upper))
        {
            const qreal center  = (minimum + maximum) / 2.0;
            const qreal padding = qMax<qreal>(1.0, qAbs(center) * 0.1);
            lower               = center - padding;
            upper               = center + padding;
        }
    }

    QColor gridColor = WidgetColors::color(WidgetColors::Role::TextMuted);
    gridColor.setAlpha(45);
    painter.setPen(QPen(gridColor, 1));
    for (int i = 0; i <= 2; ++i)
    {
        const qreal y = plotRect.top() + plotRect.height() * i / 2.0;
        painter.drawLine(QPointF(plotRect.left(), y), QPointF(plotRect.right(), y));
    }
    painter.setPen(WidgetColors::color(WidgetColors::Role::TextMuted));
    painter.drawText(
        QRectF(plotRect.right() + 5, plotRect.top() - metrics.height() / 2.0, labelWidth - 5, metrics.height()),
        Qt::AlignLeft | Qt::AlignVCenter, QString::number(upper, 'f', 1));
    painter.drawText(
        QRectF(plotRect.right() + 5, plotRect.bottom() - metrics.height() / 2.0, labelWidth - 5, metrics.height()),
        Qt::AlignLeft | Qt::AlignVCenter, QString::number(lower, 'f', 1));

    int legendX = qRound(plotRect.left());
    for (int i = 0; i < d->m_series.size(); ++i)
    {
        painter.setPen(QPen(d->m_series[i].color, 2));
        painter.drawLine(legendX, metrics.height() / 2 + 2, legendX + 12, metrics.height() / 2 + 2);
        legendX += 16;
        painter.setPen(WidgetColors::color(WidgetColors::Role::Text));
        painter.drawText(legendX, metrics.ascent() + 2, d->m_series[i].name);
        legendX += metrics.horizontalAdvance(d->m_series[i].name) + 12;
    }

    if (!hasValue)
    {
        painter.setPen(WidgetColors::color(WidgetColors::Role::TextMuted));
        painter.drawText(plotRect, Qt::AlignCenter, QStringLiteral("No data"));
        return;
    }

    painter.save();
    painter.setClipRect(plotRect);
    for (int seriesIndex = 0; seriesIndex < d->m_values.size(); ++seriesIndex)
    {
        const auto  &seriesValues = d->m_values[seriesIndex];
        QPainterPath path;
        bool         started = false;
        QPointF      lastPoint;
        for (int i = 0; i < seriesValues.size(); ++i)
        {
            const qreal value = seriesValues[i];
            if (!std::isfinite(value))
            {
                started = false;
                continue;
            }
            const int   offset = d->m_visibleSampleCount - seriesValues.size();
            const qreal x =
                plotRect.left()
                + plotRect.width()
                      * (d->m_visibleSampleCount <= 1 ? 0 : (offset + i) / qreal(d->m_visibleSampleCount - 1));
            const qreal y = plotRect.bottom() - plotRect.height() * (value - lower) / (upper - lower);
            lastPoint     = QPointF(x, y);
            if (started)
                path.lineTo(x, y);
            else
            {
                path.moveTo(x, y);
                started = true;
            }
        }
        painter.setPen(QPen(d->m_series[seriesIndex].color, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
        if (started)
        {
            painter.setBrush(d->m_series[seriesIndex].color);
            painter.drawEllipse(lastPoint, 2.5, 2.5);
            painter.setBrush(Qt::NoBrush);
        }
    }
    painter.restore();
}
