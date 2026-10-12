#include "UniformComboBox.h"
#include "WidgetColors.h"
#include "WidgetStyle.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QColor>
#include <QEasingCurve>
#include <QEvent>
#include <QFrame>
#include <QKeyEvent>
#include <QListWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QStandardItemModel>
#include <QStyle>
#include <QStyleOptionComboBox>
#include <QStyledItemDelegate>
#include <QVariantAnimation>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWindow>

namespace
{
    class UniformComboBoxItemDelegate final : public QStyledItemDelegate
    {
    public:
        explicit UniformComboBoxItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
        {
            QSize size = QStyledItemDelegate::sizeHint(option, index);
            size.setHeight(qMax(size.height(), 32));
            return size;
        }

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
        {
            QStyleOptionViewItem itemOption(option);
            initStyleOption(&itemOption, index);

            if (!index.flags().testFlag(Qt::ItemIsEnabled))
            {
                const QColor disabledText = WidgetColors::color(WidgetColors::Role::TextDisabled);
                for (QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled})
                {
                    itemOption.palette.setColor(group, QPalette::Text, disabledText);
                    itemOption.palette.setColor(group, QPalette::HighlightedText, disabledText);
                }
            }

            QStyledItemDelegate::paint(painter, itemOption, index);
        }
    };
} // namespace

UniformComboBox::UniformComboBox(QWidget *parent) : QComboBox(parent)
{
    ensureQtCustomWidgetsStyle();
    connect(WidgetPaletteNotifier::instance(), &WidgetPaletteNotifier::changed, this,
            [this]() { refreshPaletteDependentStyle(); });
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setMinimumContentsLength(0);
    setItemDelegate(new UniformComboBoxItemDelegate(this));
    bindModel(model());
    updateComboGeometry();
}

UniformComboBox::~UniformComboBox()
{
    m_suppressOverlayAnimationFinished = true;
    if (m_overlayAnimation)
        m_overlayAnimation->stop();
    if (m_appFilterInstalled && qApp)
    {
        qApp->removeEventFilter(this);
        m_appFilterInstalled = false;
    }
    if (m_overlayPopup)
    {
        m_overlayPopup->removeEventFilter(this);
        if (m_overlayList)
            m_overlayList->removeEventFilter(this);
        delete m_overlayPopup;
    }
}

void UniformComboBox::setModel(QAbstractItemModel *model)
{
    if (model == this->model())
    {
        return;
    }

    bindModel(nullptr);
    QComboBox::setModel(model);
    bindModel(model);
    updateComboGeometry();
}

void UniformComboBox::setItemEnabled(int index, bool enabled)
{
    auto          *standardModel = qobject_cast<QStandardItemModel *>(model());
    QStandardItem *item          = standardModel ? standardModel->item(index) : nullptr;
    if (!item)
    {
        return;
    }

    item->setEnabled(enabled);
}

void UniformComboBox::setOverlayPopupEnabled(bool enabled)
{
    m_overlayPopupEnabled = enabled;
    if (!enabled)
        hideOverlayPopup();
}

void UniformComboBox::setOverlayPopupMetrics(int fontPx, int rowHeight, int radius, int gapAboveBar, int framePadding)
{
    // Popup list metrics only — do not touch the combo's own font/chrome (app.qss owns that).
    m_overlayFontPx    = qMax(10, fontPx);
    m_overlayRowHeight = qMax(16, rowHeight);
    m_overlayRadius    = qMax(4, radius);
    m_overlayGap       = qMax(2, gapAboveBar);
    m_overlayFramePad  = qMax(2, framePadding);

    if (m_overlayPopup)
        applyOverlayPopupStyle();
}

bool UniformComboBox::isOverlayPopupVisible() const
{
    return m_overlayPopup && m_overlayPopup->isVisible();
}

bool UniformComboBox::containsOverlayPopup(const QPoint &globalPos) const
{
    return m_overlayPopup && m_overlayPopup->isVisible() && m_overlayPopup->frameGeometry().contains(globalPos);
}

