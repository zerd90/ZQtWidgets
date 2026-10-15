#pragma once

#include <QWidget>

#include <memory>

class StepButtonSliderPrivate;

class StepButtonSlider final : public QWidget
{
    Q_OBJECT

public:
    explicit StepButtonSlider(Qt::Orientation orientation, QWidget *parent = nullptr, bool stepButtonsVisible = true);
    ~StepButtonSlider() override;

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
    Q_DECLARE_PRIVATE(StepButtonSlider)
    std::unique_ptr<StepButtonSliderPrivate> d_ptr;
};
