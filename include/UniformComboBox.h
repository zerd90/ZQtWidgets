#pragma once

#include <QComboBox>
#include <QPointer>

class QFrame;
class QKeyEvent;
class QListWidget;
class QPaintEvent;
class QVariantAnimation;

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
    void   bindModel(QAbstractItemModel *model);
    void   updateComboGeometry();
    void   ensureOverlayPopup();
    void   applyOverlayPopupStyle();
    void   refreshPaletteDependentStyle();
    void   showOverlayPopup();
    void   hideOverlayPopup();
    void   notifyOverlayPopupHidden();
    void   placeOverlayPopup(int popupWidth, int popupHeight);
    void   applyOverlayReveal(qreal progress);
    void   runOverlayAnimation(bool expand);
    void   finishOverlayHide();
    void   applyOverlayPopupSelection(int comboIndex);
    bool   handleOverlayPopupKey(QKeyEvent *event);
    void   moveOverlayPopupSelection(int delta);
    QPoint overlayComboGlobalPos() const;
    int    overlayPopupRowHeight() const;
    int    overlayPopupFontPx() const;

    // Always use Tool overlay popup (avoids native player surface dismissing Qt::Popup).
    bool                    m_overlayPopupEnabled = true;
    bool                    m_appFilterInstalled  = false;
    int                     m_overlayFontPx       = 14;
    int                     m_overlayRowHeight    = 32;
    int                     m_overlayRadius       = 8;
    int                     m_overlayGap          = 6;
    int                     m_overlayFramePad     = 4;
    QPointer<QFrame>        m_overlayPopup;
    QPointer<QListWidget>   m_overlayList;
    QVariantAnimation      *m_overlayAnimation = nullptr;
    QRect                   m_overlayFullRect;
    bool                    m_overlayOpensAbove                = true;
    bool                    m_overlayOpenNotified              = false;
    bool                    m_suppressOverlayAnimationFinished = false;
    bool                    m_blockOverlayReveal               = false;
    qreal                   m_overlayReveal                    = 0;
    QMetaObject::Connection m_rowsInsertedConnection;
    QMetaObject::Connection m_rowsRemovedConnection;
    QMetaObject::Connection m_modelResetConnection;
    QMetaObject::Connection m_dataChangedConnection;
};