void UniformComboBox::changeEvent(QEvent *event)
{
    QComboBox::changeEvent(event);

    if (!event)
    {
        return;
    }

    switch (event->type())
    {
        case QEvent::FontChange:
        case QEvent::StyleChange:
        case QEvent::PaletteChange:
        case QEvent::LayoutDirectionChange:
            updateComboGeometry();
            break;
        default:
            break;
    }
}

void UniformComboBox::paintEvent(QPaintEvent *event)
{
    QComboBox::paintEvent(event);

    QStyleOptionComboBox option;
    initStyleOption(&option);
    const QRect arrowRect = style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxArrow, this);
    if (!arrowRect.isValid())
        return;

    const qreal   arrowWidth  = qBound(8, qRound(height() * 0.38), 12);
    const qreal   arrowHeight = arrowWidth * 0.62;
    const QPointF center      = arrowRect.center();
    QPainterPath  caret;
    caret.moveTo(center.x() - arrowWidth / 2.0, center.y() - arrowHeight / 2.0);
    caret.lineTo(center.x() + arrowWidth / 2.0, center.y() - arrowHeight / 2.0);
    caret.lineTo(center.x(), center.y() + arrowHeight / 2.0);
    caret.closeSubpath();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(!isEnabled()   ? WidgetColors::color(WidgetColors::Role::TextDisabled)
                     : underMouse() ? WidgetColors::color(WidgetColors::Role::Text)
                                    : WidgetColors::color(WidgetColors::Role::TextMuted));
    painter.drawPath(caret);
}

void UniformComboBox::showPopup()
{
    if (m_overlayPopupEnabled)
    {
        if (isOverlayPopupVisible())
            hideOverlayPopup();
        else
            showOverlayPopup();
        return;
    }

    if (view())
    {
        const int visibleRows = qMin(count(), maxVisibleItems());
        view()->setMinimumHeight(visibleRows * 32 + 2 * view()->frameWidth());
    }
    QComboBox::showPopup();
    emit popupVisibilityChanged(true);
}

void UniformComboBox::hidePopup()
{
    if (m_overlayPopupEnabled)
    {
        hideOverlayPopup();
        return;
    }
    QComboBox::hidePopup();
    emit popupVisibilityChanged(false);
}

void UniformComboBox::closeOverlayPopup()
{
    hideOverlayPopup();
}

void UniformComboBox::keyPressEvent(QKeyEvent *event)
{
    if (m_overlayPopupEnabled)
    {
        if (isOverlayPopupVisible() && handleOverlayPopupKey(event))
            return;
        if (!isOverlayPopupVisible() && event
            && (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up || event->key() == Qt::Key_Space
                || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_F4))
        {
            showOverlayPopup();
            return;
        }
    }
    QComboBox::keyPressEvent(event);
}

