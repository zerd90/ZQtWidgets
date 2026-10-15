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

class AppCheckBoxPrivate
{
public:
    explicit AppCheckBoxPrivate(AppCheckBox *owner, const QString &text) : q(owner), m_text(text) {}

    QRect indicatorRect() const { return QRect(0, (q->height() - kBoxSize) / 2, kBoxSize, kBoxSize); }

    AppCheckBox *q = nullptr;
    QString      m_text;
    QToolButton *m_button = nullptr;
};

AppCheckBox::AppCheckBox(const QString &text, QWidget *parent)
    : QWidget(parent), d_ptr(std::make_unique<AppCheckBoxPrivate>(this, text))
{
    Q_D(AppCheckBox);
    ensureQtCustomWidgetsStyle();
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_MacShowFocusRect, false);

    d->m_button = new QToolButton(this);
    d->m_button->setObjectName(QStringLiteral("appCheckBoxButton"));
    d->m_button->setCheckable(true);
    d->m_button->setChecked(false);
    d->m_button->setAutoRaise(true);
    d->m_button->setCursor(Qt::PointingHandCursor);
    d->m_button->setFocusPolicy(Qt::StrongFocus);
    d->m_button->setAttribute(Qt::WA_MacShowFocusRect, false);
    d->m_button->setAttribute(Qt::WA_TranslucentBackground, true);
    d->m_button->setAttribute(Qt::WA_NoSystemBackground, true);
    d->m_button->setAutoFillBackground(false);
    d->m_button->setStyleSheet(QStringLiteral("QToolButton#appCheckBoxButton,"
                                              "QToolButton#appCheckBoxButton:hover,"
                                              "QToolButton#appCheckBoxButton:pressed,"
                                              "QToolButton#appCheckBoxButton:checked,"
                                              "QToolButton#appCheckBoxButton:checked:hover,"
                                              "QToolButton#appCheckBoxButton:focus,"
                                              "QToolButton#appCheckBoxButton:disabled {"
                                              " background:transparent; border:none; padding:0; outline:none; }"));
    d->m_button->setGeometry(rect());
    d->m_button->installEventFilter(this);

    connect(d->m_button, &QAbstractButton::toggled, this,
            [this](bool checked)
            {
                update();
                emit toggled(checked);
            });
}

AppCheckBox::~AppCheckBox() = default;

bool AppCheckBox::isChecked() const
{
    Q_D(const AppCheckBox);
    return d->m_button && d->m_button->isChecked();
}

void AppCheckBox::setChecked(bool checked)
{
    Q_D(AppCheckBox);
    if (d->m_button)
        d->m_button->setChecked(checked);
}

QSize AppCheckBox::sizeHint() const
{
    Q_D(const AppCheckBox);
    const QFontMetrics metrics(font());
    const int          textWidth = metrics.horizontalAdvance(d->m_text);
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
    Q_D(AppCheckBox);
    QWidget::resizeEvent(event);
    if (d->m_button)
        d->m_button->setGeometry(rect());
}

void AppCheckBox::paintEvent(QPaintEvent *)
{
    Q_D(AppCheckBox);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const bool   enabled = isEnabled();
    const bool   checked = isChecked();
    const bool   hovered = enabled && d->m_button && d->m_button->underMouse();
    const QRectF box     = QRectF(d->indicatorRect()).adjusted(0.5, 0.5, -0.5, -0.5);

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
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, d->m_text);
}

bool AppCheckBox::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(AppCheckBox);
    if (watched == d->m_button && event)
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
