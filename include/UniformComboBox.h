#pragma once

#include <QComboBox>

#include <memory>

class QKeyEvent;
class QPaintEvent;
class QWheelEvent;
class UniformComboBoxPrivate;

class UniformComboBox final : public QComboBox
{
    Q_OBJECT

public:
    explicit UniformComboBox(QWidget *parent = nullptr);
    ~UniformComboBox() override;

    void setModel(QAbstractItemModel *model) override;
    void setItemEnabled(int index, bool enabled);
    void setOverlayPopupEnabled(bool enabled);
    void setOverlayPopupMetrics(int fontPx, int rowHeight, int radius, int gapAboveBar, int framePadding);
    bool isOverlayPopupVisible() const;
    bool containsOverlayPopup(const QPoint &globalPos) const;
    void closeOverlayPopup();

signals:
    void popupVisibilityChanged(bool visible);

protected:
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void showPopup() override;
    void hidePopup() override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Q_DECLARE_PRIVATE(UniformComboBox)
    std::unique_ptr<UniformComboBoxPrivate> d_ptr;
};
