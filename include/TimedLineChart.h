#ifndef TIMEDLINECHART_H
#define TIMEDLINECHART_H

#include <QColor>
#include <QString>
#include <QVector>
#include <QWidget>

#include <memory>

class TimedLineChartPrivate;

class TimedLineChart final : public QWidget
{
public:
    struct Series
    {
        QString name;
        QColor  color;
    };

    explicit TimedLineChart(QWidget *parent = nullptr);
    ~TimedLineChart() override;

    void setChartSize(const QSize &size);
    void setVisibleSampleCount(int count);
    void setSeries(const QVector<Series> &series);
    void setSeriesName(int index, const QString &name);
    void appendValues(const QVector<qreal> &values);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Q_DECLARE_PRIVATE(TimedLineChart)
    std::unique_ptr<TimedLineChartPrivate> d_ptr;
};

#endif // TIMEDLINECHART_H
