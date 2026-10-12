#include "ToggleSwitch.h"
#include "WidgetColors.h"
#include "WidgetStyle.h"

#include <QEvent>

ToggleSwitch::ToggleSwitch(QWidget *parent) : QWidget(parent)
{
    ensureQtCustomWidgetsStyle();
    m_button = new QToolButton(this);
    m_button->setObjectName(QStringLiteral("toggleSwitchButton"));
    m_button->setCheckable(true);
    m_button->setChecked(false);
    m_button->setAutoRaise(true);
    m_button->setCursor(Qt::PointingHandCursor);
    m_button->setFocusPolicy(Qt::StrongFocus);
    m_button->setAttribute(Qt::WA_TranslucentBackground, true);
    m_button->setAttribute(Qt::WA_NoSystemBackground, true);
    m_button->setAutoFillBackground(false);
    m_button->setStyleSheet(QStringLiteral("QToolButton#toggleSwitchButton,"
                                           "QToolButton#toggleSwitchButton:hover,"
                                           "QToolButton#toggleSwitchButton:pressed,"
                                           "QToolButton#toggleSwitchButton:checked,"
                                           "QToolButton#toggleSwitchButton:checked:hover,"
                                           "QToolButton#toggleSwitchButton:disabled {"
                                           " background:transparent; border:none; padding:0; }"));
    m_button->installEventFilter(this);
    setFixedSize(QSize(44, 22));
    m_button->setGeometry(rect());

    QObject::connect(m_button, &QAbstractButton::toggled, this,
                     [this](bool checked)
                     {
                         update();
                         emit toggled(checked);
                     });
}
bool ToggleSwitch::isChecked() const
{
    return m_button && m_button->isChecked();
}
void ToggleSwitch::setChecked(bool checked)
{
    if (m_button)
    {
        m_button->setChecked(checked);
    }
}
QAbstractButton *ToggleSwitch::button() const
{
    return m_button;
}
void ToggleSwitch::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_button)
    {
        m_button->setGeometry(rect());
    }
}
bool ToggleSwitch::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_button && event)
    {
        switch (event->type())
        {
            case QEvent::Enter:
            case QEvent::Leave:
            case QEvent::HoverEnter:
            case QEvent::HoverLeave:
                update();
                break;
            default:
                break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ToggleSwitch::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF r      = rect().adjusted(1, 1, -1, -1);
    const qreal  radius = r.height() / 2.0;

    const bool checked = isChecked();
    const bool hovered = isEnabled() && m_button && m_button->underMouse();

    QColor trackOff    = WidgetColors::color(WidgetColors::Role::Border);
    QColor trackOn     = WidgetColors::color(WidgetColors::Role::Accent);
    QColor knobColor   = WidgetColors::color(WidgetColors::Role::Surface);
    QColor trackBorder = WidgetColors::color(WidgetColors::Role::TextDisabled);
    trackBorder.setAlpha(80);

    if (hovered && !checked)
    {
        trackOff = WidgetColors::color(WidgetColors::Role::TextDisabled);
        trackBorder.setAlpha(120);
    }
    else if (hovered && checked)
    {
        trackOn = WidgetColors::color(WidgetColors::Role::AccentHover);
    }

    if (!isEnabled())
    {
        trackOff.setAlphaF(0.6);
        trackOn.setAlphaF(0.6);
        knobColor.setAlphaF(0.85);
        trackBorder.setAlpha(45);
    }

    p.setPen(QPen(trackBorder, 1.0));
    p.setBrush(checked ? trackOn : trackOff);
    p.drawRoundedRect(r, radius, radius);

    const qreal knobSize = r.height() - 4;
    const qreal knobY    = r.top() + 2;
    const qreal knobX    = checked ? (r.right() - 2 - knobSize) : (r.left() + 2);
    p.setPen(Qt::NoPen);
    p.setBrush(knobColor);
    p.drawEllipse(QRectF(knobX, knobY, knobSize, knobSize));
}
