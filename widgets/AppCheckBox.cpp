#include "AppCheckBox.h"
#include "WidgetColors.h"
#include "WidgetStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QToolButton>

namespace
{
    constexpr int kBoxSize = 16;
    constexpr int kGap     = 8;
} // namespace

AppCheckBox::AppCheckBox(const QString &text, QWidget *parent) : QWidget(parent), m_text(text)
{
    ensureQtCustomWidgetsStyle();
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_MacShowFocusRect, false);

    m_button = new QToolButton(this);
    m_button->setObjectName(QStringLiteral("appCheckBoxButton"));
    m_button->setCheckable(true);
    m_button->setChecked(false);
    m_button->setAutoRaise(true);
    m_button->setCursor(Qt::PointingHandCursor);
    m_button->setFocusPolicy(Qt::StrongFocus);
    m_button->setAttribute(Qt::WA_MacShowFocusRect, false);
    m_button->setAttribute(Qt::WA_TranslucentBackground, true);
    m_button->setAttribute(Qt::WA_NoSystemBackground, true);
    m_button->setAutoFillBackground(false);
    m_button->setStyleSheet(QStringLiteral("QToolButton#appCheckBoxButton,"
                                           "QToolButton#appCheckBoxButton:hover,"
                                           "QToolButton#appCheckBoxButton:pressed,"
                                           "QToolButton#appCheckBoxButton:checked,"
                                           "QToolButton#appCheckBoxButton:checked:hover,"
                                           "QToolButton#appCheckBoxButton:focus,"
                                           "QToolButton#appCheckBoxButton:disabled {"
                                           " background:transparent; border:none; padding:0; outline:none; }"));
    m_button->setGeometry(rect());
    m_button->installEventFilter(this);

    connect(m_button, &QAbstractButton::toggled, this,
            [this](bool checked)
            {
                update();
                emit toggled(checked);
            });
}

bool AppCheckBox::isChecked() const
{
    return m_button && m_button->isChecked();
}

void AppCheckBox::setChecked(bool checked)
{
    if (m_button)
        m_button->setChecked(checked);
}

QSize AppCheckBox::sizeHint() const
{
    const QFontMetrics metrics(font());
    const int          textWidth = metrics.horizontalAdvance(m_text);
    const int          height    = qMax(kBoxSize, metrics.height());
    return QSize(kBoxSize + kGap + textWidth, height);
}

QSize AppCheckBox::minimumSizeHint() const
{
    return sizeHint();
}

void AppCheckBox::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::EnabledChange || event->type() == QEvent::FontChange
        || event->type() == QEvent::PaletteChange)
        update();
}

void AppCheckBox::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_button)
        m_button->setGeometry(rect());
}

QRect AppCheckBox::indicatorRect() const
{
    return QRect(0, (height() - kBoxSize) / 2, kBoxSize, kBoxSize);
}

void AppCheckBox::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const bool   enabled = isEnabled();
    const bool   checked = isChecked();
    const bool   hovered = enabled && m_button && m_button->underMouse();
    const QRectF box     = QRectF(indicatorRect()).adjusted(0.5, 0.5, -0.5, -0.5);

    QColor fill   = WidgetColors::color(WidgetColors::Role::Surface);
    QColor border = hovered ? WidgetColors::color(WidgetColors::Role::TextDisabled)
                            : WidgetColors::color(WidgetColors::Role::Border);
    if (checked)
    {
        fill   = hovered ? WidgetColors::color(WidgetColors::Role::AccentHover)
                         : WidgetColors::color(WidgetColors::Role::Accent);
        border = fill;
    }
    if (!enabled)
    {
        fill.setAlpha(checked ? 90 : 255);
        border.setAlpha(checked ? 90 : 255);
        if (!checked)
            fill = WidgetColors::color(WidgetColors::Role::SurfaceMuted);
    }

    painter.setPen(QPen(border, 1.0));
    painter.setBrush(fill);
    painter.drawRoundedRect(box, 4.0, 4.0);

    if (checked)
    {
        QPainterPath mark;
        mark.moveTo(box.left() + 3.5, box.center().y());
        mark.lineTo(box.left() + 6.4, box.bottom() - 3.6);
        mark.lineTo(box.right() - 3.2, box.top() + 4.2);
        QColor markColor = WidgetColors::color(WidgetColors::Role::Surface);
        if (!enabled)
            markColor.setAlpha(180);
        painter.setPen(QPen(markColor, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(mark);
    }

    painter.setPen(enabled ? WidgetColors::color(WidgetColors::Role::Text)
                           : WidgetColors::color(WidgetColors::Role::TextDisabled));
    painter.setFont(font());
    const QRect textRect(kBoxSize + kGap, 0, qMax(0, width() - kBoxSize - kGap), height());
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_text);
}

bool AppCheckBox::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_button && event)
    {
        switch (event->type())
        {
            case QEvent::Enter:
            case QEvent::Leave:
            case QEvent::HoverEnter:
            case QEvent::HoverLeave:
            case QEvent::FocusIn:
            case QEvent::FocusOut:
                update();
                break;
            default:
                break;
        }
    }
    return QWidget::eventFilter(watched, event);
}
