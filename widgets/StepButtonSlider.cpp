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

class StepButtonSliderPrivate
{
public:
    void step(int direction)
    {
        if (!m_slider || direction == 0)
            return;
        m_slider->setValue(m_slider->value() + direction * m_slider->singleStep());
    }

    void beginStepping(int direction)
    {
        m_stepDirection = direction;
        m_slider->setSliderDown(true);
        step(direction);
        m_repeatTimer->start(400);
    }

    void endStepping(int direction)
    {
        if (m_stepDirection != direction)
            return;
        m_repeatTimer->stop();
        m_stepDirection = 0;
        m_slider->setSliderDown(false);
    }

    QPushButton *m_minusButton   = nullptr;
    QSlider     *m_slider        = nullptr;
    QPushButton *m_plusButton    = nullptr;
    QTimer      *m_repeatTimer   = nullptr;
    int          m_stepDirection = 0;
};

StepButtonSlider::StepButtonSlider(Qt::Orientation orientation, QWidget *parent, bool stepButtonsVisible)
    : QWidget(parent), d_ptr(std::make_unique<StepButtonSliderPrivate>())
{
    Q_D(StepButtonSlider);
    ensureQtCustomWidgetsStyle();
    setObjectName(QStringLiteral("stepButtonSlider"));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    d->m_minusButton = new QPushButton(QStringLiteral("-"), this);
    d->m_minusButton->setObjectName(QStringLiteral("stepButtonSliderButton"));
    d->m_minusButton->setFocusPolicy(Qt::NoFocus);
    d->m_minusButton->setCursor(Qt::PointingHandCursor);
    d->m_minusButton->setFixedSize(20, 20);
    d->m_minusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    d->m_slider = new ClickJumpSlider(orientation, this);
    d->m_slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    d->m_plusButton = new QPushButton(QStringLiteral("+"), this);
    d->m_plusButton->setObjectName(QStringLiteral("stepButtonSliderButton"));
    d->m_plusButton->setFocusPolicy(Qt::NoFocus);
    d->m_plusButton->setCursor(Qt::PointingHandCursor);
    d->m_plusButton->setFixedSize(20, 20);
    d->m_plusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    layout->addWidget(d->m_minusButton);
    layout->addWidget(d->m_slider, 1);
    layout->addWidget(d->m_plusButton);

    d->m_repeatTimer = new QTimer(this);
    d->m_repeatTimer->setSingleShot(false);
    connect(d->m_repeatTimer, &QTimer::timeout, this,
            [d]()
            {
                d->step(d->m_stepDirection);
                if (d->m_repeatTimer->interval() != 80)
                    d->m_repeatTimer->setInterval(80);
            });
    connect(d->m_minusButton, &QPushButton::pressed, this, [d]() { d->beginStepping(-1); });
    connect(d->m_minusButton, &QPushButton::released, this, [d]() { d->endStepping(-1); });
    connect(d->m_plusButton, &QPushButton::pressed, this, [d]() { d->beginStepping(1); });
    connect(d->m_plusButton, &QPushButton::released, this, [d]() { d->endStepping(1); });
    connect(d->m_slider, &QSlider::valueChanged, this, &StepButtonSlider::valueChanged);
    connect(d->m_slider, &QSlider::sliderReleased, this, &StepButtonSlider::sliderReleased);

    setStepButtonsVisible(stepButtonsVisible);
}

StepButtonSlider::~StepButtonSlider() = default;

int StepButtonSlider::value() const
{
    Q_D(const StepButtonSlider);
    return d->m_slider ? d->m_slider->value() : 0;
}

int StepButtonSlider::minimum() const
{
    Q_D(const StepButtonSlider);
    return d->m_slider ? d->m_slider->minimum() : 0;
}

int StepButtonSlider::maximum() const
{
    Q_D(const StepButtonSlider);
    return d->m_slider ? d->m_slider->maximum() : 0;
}

void StepButtonSlider::setRange(int minimum, int maximum)
{
    Q_D(StepButtonSlider);
    if (d->m_slider)
        d->m_slider->setRange(minimum, maximum);
}

void StepButtonSlider::setValue(int value)
{
    Q_D(StepButtonSlider);
    if (d->m_slider)
        d->m_slider->setValue(value);
}

void StepButtonSlider::setSingleStep(int step)
{
    Q_D(StepButtonSlider);
    if (d->m_slider)
        d->m_slider->setSingleStep(step);
}

void StepButtonSlider::setPageStep(int step)
{
    Q_D(StepButtonSlider);
    if (d->m_slider)
        d->m_slider->setPageStep(step);
}

void StepButtonSlider::setMinimumWidth(int width)
{
    Q_D(StepButtonSlider);
    if (d->m_slider)
        d->m_slider->setMinimumWidth(width);
}

bool StepButtonSlider::isSliderDown() const
{
    Q_D(const StepButtonSlider);
    return d->m_slider && d->m_slider->isSliderDown();
}

void StepButtonSlider::setStepButtonsVisible(bool visible)
{
    Q_D(StepButtonSlider);
    if (d->m_minusButton)
        d->m_minusButton->setVisible(visible);
    if (d->m_plusButton)
        d->m_plusButton->setVisible(visible);
}

bool StepButtonSlider::stepButtonsVisible() const
{
    Q_D(const StepButtonSlider);
    return d->m_minusButton && d->m_minusButton->isVisible();
}