bool UniformComboBox::eventFilter(QObject *watched, QEvent *event)
{
    if (!event)
        return QComboBox::eventFilter(watched, event);

    // Tool popup does not auto-dismiss like Qt::Popup; close on outside press.
    if (m_overlayPopupEnabled && isOverlayPopupVisible() && event->type() == QEvent::MouseButtonPress)
    {
        const auto *mouse = static_cast<QMouseEvent *>(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const QPoint globalPos = mouse->globalPosition().toPoint();
#else
        const QPoint globalPos = mouse->globalPos();
#endif
        const QRect comboGlobal(mapToGlobal(QPoint(0, 0)), size());
        if (!containsOverlayPopup(globalPos) && !comboGlobal.contains(globalPos))
            hideOverlayPopup();
    }

    if (watched == m_overlayPopup || watched == m_overlayList)
    {
        if (event->type() == QEvent::KeyPress && handleOverlayPopupKey(static_cast<QKeyEvent *>(event)))
            return true;
        if (watched == m_overlayPopup && event->type() == QEvent::Paint)
        {
            // WA_TranslucentBackground ignores stylesheet fills on macOS; paint manually.
            QPainter painter(m_overlayPopup);
            painter.setRenderHint(QPainter::Antialiasing, true);
            const QRectF bounds = QRectF(m_overlayPopup->rect()).adjusted(0.5, 0.5, -0.5, -0.5);
            QPainterPath path;
            path.addRoundedRect(bounds, m_overlayRadius, m_overlayRadius);
            painter.fillPath(path, WidgetColors::color(WidgetColors::Role::Surface));
            painter.setPen(QPen(WidgetColors::color(WidgetColors::Role::Border), 1.0));
            painter.drawPath(path);
            return false;
        }
        if (watched == m_overlayPopup && (event->type() == QEvent::Hide || event->type() == QEvent::Close))
            notifyOverlayPopupHidden();
    }
    return QComboBox::eventFilter(watched, event);
}

int UniformComboBox::overlayPopupFontPx() const
{
    return m_overlayFontPx;
}

int UniformComboBox::overlayPopupRowHeight() const
{
    return m_overlayRowHeight;
}

QPoint UniformComboBox::overlayComboGlobalPos() const
{
    QWidget *barWindow = window();
    if (barWindow)
    {
        if (QWindow *handle = barWindow->windowHandle())
            return handle->mapToGlobal(mapTo(barWindow, QPoint(0, 0)));
    }
    return mapToGlobal(QPoint(0, 0));
}

void UniformComboBox::placeOverlayPopup(int popupWidth, int popupHeight)
{
    if (!m_overlayPopup)
        return;

    // Anchor to the combo itself (works for toolbar overlay fish combos and
    // normal window combos like stream/speed). Prefer above; fall back below.
    const QPoint comboTopLeft = overlayComboGlobalPos();
    const int    comboBottom  = comboTopLeft.y() + height();
    int          popupX       = comboTopLeft.x();
    int          popupY       = comboTopLeft.y() - popupHeight - m_overlayGap;

    if (QScreen *screen = this->screen())
    {
        const QRect avail    = screen->availableGeometry();
        popupX               = qBound(avail.left(), popupX, avail.right() - popupWidth + 1);
        const int  aboveY    = comboTopLeft.y() - popupHeight - m_overlayGap;
        const int  belowY    = comboBottom + m_overlayGap;
        const bool fitsAbove = aboveY >= avail.top();
        const bool fitsBelow = belowY + popupHeight - 1 <= avail.bottom();
        if (fitsAbove)
            popupY = aboveY;
        else if (fitsBelow)
            popupY = belowY;
        else
            popupY = avail.top();
    }

    m_overlayFullRect   = QRect(popupX, popupY, popupWidth, popupHeight);
    m_overlayOpensAbove = popupY + popupHeight / 2 < comboTopLeft.y() + height() / 2;
}

void UniformComboBox::applyOverlayPopupStyle()
{
    if (!m_overlayPopup)
        return;
    // Match app.qss UniformComboBox QAbstractItemView look for every combo.
    m_overlayPopup->setStyleSheet(
        applyWidgetColorTokens(QStringLiteral("QFrame#uniformComboPopup {"
                                              " background:transparent;"
                                              " border:none;"
                                              "}"
                                              "QListWidget#uniformComboPopupList {"
                                              " background:transparent;"
                                              " border:none;"
                                              " outline:0;"
                                              " color:@text;"
                                              " padding:0;"
                                              " font-size:%3px;"
                                              "}"
                                              "QListWidget#uniformComboPopupList::item {"
                                              " min-height:%1px;"
                                              " padding:0 10px;"
                                              " border-radius:%2px;"
                                              " margin:1px 0;"
                                              "}"
                                              "QListWidget#uniformComboPopupList::item:hover {"
                                              " background:@surfaceMuted;"
                                              "}"
                                              "QListWidget#uniformComboPopupList::item:selected {"
                                              " color:@surface;"
                                              " background:@accent;"
                                              "}"
                                              "QListWidget#uniformComboPopupList::item:disabled,"
                                              "QListWidget#uniformComboPopupList::item:disabled:hover,"
                                              "QListWidget#uniformComboPopupList::item:disabled:selected {"
                                              " color:@textDisabled;"
                                              " background:transparent;"
                                              "}"))
            .arg(overlayPopupRowHeight())
            .arg(qMax(4, m_overlayRadius - 2))
            .arg(overlayPopupFontPx()));
    if (m_overlayList)
    {
        QFont listFont = m_overlayList->font();
        listFont.setPixelSize(overlayPopupFontPx());
        m_overlayList->setFont(listFont);
    }
    if (auto *layout = m_overlayPopup->layout())
        layout->setContentsMargins(m_overlayFramePad, m_overlayFramePad, m_overlayFramePad, m_overlayFramePad);
}

void UniformComboBox::refreshPaletteDependentStyle()
{
    update();
    if (!m_overlayList)
        return;

    applyOverlayPopupStyle();
    for (int i = 0; i < m_overlayList->count(); ++i)
    {
        QListWidgetItem *item = m_overlayList->item(i);
        if (item && !item->flags().testFlag(Qt::ItemIsEnabled))
            item->setForeground(WidgetColors::color(WidgetColors::Role::TextDisabled));
    }
}

void UniformComboBox::ensureOverlayPopup()
{
    if (m_overlayPopup)
        return;

    // Prefer Tool+StaysOnTop over Qt::Popup: native Metal/OpenGL player surfaces
    // sit above normal popups and steal hover, which instantly dismisses Qt::Popup.
    m_overlayPopup =
        new QFrame(nullptr, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint);
    m_overlayPopup->setObjectName(QStringLiteral("uniformComboPopup"));
    m_overlayPopup->setAttribute(Qt::WA_TranslucentBackground, true);
    m_overlayPopup->setAttribute(Qt::WA_NoSystemBackground, true);
    m_overlayPopup->setAttribute(Qt::WA_ShowWithoutActivating, true);
    m_overlayPopup->setAutoFillBackground(false);
    m_overlayPopup->setFrameShape(QFrame::NoFrame);

    auto *layout = new QVBoxLayout(m_overlayPopup);
    layout->setSpacing(0);

    m_overlayList = new QListWidget(m_overlayPopup);
    m_overlayList->setObjectName(QStringLiteral("uniformComboPopupList"));
    m_overlayList->setFocusPolicy(Qt::StrongFocus);
    m_overlayList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_overlayList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_overlayList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_overlayList->setUniformItemSizes(true);
    m_overlayList->setMouseTracking(true);
    layout->addWidget(m_overlayList);
    applyOverlayPopupStyle();

    connect(m_overlayList, &QListWidget::itemClicked, this,
            [this](QListWidgetItem *item)
            {
                if (!item)
                    return;
                applyOverlayPopupSelection(item->data(Qt::UserRole).toInt());
            });

    m_overlayPopup->installEventFilter(this);
    m_overlayList->installEventFilter(this);
}

void UniformComboBox::showOverlayPopup()
{
    ensureOverlayPopup();
    if (!m_overlayPopup || !m_overlayList)
        return;

    m_overlayList->clear();
    QFontMetrics metrics(font());
    int          textWidth = 0;
    const int    rowHeight = overlayPopupRowHeight();
    for (int i = 0; i < count(); ++i)
    {
        auto *item = new QListWidgetItem(itemText(i), m_overlayList);
        item->setData(Qt::UserRole, i);
        const bool enabled = model() && model()->flags(model()->index(i, modelColumn())).testFlag(Qt::ItemIsEnabled);
        item->setFlags(enabled ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable) : Qt::NoItemFlags);
        if (!enabled)
            item->setForeground(WidgetColors::color(WidgetColors::Role::TextDisabled));
        item->setSizeHint(QSize(0, rowHeight));
        textWidth = qMax(textWidth, metrics.horizontalAdvance(itemText(i)));
        if (enabled && i == currentIndex())
            item->setSelected(true);
    }

    const int rows        = qMax(count(), 1);
    const int listHeight  = rows * rowHeight + qMax(0, rows - 1) * 2;
    const int framePad    = m_overlayFramePad * 2;
    const int popupWidth  = qMax(width(), textWidth + 36);
    const int popupHeight = listHeight + framePad;

    m_overlayList->setFixedSize(popupWidth - framePad, listHeight);
    if (auto *layout = m_overlayPopup->layout())
        layout->setEnabled(false);

    placeOverlayPopup(popupWidth, popupHeight);
    applyOverlayReveal(0);
    m_overlayPopup->show();
    placeOverlayPopup(popupWidth, popupHeight);
    applyOverlayReveal(0);
    m_overlayPopup->raise();
    m_overlayList->setFocus(Qt::PopupFocusReason);
    if (qApp && !m_appFilterInstalled)
    {
        qApp->installEventFilter(this);
        m_appFilterInstalled = true;
    }
    if (!m_overlayOpenNotified)
    {
        m_overlayOpenNotified = true;
        emit popupVisibilityChanged(true);
    }
    runOverlayAnimation(true);
}

