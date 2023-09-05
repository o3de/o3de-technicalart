#ifndef H_ENGINE_PARAMETERWIDGET_MULTIINSTANCE
#define H_ENGINE_PARAMETERWIDGET_MULTIINSTANCE

#if !defined(Q_MOC_RUN)
#include "HE_ParameterWidget.h"

#include <vector>
#endif

class QGridLayout;
class QPushButton;

namespace HoudiniEngine
{
    const int MultiInstanceMaxButtonWidth = 20;

    class HE_ParameterWidget_MultiInstance : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_MultiInstance() = delete;
        HE_ParameterWidget_MultiInstance(HAPI_ParmId ParmId, HAPI_ParmId ParentParmId);
        ~HE_ParameterWidget_MultiInstance();

        void AddParameter(HE_ParameterWidget* PWidget);

    private:
        QGridLayout *Layout;
        QPushButton *AddBefore;
        QPushButton *Remove;

        std::vector<HE_ParameterWidget*> ParameterWidgets;

        HAPI_ParmId ParentId;
    };
}

#endif
