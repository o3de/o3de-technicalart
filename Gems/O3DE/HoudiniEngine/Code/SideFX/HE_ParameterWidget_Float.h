#ifndef H_ENGINE_PARAMETERWIDGET_FLOAT
#define H_ENGINE_PARAMETERWIDGET_FLOAT

#if !defined(Q_MOC_RUN)
#include "HE_ParameterWidget.h"

#include <vector>
#include <string>
#include <HAPI/HAPI.h>
#endif

class QGridLayout;
class QLabel;
class QLineEdit;
class QSlider;

namespace HoudiniEngine
{
    class HE_ParameterWidget_Float : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_Float() = delete;
        HE_ParameterWidget_Float(HAPI_ParmId ParmId, const char *ParmLabel, const std::vector<float> &Values, int ParmSize);
        HE_ParameterWidget_Float(HAPI_ParmId ParmId, const char *ParmLabel, float Value, float Min, float Max);
        ~HE_ParameterWidget_Float();

        void SetHelpToolTip(std::string HelpString) override;

    private:
        QGridLayout *Layout;
        QLabel *Label;
        std::vector<QLineEdit*> Floats;
        QSlider* Slider;

        float UIMin;
        float UIMax;

        int CalculateSliderPosition();
        void UpdateSliderPosition();
        float CalculateFloatValueFromSliderPosition();
        float CalculateFloatValueFromSliderPosition(int Position);

        std::vector<float> GetFloatValues();

    private slots:
        void EditingFinished();
        void SliderValueChanged(int Value);
        void SliderReleased();

    signals:
        void Signal_FloatParmUpdate(HAPI_ParmId, std::vector<float>);
    };
}

#endif