void UniformComboBox::applyOverlayReveal(qreal progress)
{
    if (!m_overlayPopup)
        return;

    m_overlayReveal    = qBound(0.0, progress, 1.0);
    const int fullW    = m_overlayFullRect.width();
    const int fullH    = qMax(1, m_overlayFullRect.height());
    const int visibleH = qRound(fullH * m_overlayReveal);
    // A 1px translucent tool window is repainted opaque by macOS.
    if (visibleH < 4)
    {
        m_overlayPopup->setWindowOpacity(0);
        return;
    }

    const int y = m_overlayOpensAbove ? (m_overlayFullRect.bottom() + 1 - visibleH) : m_overlayFullRect.top();
    m_overlayPopup->setGeometry(m_overlayFullRect.x(), y, fullW, visibleH);
    m_overlayPopup->setWindowOpacity(m_overlayReveal);

    if (!m_overlayList)
        return;
    const int listY =
        m_overlayOpensAbove ? (visibleH - m_overlayFramePad - m_overlayList->height()) : m_overlayFramePad;
    m_overlayList->move(m_overlayFramePad, listY);
}

void UniformComboBox::runOverlayAnimation(bool expand)
{
    if (!m_overlayPopup)
        return;
    if (!m_overlayAnimation)
    {
        m_overlayAnimation = new QVariantAnimation(this);
        m_overlayAnimation->setDuration(170);
        connect(m_overlayAnimation, &QVariantAnimation::valueChanged, this,
                [this](const QVariant &value)
                {
                    if (m_blockOverlayReveal)
                        return;
                    applyOverlayReveal(value.toReal());
                });
        connect(m_overlayAnimation, &QVariantAnimation::finished, this,
                [this]()
                {
                    if (m_suppressOverlayAnimationFinished || m_blockOverlayReveal || !m_overlayPopup
                        || !m_overlayPopup->isVisible())
                        return;
                    if (m_overlayAnimation->endValue().toReal() < 0.5)
                        finishOverlayHide();
                });
    }

    if (m_overlayAnimation->state() == QAbstractAnimation::Running
        && (m_overlayAnimation->endValue().toReal() > 0.5) == expand)
        return;

    const qreal from                   = m_overlayReveal;
    m_suppressOverlayAnimationFinished = true;
    m_blockOverlayReveal               = true;
    m_overlayAnimation->stop();
    // A finished animation stays at the end time. Changing the end value there would
    // interpolate to 0 immediately and skip the collapse.
    m_overlayAnimation->setCurrentTime(0);
    m_overlayAnimation->setEasingCurve(expand ? QEasingCurve::OutCubic : QEasingCurve::InCubic);
    m_overlayAnimation->setStartValue(from);
    m_overlayAnimation->setEndValue(expand ? 1.0 : 0.0);
    m_blockOverlayReveal               = false;
    m_suppressOverlayAnimationFinished = false;
    m_overlayAnimation->start();
}

