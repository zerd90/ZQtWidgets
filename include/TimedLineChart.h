#ifndef TIMEDLINECHART_H
#define TIMEDLINECHART_H

#include <QColor>
#include <QString>
#include <QVector>
#include <QWidget>

class TimedLineChart final : public QWidget
{
public:
    struct Series
    {
        QString name;
        QColor  color;
    };

    explicit TimedLineChart(QWidget *parent = nullptr);

    void setChartSize(const QSize &size);
    void setVisibleSampleCount(int count);
    void setSeries(const QVector<Series> &series);
    void setSeriesName(int index, const QString &name);
    void appendValues(const QVector<qreal> &values);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<Series>         m_series;
    QVector<QVector<qreal>> m_values;
    int                     m_visibleSampleCount = 60;
};

#endif // TIMEDLINECHART_H
