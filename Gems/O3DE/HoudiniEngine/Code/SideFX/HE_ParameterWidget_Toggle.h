#ifndef H_ENGINE_PARAMETERWIDGET_TOGGLE
#define H_ENGINE_PARAMETERWIDGET_TOGGLE

#if !defined(Q_MOC_RUN)
#include "HE_ParameterWidget.h"

#include <HAPI/HAPI.h>
#include <string>
#endif

class QHBoxLayout;
class QCheckBox;
class QLabel;
class QSpacerItem;

namespace HoudiniEngine
{
    const int ToggleSpacerItemWidth = 40;

    class HE_ParameterWidget_Toggle : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_Toggle() = delete;
        HE_ParameterWidget_Toggle(HAPI_ParmId ParmId, const char *ParmLabel, bool Checked);
        ~HE_ParameterWidget_Toggle();

        void SetHelpToolTip(std::string HelpString) override;

    private:
        QHBoxLayout *Layout;
        QCheckBox *Toggle;
        QLabel *Label;
        QSpacerItem *Spacer;

    private slots:
        void ToggleStateChanged(int);

    signals:
        void Signal_ToggleParmUpdate(HAPI_ParmId, int);
    };
}

#endif
