#ifndef TOGGLESWITCH_H
#define TOGGLESWITCH_H

#include <QWidget>

#include <memory>

class QAbstractButton;
class ToggleSwitchPrivate;

class ToggleSwitch : public QWidget
{
    Q_OBJECT

public:
    explicit ToggleSwitch(QWidget *parent = nullptr);
    ~ToggleSwitch() override;

    bool             isChecked() const;
    void             setChecked(bool checked);
    QAbstractButton *button() const;

signals:
    void toggled(bool checked);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Q_DECLARE_PRIVATE(ToggleSwitch)
    std::unique_ptr<ToggleSwitchPrivate> d_ptr;
};

#endif // TOGGLESWITCH_H
