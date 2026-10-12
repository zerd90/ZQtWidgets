#ifndef SPINNERWIDGET_H
#define SPINNERWIDGET_H

#include <QPainter>
#include <QTimer>
#include <QWidget>

class SpinnerWidget final : public QWidget
{
public:
    explicit SpinnerWidget(QWidget *parent = nullptr);

    void start();

    void stop();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QTimer m_timer;
    int    m_angle = 0;
};

#endif // SPINNERWIDGET_H