void UniformComboBox::finishOverlayHide()
{
    m_overlayReveal       = 0;
    m_overlayOpenNotified = false;
    if (m_appFilterInstalled && qApp)
    {
        qApp->removeEventFilter(this);
        m_appFilterInstalled = false;
    }
    if (!m_overlayPopup || !m_overlayPopup->isVisible())
        return;
    m_overlayPopup->setWindowOpacity(0);
    m_overlayPopup->hide();
}

void UniformComboBox::hideOverlayPopup()
{
    if (!m_overlayPopup || (!m_overlayPopup->isVisible() && m_overlayReveal <= 0))
        return;
    if (m_overlayReveal <= 0.001)
    {
        finishOverlayHide();
        return;
    }
    runOverlayAnimation(false);
}

void UniformComboBox::notifyOverlayPopupHidden()
{
    emit popupVisibilityChanged(false);
}

void UniformComboBox::applyOverlayPopupSelection(int comboIndex)
{
    if (comboIndex >= 0 && comboIndex < count())
        setCurrentIndex(comboIndex);
    hideOverlayPopup();
}

void UniformComboBox::moveOverlayPopupSelection(int delta)
{
    if (!m_overlayList || m_overlayList->count() == 0)
        return;

    int row = m_overlayList->currentRow();
    if (row < 0)
        row = 0;
    for (int step = 0; step < m_overlayList->count(); ++step)
    {
        row                   = (row + delta + m_overlayList->count()) % m_overlayList->count();
        QListWidgetItem *item = m_overlayList->item(row);
        if (item && item->flags().testFlag(Qt::ItemIsEnabled))
        {
            m_overlayList->setCurrentRow(row);
            return;
        }
    }
}

