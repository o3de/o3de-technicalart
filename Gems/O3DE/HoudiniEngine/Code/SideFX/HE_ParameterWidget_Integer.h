#ifndef H_ENGINE_PARAMETERWIDGET_INTEGER
#define H_ENGINE_PARAMETERWIDGET_INTEGER

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
    class HE_ParameterWidget_Integer : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_Integer() = delete;
        HE_ParameterWidget_Integer(HAPI_ParmId ParmId, const char *ParmLabel, const std::vector<int> &Values, int ParmSize);
        HE_ParameterWidget_Integer(HAPI_ParmId ParmId, const char *ParmLabel, int Value, int UIMin, int UIMax);
        ~HE_ParameterWidget_Integer();

        void SetHelpToolTip(std::string HelpString) override;

        int CalculateSliderPosition();
        void UpdateSliderPosition();
        int CalculateValueFromSliderPosition();

    private:
        QGridLayout *Layout;
        QLabel *Label;
        std::vector<QLineEdit*> Integers;
        QSlider* Slider;

        std::vector<int> GetIntegerValues();

    private slots:
        void EditingFinished();
        void SliderValueChanged(int Value);
        void SliderReleased();

    signals:
        void Signal_IntegerParmUpdate(HAPI_ParmId ParmId, std::vector<int> Integers);
    };
}

#endif
