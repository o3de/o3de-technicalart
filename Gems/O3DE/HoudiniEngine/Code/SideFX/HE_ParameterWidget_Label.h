#ifndef H_ENGINE_PARAMETERWIDGET_LABEL
#define H_ENGINE_PARAMETERWIDGET_LABEL

#if !defined(Q_MOC_RUN)
#include "HE_ParameterWidget.h"

#include <string>
#endif

class QGridLayout;
class QLabel;

namespace HoudiniEngine
{
    class HE_ParameterWidget_Label : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_Label() = delete;
        HE_ParameterWidget_Label(HAPI_ParmId ParmId, const char *LabelText);
        ~HE_ParameterWidget_Label();

        void SetHelpToolTip(std::string HelpString) override;

    private:
        QGridLayout* Layout;
        QLabel* Label;
    };
}

#endif
