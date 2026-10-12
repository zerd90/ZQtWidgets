#pragma once

#include <QWidget>

class QPushButton;
class QSlider;
class QTimer;

class StepButtonSlider final : public QWidget
{
    Q_OBJECT

public:
    explicit StepButtonSlider(Qt::Orientation orientation, QWidget *parent = nullptr, bool stepButtonsVisible = true);

    int  value() const;
    int  minimum() const;
    int  maximum() const;
    void setRange(int minimum, int maximum);
    void setValue(int value);
    void setSingleStep(int step);
    void setPageStep(int step);
    void setMinimumWidth(int width);
    bool isSliderDown() const;

    void setStepButtonsVisible(bool visible);
    bool stepButtonsVisible() const;

signals:
    void valueChanged(int value);
    void sliderReleased();

private:
    void step(int direction);
    void beginStepping(int direction);
    void endStepping(int direction);

    QPushButton *m_minusButton   = nullptr;
    QSlider     *m_slider        = nullptr;
    QPushButton *m_plusButton    = nullptr;
    QTimer      *m_repeatTimer   = nullptr;
    int          m_stepDirection = 0;
};
