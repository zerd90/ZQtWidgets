#ifndef TOGGLESWITCH_H
#define TOGGLESWITCH_H

#include <QWidget>
#include <QToolButton>
#include <QPainter>
#include <QResizeEvent>
#include <QPaintEvent>

class ToggleSwitch : public QWidget
{
    Q_OBJECT

public:
    explicit ToggleSwitch(QWidget *parent = nullptr);

    bool isChecked() const;
    void setChecked(bool checked);

    QAbstractButton *button() const;

signals:
    void toggled(bool checked);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QToolButton *m_button = nullptr;
};

#endif // TOGGLESWITCH_H
