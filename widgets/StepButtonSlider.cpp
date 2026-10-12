#include "StepButtonSlider.h"
#include "WidgetStyle.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPushButton>
#include <QStyle>
#include <QStyleOptionSlider>
#include <QTimer>
#include <QWheelEvent>

namespace
{
    class ClickJumpSlider final : public QSlider
    {
    public:
        using QSlider::QSlider;

    protected:
        void mousePressEvent(QMouseEvent *event) override
        {
            if (event == nullptr)
                return;

            if (event->button() == Qt::LeftButton)
            {
                QStyleOptionSlider opt;
                initStyleOption(&opt);

                const QRect handleRect =
                    style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
                if (!handleRect.contains(event->pos()))
                {
                    const int pos = (orientation() == Qt::Horizontal) ? event->pos().x() : event->pos().y();

                    const int sliderLength = style()->pixelMetric(QStyle::PM_SliderLength, &opt, this);
                    const int sliderMin    = (orientation() == Qt::Horizontal) ? opt.rect.x() : opt.rect.y();
                    const int sliderMax    = (orientation() == Qt::Horizontal) ? (opt.rect.right() - sliderLength + 1)
                                                                               : (opt.rect.bottom() - sliderLength + 1);

                    const int newPos   = pos - (sliderLength / 2);
                    const int newValue = QStyle::sliderValueFromPosition(minimum(), maximum(), newPos - sliderMin,
                                                                         sliderMax - sliderMin, opt.upsideDown);

                    setValue(newValue);

                    QStyleOptionSlider optAfter;
                    initStyleOption(&optAfter);
                    const QRect handleRectAfter =
                        style()->subControlRect(QStyle::CC_Slider, &optAfter, QStyle::SC_SliderHandle, this);
                    const QPointF localPos(handleRectAfter.center());
                    QMouseEvent   fakeEvent(QEvent::MouseButtonPress, localPos, mapToGlobal(handleRectAfter.center()),
                                            Qt::LeftButton, event->buttons() | Qt::LeftButton, event->modifiers());
                    QSlider::mousePressEvent(&fakeEvent);
                    event->accept();
                    return;
                }
            }

            QSlider::mousePressEvent(event);
        }

        void wheelEvent(QWheelEvent *event) override
        {
            if (event)
                event->ignore();
        }
    };
} // namespace

StepButtonSlider::StepButtonSlider(Qt::Orientation orientation, QWidget *parent, bool stepButtonsVisible)
    : QWidget(parent)
{
    ensureQtCustomWidgetsStyle();
    setObjectName(QStringLiteral("stepButtonSlider"));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    m_minusButton = new QPushButton(QStringLiteral("-"), this);
    m_minusButton->setObjectName(QStringLiteral("stepButtonSliderButton"));
    m_minusButton->setFocusPolicy(Qt::NoFocus);
    m_minusButton->setCursor(Qt::PointingHandCursor);
    m_minusButton->setFixedSize(20, 20);
    m_minusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_slider = new ClickJumpSlider(orientation, this);
    m_slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_plusButton = new QPushButton(QStringLiteral("+"), this);
    m_plusButton->setObjectName(QStringLiteral("stepButtonSliderButton"));
    m_plusButton->setFocusPolicy(Qt::NoFocus);
    m_plusButton->setCursor(Qt::PointingHandCursor);
    m_plusButton->setFixedSize(20, 20);
    m_plusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    layout->addWidget(m_minusButton);
    layout->addWidget(m_slider, 1);
    layout->addWidget(m_plusButton);

    m_repeatTimer = new QTimer(this);
    m_repeatTimer->setSingleShot(false);
    connect(m_repeatTimer, &QTimer::timeout, this,
            [this]()
            {
                step(m_stepDirection);
                if (m_repeatTimer->interval() != 80)
                    m_repeatTimer->setInterval(80);
            });
    connect(m_minusButton, &QPushButton::pressed, this, [this]() { beginStepping(-1); });
    connect(m_minusButton, &QPushButton::released, this, [this]() { endStepping(-1); });
    connect(m_plusButton, &QPushButton::pressed, this, [this]() { beginStepping(1); });
    connect(m_plusButton, &QPushButton::released, this, [this]() { endStepping(1); });
    connect(m_slider, &QSlider::valueChanged, this, &StepButtonSlider::valueChanged);
    connect(m_slider, &QSlider::sliderReleased, this, &StepButtonSlider::sliderReleased);

    setStepButtonsVisible(stepButtonsVisible);
}

int StepButtonSlider::value() const
{
    return m_slider ? m_slider->value() : 0;
}

int StepButtonSlider::minimum() const
{
    return m_slider ? m_slider->minimum() : 0;
}

int StepButtonSlider::maximum() const
{
    return m_slider ? m_slider->maximum() : 0;
}

void StepButtonSlider::setRange(int minimum, int maximum)
{
    if (m_slider)
        m_slider->setRange(minimum, maximum);
}

void StepButtonSlider::setValue(int value)
{
    if (m_slider)
        m_slider->setValue(value);
}

void StepButtonSlider::setSingleStep(int step)
{
    if (m_slider)
        m_slider->setSingleStep(step);
}

void StepButtonSlider::setPageStep(int step)
{
    if (m_slider)
        m_slider->setPageStep(step);
}

void StepButtonSlider::setMinimumWidth(int width)
{
    if (m_slider)
        m_slider->setMinimumWidth(width);
}

bool StepButtonSlider::isSliderDown() const
{
    return m_slider && m_slider->isSliderDown();
}

void StepButtonSlider::step(int direction)
{
    if (!m_slider || direction == 0)
        return;
    m_slider->setValue(m_slider->value() + direction * m_slider->singleStep());
}

void StepButtonSlider::beginStepping(int direction)
{
    m_stepDirection = direction;
    m_slider->setSliderDown(true);
    step(direction);
    m_repeatTimer->start(400);
}

void StepButtonSlider::endStepping(int direction)
{
    if (m_stepDirection != direction)
        return;
    m_repeatTimer->stop();
    m_stepDirection = 0;
    m_slider->setSliderDown(false);
}

void StepButtonSlider::setStepButtonsVisible(bool visible)
{
    if (m_minusButton)
        m_minusButton->setVisible(visible);
    if (m_plusButton)
        m_plusButton->setVisible(visible);
}

bool StepButtonSlider::stepButtonsVisible() const
{
    return m_minusButton && m_minusButton->isVisible();
}
