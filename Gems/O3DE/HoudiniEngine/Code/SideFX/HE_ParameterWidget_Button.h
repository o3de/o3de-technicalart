#ifndef H_ENGINE_PARAMETERWIDGET_BUTTON
#define H_ENGINE_PARAMETERWIDGET_BUTTON

#if !defined(Q_MOC_RUN)
#include "HE_ParameterWidget.h"

#include <HAPI/HAPI.h>
#include <string>
#endif

class QGridLayout;
class QPushButton;

namespace HoudiniEngine
{
    class HE_ParameterWidget_Button : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_Button() = delete;
        HE_ParameterWidget_Button(HAPI_ParmId ParmId, const char *ParmLabel);
        ~HE_ParameterWidget_Button();

        void SetHelpToolTip(std::string HelpString) override;

    private:
        QGridLayout *Layout;
        QPushButton *Button;

    private slots:
        void ButtonClicked();

    signals:
        void Signal_ButtonParmUpdate(HAPI_ParmId);
    };
}

#endif
