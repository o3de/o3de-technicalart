#include "StdAfx.h"
#include "HE_ParameterWidget_Float.h"

#include <string>
#include <vector>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qslider.h>

namespace HoudiniEngine
{
    HE_ParameterWidget_Float::HE_ParameterWidget_Float(HAPI_ParmId ParmId, const char *ParmLabel, const std::vector<float> &Values, int ParmSize)
        : HE_ParameterWidget(ParmId)
    {
        Layout = new QGridLayout;
        Label = new QLabel(ParmLabel);
        Slider = nullptr;

        Layout->addWidget(Label, 0, 0);

        Floats.resize(ParmSize);
        for (int i = 0; i < ParmSize; i++)
        {
            Floats[i] = new QLineEdit(std::to_string(Values[i]).c_str());
            Layout->addWidget(Floats[i], 0, i + 1);
            QObject::connect(Floats[i], SIGNAL(editingFinished()),
                             this, SLOT(EditingFinished()));
        }

        Layout->setAlignment(Qt::AlignTop);
        Label->setAlignment(Qt::AlignCenter);
        Layout->setColumnMinimumWidth(0, 100);

        this->setLayout(Layout);
    }

    HE_ParameterWidget_Float::HE_ParameterWidget_Float(HAPI_ParmId ParmId, const char *ParmLabel, float Value,
                                                        float Min, float Max)
        : HE_ParameterWidget(ParmId)
    {
        UIMin = Min;
        UIMax = Max;

        Layout = new QGridLayout;
        Label = new QLabel(ParmLabel);
        Slider = new QSlider(Qt::Horizontal);

        Layout->addWidget(Label, 0, 0);

        Floats.resize(1);
        Floats[0] = new QLineEdit(std::to_string(Value).c_str());
        Layout->addWidget(Floats[0], 0, 1);

        QObject::connect(Floats[0], SIGNAL(editingFinished()),
                         this, SLOT(EditingFinished()));

        Layout->setAlignment(Qt::AlignTop);
        Label->setAlignment(Qt::AlignCenter);
        Layout->setColumnMinimumWidth(0, 100);

        Slider->setRange(0, 100);
        Slider->setSingleStep(1);

        UpdateSliderPosition();

        Layout->addWidget(Slider, 0, 2);

        this->setLayout(Layout);

        QObject::connect(Slider, SIGNAL(valueChanged(int)), this, SLOT(SliderValueChanged(int)));
        QObject::connect(Slider, SIGNAL(sliderReleased()), this, SLOT(SliderReleased()));
    }

    HE_ParameterWidget_Float::~HE_ParameterWidget_Float()
    {
        for (int i = 0; i < Floats.size(); i++)
        {
            if (Floats[i])
            {
                delete Floats[i];
            }

            if (Slider)
            {
                delete Slider;
            }
        }

        delete Label;
        delete Layout;
    }

    void
    HE_ParameterWidget_Float::SetHelpToolTip(std::string HelpString)
    {
        if (Label)
        {
            Label->setToolTip(HelpString.c_str());
        }
    }

    int
    HE_ParameterWidget_Float::CalculateSliderPosition()
    {
        return (100 * ((Floats[0]->text().toFloat() - UIMin) / (UIMax - UIMin)));
    }

    void
    HE_ParameterWidget_Float::UpdateSliderPosition()
    {
        if (Slider)
        {
            Slider->setSliderPosition(CalculateSliderPosition());
        }
    }

    float
    HE_ParameterWidget_Float::CalculateFloatValueFromSliderPosition()
    {
        return CalculateFloatValueFromSliderPosition(Slider->sliderPosition());
    }

    float
    HE_ParameterWidget_Float::CalculateFloatValueFromSliderPosition(int Position)
    {
        float SliderPercentage = Position / 100.0;

        return (((UIMax - UIMin) * SliderPercentage) + UIMin);
    }

    std::vector<float>
    HE_ParameterWidget_Float::GetFloatValues()
    {
        std::vector<float> Ret;
        for (int i = 0; i < Floats.size(); i++)
        {
            Ret.push_back(Floats[i]->text().toFloat());
        }

        return Ret;
    }

    void
    HE_ParameterWidget_Float::EditingFinished()
    {
        emit Signal_FloatParmUpdate(Id, GetFloatValues());
    }

    void
    HE_ParameterWidget_Float::SliderValueChanged(int Value)
    {
        Floats[0]->setText(std::to_string(CalculateFloatValueFromSliderPosition(Value)).c_str());
    }

    void
    HE_ParameterWidget_Float::SliderReleased()
    {
        Floats[0]->setText(std::to_string(CalculateFloatValueFromSliderPosition()).c_str());
        emit Signal_FloatParmUpdate(Id, GetFloatValues());
    }
}

#include "SideFX/moc_HE_ParameterWidget_Float.cpp"