bool UniformComboBox::handleOverlayPopupKey(QKeyEvent *event)
{
    if (!event || !isOverlayPopupVisible())
        return false;

    switch (event->key())
    {
        case Qt::Key_Escape:
            hideOverlayPopup();
            return true;
        case Qt::Key_Down:
            moveOverlayPopupSelection(1);
            return true;
        case Qt::Key_Up:
            moveOverlayPopupSelection(-1);
            return true;
        case Qt::Key_Home:
            if (m_overlayList)
                m_overlayList->setCurrentRow(0);
            return true;
        case Qt::Key_End:
            if (m_overlayList)
                m_overlayList->setCurrentRow(m_overlayList->count() - 1);
            return true;
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Space:
            if (m_overlayList && m_overlayList->currentItem())
                applyOverlayPopupSelection(m_overlayList->currentItem()->data(Qt::UserRole).toInt());
            else
                hideOverlayPopup();
            return true;
        default:
            return false;
    }
}

void UniformComboBox::wheelEvent(QWheelEvent *event)
{
    // 忽略滚轮事件，阻止滚轮切换选项
    event->ignore();
}

void UniformComboBox::bindModel(QAbstractItemModel *model)
{
    QObject::disconnect(m_rowsInsertedConnection);
    QObject::disconnect(m_rowsRemovedConnection);
    QObject::disconnect(m_modelResetConnection);
    QObject::disconnect(m_dataChangedConnection);

    if (!model)
    {
        return;
    }

    m_rowsInsertedConnection =
        QObject::connect(model, &QAbstractItemModel::rowsInserted, this, [this]() { updateComboGeometry(); });
    m_rowsRemovedConnection =
        QObject::connect(model, &QAbstractItemModel::rowsRemoved, this, [this]() { updateComboGeometry(); });
    m_modelResetConnection =
        QObject::connect(model, &QAbstractItemModel::modelReset, this, [this]() { updateComboGeometry(); });
    m_dataChangedConnection =
        QObject::connect(model, &QAbstractItemModel::dataChanged, this, [this]() { updateComboGeometry(); });
}

void UniformComboBox::updateComboGeometry()
{
    adjustSize();
    updateGeometry();
    update();
}
