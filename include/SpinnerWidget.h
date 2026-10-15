#ifndef SPINNERWIDGET_H
#define SPINNERWIDGET_H

#include <QWidget>

#include <memory>

class SpinnerWidgetPrivate;

class SpinnerWidget final : public QWidget
{
public:
    explicit SpinnerWidget(QWidget *parent = nullptr);
    ~SpinnerWidget() override;

    void start();
    void stop();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    Q_DECLARE_PRIVATE(SpinnerWidget)
    std::unique_ptr<SpinnerWidgetPrivate> d_ptr;
};

#endif // SPINNERWIDGET_H
