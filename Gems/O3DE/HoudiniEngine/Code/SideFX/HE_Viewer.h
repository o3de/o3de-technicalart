#ifndef H_ENGINE_VIEWER
#define H_ENGINE_VIEWER

#if !defined(Q_MOC_RUN)
#include "HE_ParameterWidget.h"
#include "HE_ParameterWidget_Folder.h"
#include "HE_ParameterWidget_FolderList.h"
#include "HE_ParameterWidget_Label.h"
#include "HE_ParameterWidget_Integer.h"
#include "HE_ParameterWidget_String.h"
#include "HE_ParameterWidget_Float.h"
#include "HE_ParameterWidget_Toggle.h"
#include "HE_ParameterWidget_Button.h"
#include "HE_ParameterWidget_IntegerChoice.h"
#include "HE_ParameterWidget_StringChoice.h"
#include "HE_ParameterWidget_Multi.h"
#include "HE_ParameterWidget_MultiInstance.h"

#include <QWidget>
#include <vector>
#include <string>
#include <HAPI/HAPI.h>
#endif

class QVBoxLayout;
class QGroupBox;
class QGridLayout;
class QLabel;
class QHBoxLayout;
class QScrollArea;

namespace HoudiniEngine
{
    class HE_Viewer : public QWidget 
    {
        Q_OBJECT

    public:
        HE_Viewer();
        ~HE_Viewer();

        void SetSession(const HAPI_Session *S);

        void SetNode(const HAPI_NodeId N);
        void ClearNode();

        void Refresh();

    protected:
        QVBoxLayout* MainLayout;

        QGroupBox* MainBox;
        QVBoxLayout* MainBoxLayout;

        QGroupBox* ParametersBox;
        QGridLayout* ParametersBoxLayout;
        QLabel* ParametersSelectedAssetLabel;
        QLabel* ParametersSelectedAssetName;

        QScrollArea* ParametersDetailScrollArea;
        QGroupBox* ParametersDetailBox;
        QGridLayout* ParametersDetailGridLayout;
        std::vector<QHBoxLayout*> ParametersDetailRows;

    private:
        const HAPI_Session* Session;
        HAPI_NodeId Node; 

        int RowCount;

        std::vector<HE_ParameterWidget*> Widgets;

        // Returns true if the set session is valid
        bool IsSessionValid();

        // Places the node name in Name
        // Return val: indicates whether the function succeeded
        bool GetNodeName(const HAPI_NodeInfo &NodeInfo, std::string &Name);

        // Places the string in HapiString
        // Return val: indicates whether the function succeeded
        bool GetHapiString(HAPI_StringHandle Handle, std::string& HapiString);

        // Places the node label in Label
        // Return val: indicates whether the function succeeded
        bool GetParameterLabel(const HAPI_ParmInfo &ParmInfo, std::string &Label);

        // Returns whether the parameter is a root level parameter
        bool IsParameterRootLevel(const HAPI_ParmInfo &ParmInfo);

        // Gets the 0-based index of a multi-parameter instance
        int GetMultiParmIndex(const HAPI_ParmInfo &ParmInfo);

        void AppendNewRow(HE_ParameterWidget* Widget);
        void AddWidgetToRow(HE_ParameterWidget* Widget, int Row);

        HE_ParameterWidget_Folder* CreateFolderWidget(const HAPI_ParmInfo &ParmInfo);
        HE_ParameterWidget_FolderList* CreateFolderListWidget(const HAPI_ParmInfo &ParmInfo);
        HE_ParameterWidget_Label* CreateLabelWidget(const HAPI_ParmInfo &ParmInfo);
        HE_ParameterWidget_Integer* CreateIntegerWidget(const HAPI_ParmInfo &ParmInfo);
        HE_ParameterWidget_Float* CreateFloatWidget(const HAPI_ParmInfo &ParmInfo);
        HE_ParameterWidget_String* CreateStringWidget(const HAPI_ParmInfo &ParmInfo);
        HE_ParameterWidget_Toggle* CreateToggleWidget(const HAPI_ParmInfo &Parminfo);
        HE_ParameterWidget_Button* CreateButtonWidget(const HAPI_ParmInfo &ParmInfo);
        HE_ParameterWidget_IntegerChoice* CreateIntegerChoiceWidget(const HAPI_ParmInfo &ParmInfo);
        HE_ParameterWidget_StringChoice* CreateStringChoiceWidget(const HAPI_ParmInfo &ParmInfo);
        HE_ParameterWidget_Multi* CreateMultiWidget(const HAPI_ParmInfo &ParmInfo);

    private slots:
        void Slot_IntegerParmUpdate(HAPI_ParmId ParmId, std::vector<int> Integers);
        void Slot_FloatParmUpdate(HAPI_ParmId ParmId, std::vector<float> Floats);
        void Slot_StringParmUpdate(HAPI_ParmId ParmId, std::vector<std::string> Strings);
        void Slot_ToggleParmUpdate(HAPI_ParmId ParmId, int On);
        void Slot_ButtonParmUpdate(HAPI_ParmId ParmId);
        void Slot_IntegerChoiceParmUpdate(HAPI_ParmId ParmId, int Choice);
        void Slot_StringChoiceParmUpdate(HAPI_ParmId ParmId, std::string Choice);
        void Slot_MultiParmUpdate();

    signals:
        void IntParmChanged(HAPI_ParmId, std::vector<int>);
        void FloatParmChanged(HAPI_ParmId, std::vector<float>);
        void StringParmChanged(HAPI_ParmId, std::vector<std::string>);
    };
}

#endif
