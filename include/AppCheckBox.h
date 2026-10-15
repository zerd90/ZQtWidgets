#pragma once

#include <QWidget>

#include <memory>

class AppCheckBoxPrivate;

class AppCheckBox final : public QWidget
{
    Q_OBJECT

public:
    explicit AppCheckBox(const QString &text, QWidget *parent = nullptr);
    ~AppCheckBox() override;

    bool isChecked() const;
    void setChecked(bool checked);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void toggled(bool checked);

protected:
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Q_DECLARE_PRIVATE(AppCheckBox)
    std::unique_ptr<AppCheckBoxPrivate> d_ptr;
};
