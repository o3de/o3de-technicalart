#include "StdAfx.h"
#include "HE_ParameterWidget_Integer.h"

#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qslider.h>

namespace HoudiniEngine
{
    HE_ParameterWidget_Integer::HE_ParameterWidget_Integer(HAPI_ParmId ParmId, const char *ParmLabel,
                                                            const std::vector<int> &Values, int ParmSize)
        : HE_ParameterWidget(ParmId)
    {
        Layout = new QGridLayout;
        Label = new QLabel(ParmLabel);
        Slider = nullptr;

        Layout->addWidget(Label, 0, 0);

        Integers.resize(ParmSize);

        for (int i = 0; i < ParmSize; i++)
        {
            Integers[i] = new QLineEdit(std::to_string(Values[i]).c_str());
            Layout->addWidget(Integers[i], 0, i + 1);

            QObject::connect(Integers[i], SIGNAL(editingFinished()), this, SLOT(EditingFinished()));
        }

        Layout->setAlignment(Qt::AlignTop);
        Label->setAlignment(Qt::AlignCenter);
        Layout->setColumnMinimumWidth(0, 100);
        
        this->setLayout(Layout);
    }

    HE_ParameterWidget_Integer::HE_ParameterWidget_Integer(HAPI_ParmId ParmId, const char *ParmLabel,
                                                            int Value,
                                                            int UIMin, int UIMax)
        : HE_ParameterWidget(ParmId)
    {
        Layout = new QGridLayout;
        Label = new QLabel(ParmLabel);
        Slider = new QSlider(Qt::Horizontal);

        Layout->addWidget(Label, 0, 0);

        Integers.resize(1);
        Integers[0] = new QLineEdit(std::to_string(Value).c_str());
        Layout->addWidget(Integers[0], 0, 1);

        QObject::connect(Integers[0], SIGNAL(editingFinished()), this, SLOT(EditingFinished()));

        Layout->setAlignment(Qt::AlignTop);
        Label->setAlignment(Qt::AlignCenter);
        Layout->setColumnMinimumWidth(0, 100);

        Slider->setRange(UIMin, UIMax);
        Slider->setSingleStep(1);
        Slider->setPageStep(1);

        UpdateSliderPosition();

        Layout->addWidget(Slider, 0, 2);

        this->setLayout(Layout);

        QObject::connect(Slider, SIGNAL(valueChanged(int)), this, SLOT(SliderValueChanged(int)));
        QObject::connect(Slider, SIGNAL(sliderReleased()), this, SLOT(SliderReleased()));
    }

    HE_ParameterWidget_Integer::~HE_ParameterWidget_Integer()
    {
        for (int i = 0; i < Integers.size(); i++)
        {
            if (Integers[i])
            {
                delete Integers[i];
            }
        }

        if (Slider)
        {
            delete Slider;
        }

        delete Label;
        delete Layout;
    }

    void
    HE_ParameterWidget_Integer::SetHelpToolTip(std::string HelpString)
    {
        if (Label)
        {
            Label->setToolTip(HelpString.c_str());
        }
    }

    int
    HE_ParameterWidget_Integer::CalculateSliderPosition()
    {
        return (Integers[0]->text().toInt());
    }

    void
    HE_ParameterWidget_Integer::UpdateSliderPosition()
    {
        if (Slider)
        {
            Slider->setSliderPosition(CalculateSliderPosition());
        }
    }

    int
    HE_ParameterWidget_Integer::CalculateValueFromSliderPosition()
    {
        return (Slider->sliderPosition());
    }

    std::vector<int>
    HE_ParameterWidget_Integer::GetIntegerValues()
    {
        std::vector<int> Ret;
        for (int i = 0; i < Integers.size(); i++)
        {
            Ret.push_back(Integers[i]->text().toInt()); 
        }

        return Ret;
    }

    void
    HE_ParameterWidget_Integer::EditingFinished()
    {
        emit Signal_IntegerParmUpdate(Id, GetIntegerValues()); 
    }

    void
    HE_ParameterWidget_Integer::SliderValueChanged(int Value)
    {
        Integers[0]->setText(std::to_string(Value).c_str());
    }

    void
    HE_ParameterWidget_Integer::SliderReleased()
    {
        Integers[0]->setText(std::to_string(CalculateValueFromSliderPosition()).c_str()); 
        emit Signal_IntegerParmUpdate(Id, GetIntegerValues());
    }
}

#include "SideFX/moc_HE_ParameterWidget_Integer.cpp"
