#ifndef H_ENGINE_PARAMETERWIDGET
#define H_ENGINE_PARAMETERWIDGET

#if !defined(Q_MOC_RUN)
#include <QtWidgets/qwidget.h>
#include <HAPI/HAPI.h>
#include <string>
#endif

namespace HoudiniEngine
{
    class HE_ParameterWidget : public QWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget() = delete;
        HE_ParameterWidget(HAPI_ParmId ParmId);
        ~HE_ParameterWidget();

        virtual void SetHelpToolTip(std::string HelpString);

    protected:
        HAPI_ParmId Id;
    };
}

#endif
