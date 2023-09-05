#ifndef H_ENGINE_PARAMETERWIDGET_MULTI
#define H_ENGINE_PARAMETERWIDGET_MULTI

#if !defined(Q_MOC_RUN)
#include "HE_ParameterWidget.h"
#include "HE_ParameterWidget_MultiInstance.h"

#include <vector>
#endif

class QVBoxLayout;
class QWidget;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;

namespace HoudiniEngine
{
    class HE_ParameterWidget_Multi : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_Multi() = delete;
        HE_ParameterWidget_Multi(HAPI_ParmId ParmId, const char *Label, int InstanceCount);
        ~HE_ParameterWidget_Multi();

        void AppendInstance(HE_ParameterWidget_MultiInstance* Instance);
        HE_ParameterWidget_MultiInstance* GetInstance(int Index);

    private:
        int NumOfInstances;

        std::vector<HE_ParameterWidget_MultiInstance*> Instances;

        QVBoxLayout *Layout;
        QWidget *ControlWidget;
        QHBoxLayout *ControlLayout;
        QLabel *ParameterLabel;
        QLineEdit *InstanceCountEdit;
        QPushButton *ClearButton;
        QPushButton *AddInstanceButton;
        QPushButton *RemoveInstanceButton;
    
    private slots:
        void InstanceCountEdit_returnPressed();
        void AddInstanceButton_clicked();
        void RemoveInstanceButton_clicked();
        void ClearButton_clicked();
    };
}

#endif
