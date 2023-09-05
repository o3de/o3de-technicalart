#include "StdAfx.h"
#include "HE_Viewer.h"

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

#include <HAPI/HAPI.h>
#include <string>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qframe.h>
#include <iostream>
#include <unordered_map>

#include <iostream>

namespace HoudiniEngine
{
#pragma warning(push)
#pragma warning(disable: 4541)

    HE_Viewer::HE_Viewer()
    {
        // Create all of the UI elements
        MainLayout = new QVBoxLayout;

        MainBox = new QGroupBox("Parameters");
        MainBoxLayout = new QVBoxLayout;

        ParametersBox = new QGroupBox;
        ParametersBoxLayout = new QGridLayout;
        ParametersSelectedAssetLabel = new QLabel("Active Asset:");
        ParametersSelectedAssetName = new QLabel;

        ParametersDetailScrollArea = new QScrollArea;
        ParametersDetailBox = new QGroupBox;
        ParametersDetailGridLayout = new QGridLayout;

        // Align the UI elements
        MainBox->setAlignment(Qt::AlignTop);
        MainBoxLayout->setAlignment(Qt::AlignTop);
        ParametersBox->setAlignment(Qt::AlignTop);
        ParametersBoxLayout->setAlignment(Qt::AlignTop);
        ParametersDetailBox->setAlignment(Qt::AlignTop);
        ParametersDetailGridLayout->setAlignment(Qt::AlignTop);

        // Setup UI
        this->setLayout(MainLayout);

        MainBox->setLayout(MainBoxLayout);
        MainLayout->addWidget(MainBox);

        ParametersBoxLayout->addWidget(ParametersSelectedAssetLabel, 0, 0);
        ParametersBoxLayout->addWidget(ParametersSelectedAssetName, 0, 1);
        ParametersBox->setLayout(ParametersBoxLayout);
        MainBoxLayout->addWidget(ParametersBox);

        ParametersDetailBox->setLayout(ParametersDetailGridLayout);
        ParametersDetailScrollArea->setWidget(ParametersDetailBox);
        ParametersDetailScrollArea->setWidgetResizable(true);
        ParametersDetailScrollArea->setFrameShape(QFrame::NoFrame);
        MainBoxLayout->addWidget(ParametersDetailScrollArea);

        // Internal
        RowCount = 0;
    }

    HE_Viewer::~HE_Viewer()
    {
        delete ParametersDetailGridLayout;
        delete ParametersDetailBox;
        delete ParametersDetailScrollArea;

        delete ParametersSelectedAssetName;
        delete ParametersSelectedAssetLabel;
        delete ParametersBoxLayout;
        delete ParametersBox;

        delete MainBoxLayout;
        delete MainBox;

        delete MainLayout;
    }

    void
    HE_Viewer::SetSession(const HAPI_Session *S)
    {
        Session = S;
    }

    void
    HE_Viewer::SetNode(const HAPI_NodeId N)
    {
        Node = N;
    }

    void
    HE_Viewer::ClearNode()
    {
        Node = -1;
    }

    void
    HE_Viewer::Refresh()
    {
        // Clear any existing widgets
        for (int i = 0; i < Widgets.size(); i++)
        {
            if (Widgets[i])
            {
                Widgets[i]->deleteLater();
            }
        }

        Widgets.clear();

        if (IsSessionValid())
        {
            HAPI_NodeInfo NodeInfo;
            if (HAPI_GetNodeInfo(Session, Node, &NodeInfo) == HAPI_RESULT_SUCCESS)
            {
                std::string NodeName;
                if (GetNodeName(NodeInfo, NodeName))
                {
                    ParametersSelectedAssetName->setText(NodeName.c_str());

                    HAPI_ParmInfo *ParmInfos = new HAPI_ParmInfo[NodeInfo.parmCount];

                    if (HAPI_GetParameters(Session, Node, ParmInfos, 0, NodeInfo.parmCount) == HAPI_RESULT_SUCCESS)
                    {
                        std::unordered_map<HAPI_ParmId, HE_ParameterWidget_Multi*> MultiListCache;
                        std::unordered_map<HAPI_ParmId, HE_ParameterWidget_Folder*> FolderCache;
                        bool ParsingFolderList = false;
                        HE_ParameterWidget_FolderList* ActiveFolderList = nullptr;
                        int RemainingFolders = 0;

                        for (int p = 0; p < NodeInfo.parmCount; p++)
                        {
                            HE_ParameterWidget* WidgetToAdd = nullptr;
                            switch (ParmInfos[p].type)
                            {
                                case HAPI_PARMTYPE_FOLDERLIST:
                                {
                                    WidgetToAdd = CreateFolderListWidget(ParmInfos[p]);
                                    
                                    if (ParmInfos[p].size > 0)
                                    {
                                        ParsingFolderList = true;
                                        RemainingFolders = ParmInfos[p].size;
                                        ActiveFolderList = (HE_ParameterWidget_FolderList*)(WidgetToAdd);
                                    }
                                } break;
                                case HAPI_PARMTYPE_FOLDER:
                                {
                                    WidgetToAdd = CreateFolderWidget(ParmInfos[p]);
                                    FolderCache.insert({ ParmInfos[p].id, (HE_ParameterWidget_Folder*)(WidgetToAdd) });
                                } break;
                                case HAPI_PARMTYPE_LABEL:
                                {
                                    WidgetToAdd = CreateLabelWidget(ParmInfos[p]);
                                } break;
                                case HAPI_PARMTYPE_INT:
                                {
                                    if (ParmInfos[p].choiceCount > 0)
                                    {
                                        WidgetToAdd = CreateIntegerChoiceWidget(ParmInfos[p]);
                                    }
                                    else
                                    {
                                        WidgetToAdd = CreateIntegerWidget(ParmInfos[p]);
                                    }
                                } break;
                                case HAPI_PARMTYPE_FLOAT:
                                {
                                    WidgetToAdd = CreateFloatWidget(ParmInfos[p]);
                                } break;
                                case HAPI_PARMTYPE_STRING:
                                {
                                    if (ParmInfos[p].choiceCount > 0)
                                    {
                                        WidgetToAdd = CreateStringChoiceWidget(ParmInfos[p]);
                                    }
                                    else
                                    {
                                        WidgetToAdd = CreateStringWidget(ParmInfos[p]);
                                    }
                                } break;
                                case HAPI_PARMTYPE_TOGGLE:
                                {
                                    WidgetToAdd = CreateToggleWidget(ParmInfos[p]);
                                } break;
                                case HAPI_PARMTYPE_BUTTON:
                                {
                                    WidgetToAdd = CreateButtonWidget(ParmInfos[p]);
                                } break;
                                case HAPI_PARMTYPE_MULTIPARMLIST:
                                {
                                    WidgetToAdd = CreateMultiWidget(ParmInfos[p]);
                                    MultiListCache.insert({ ParmInfos[p].id, (HE_ParameterWidget_Multi*)(WidgetToAdd) });
                                } break;
                                default:
                                {
                                    WidgetToAdd = nullptr;
                                } break;
                            }

                            if (WidgetToAdd)
                            {
                                if (ParsingFolderList && ParmInfos[p].type != HAPI_PARMTYPE_FOLDERLIST)
                                {
                                    HE_ParameterWidget_Folder* Folder = (HE_ParameterWidget_Folder*)(WidgetToAdd);
                                    ActiveFolderList->AppendFolder(Folder);
                                    RemainingFolders--;

                                    if (RemainingFolders <= 0)
                                    {
                                        ParsingFolderList = false;
                                    }
                                }
                                else if (IsParameterRootLevel(ParmInfos[p]))
                                {
                                    AppendNewRow(WidgetToAdd);
                                }
                                else if (ParmInfos[p].isChildOfMultiParm)
                                {
                                    auto Search = MultiListCache.find(ParmInfos[p].parentId);
                                    if (Search != MultiListCache.end())
                                    {
                                        HE_ParameterWidget_Multi* MultiParm = Search->second;
                                        HE_ParameterWidget_MultiInstance* Instance = MultiParm->GetInstance(GetMultiParmIndex(ParmInfos[p]));

                                        if (Instance)
                                        {
                                            Instance->AddParameter(WidgetToAdd);
                                        }
                                        else
                                        {
                                            HE_ParameterWidget_MultiInstance *NewInstance = new HE_ParameterWidget_MultiInstance(ParmInfos[p].id,
                                                                                                                                 ParmInfos[p].parentId);
                                            NewInstance->AddParameter(WidgetToAdd);
                                            MultiParm->AppendInstance(NewInstance);
                                        }
                                    }
                                }
                                else
                                {
                                    auto FolderSearch = FolderCache.find(ParmInfos[p].parentId);
                                    if (FolderSearch != FolderCache.end())
                                    {
                                        HE_ParameterWidget_Folder* Folder = FolderSearch->second;
                                        Folder->AppendNewRow(WidgetToAdd);
                                    }
                                }

                                if (ParmInfos[p].invisible)
                                {
                                    WidgetToAdd->hide();
                                }

                                if (ParmInfos[p].disabled)
                                {
                                    WidgetToAdd->setEnabled(false);                                    
                                }
                            }
                        }
                    }

                    delete[] ParmInfos;
                }
            }
        }
    }

    bool
    HE_Viewer::IsSessionValid()
    {
        if (HAPI_IsSessionValid(Session) == HAPI_RESULT_SUCCESS)
        {
            return true;
        }

        return false;
    }

    bool
    HE_Viewer::GetHapiString(HAPI_StringHandle Handle, std::string& HapiString)
    {
        int BufSize = 0;
        if (HAPI_GetStringBufLength(Session, Handle, &BufSize) == HAPI_RESULT_SUCCESS)
        {
            char *Val = new char[BufSize];
            if (HAPI_GetString(Session, Handle, Val, BufSize) == HAPI_RESULT_SUCCESS)
            {
                HapiString = Val;
                delete[] Val;
                return true;
            }
            delete[] Val;
        }

        return false;
    }

    bool
    HE_Viewer::GetNodeName(const HAPI_NodeInfo &NodeInfo, std::string &Name)
    {
        return GetHapiString(NodeInfo.nameSH, Name);
    }

    bool
    HE_Viewer::GetParameterLabel(const HAPI_ParmInfo &ParmInfo, std::string &Label)
    {
        return GetHapiString(ParmInfo.labelSH, Label);
    }

    bool
    HE_Viewer::IsParameterRootLevel(const HAPI_ParmInfo &ParmInfo)
    {
        if (ParmInfo.parentId < 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    int
    HE_Viewer::GetMultiParmIndex(const HAPI_ParmInfo &ParmInfo)
    {
        int InstanceNum = ParmInfo.instanceNum;

        return (ParmInfo.instanceStartOffset ? (InstanceNum - 1) : InstanceNum);
    }

    void
    HE_Viewer::AppendNewRow(HE_ParameterWidget *Widget)
    {
        Widgets.push_back(Widget);    

        QHBoxLayout *NewRow = new QHBoxLayout;
        ParametersDetailRows.push_back(NewRow);

        ParametersDetailGridLayout->addLayout(NewRow, RowCount, 0);
        NewRow->addWidget(Widget);

        RowCount++;
    }

    void
    HE_Viewer::AddWidgetToRow(HE_ParameterWidget *Widget, int Row)
    {
        if (Row < RowCount)
        {
            Widgets.push_back(Widget);
            ParametersDetailRows[Row]->addWidget(Widget);
        }
    }

    HE_ParameterWidget_Folder*
    HE_Viewer::CreateFolderWidget(const HAPI_ParmInfo &ParmInfo)
    {
        std::string Label;
        if (GetParameterLabel(ParmInfo, Label))
        {
            return new HE_ParameterWidget_Folder(ParmInfo.id, Label.c_str());
        }
        else
        {
            return nullptr;
        }
    }

    HE_ParameterWidget_FolderList*
    HE_Viewer::CreateFolderListWidget(const HAPI_ParmInfo &ParmInfo)
    {
        return new HE_ParameterWidget_FolderList(ParmInfo.id); 
    }

    HE_ParameterWidget_Label*
    HE_Viewer::CreateLabelWidget(const HAPI_ParmInfo &ParmInfo)
    {
        std::string Label;
        if (GetParameterLabel(ParmInfo, Label))
        {
            return new HE_ParameterWidget_Label(ParmInfo.id, Label.c_str());
        }
        else
        {
            return nullptr;
        }
    }

    HE_ParameterWidget_Integer*
    HE_Viewer::CreateIntegerWidget(const HAPI_ParmInfo &ParmInfo)
    {
        std::string Label;
        if (GetParameterLabel(ParmInfo, Label))
        {
            std::vector<int> ParmIntValues(ParmInfo.size);
            
            if (HAPI_GetParmIntValues(Session, Node, &ParmIntValues.front(), ParmInfo.intValuesIndex, ParmInfo.size) == HAPI_RESULT_SUCCESS)
            {
                HE_ParameterWidget_Integer *Widget;
                if (ParmInfo.size == 1 && ParmInfo.hasUIMax && ParmInfo.hasUIMin)
                {
                    Widget = new HE_ParameterWidget_Integer(ParmInfo.id, Label.c_str(), ParmIntValues[0],
                                                            ParmInfo.UIMin, ParmInfo.UIMax);
                }
                else
                {
                    Widget = new HE_ParameterWidget_Integer(ParmInfo.id, Label.c_str(), ParmIntValues, ParmInfo.size);
                }

                QObject::connect(Widget, SIGNAL(Signal_IntegerParmUpdate(HAPI_ParmId, std::vector<int>)),
                                 this, SLOT(Slot_IntegerParmUpdate(HAPI_ParmId, std::vector<int>)));

                return Widget;
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }

    HE_ParameterWidget_Float*
    HE_Viewer::CreateFloatWidget(const HAPI_ParmInfo &ParmInfo)
    {
        std::string Label;
        if (GetParameterLabel(ParmInfo, Label))
        {
            std::vector<float> ParmFloatValues(ParmInfo.size);

            if (HAPI_GetParmFloatValues(Session, Node, &ParmFloatValues.front(), ParmInfo.floatValuesIndex, ParmInfo.size) == HAPI_RESULT_SUCCESS)
            {
                HE_ParameterWidget_Float* Widget;
                if (ParmInfo.size == 1 && ParmInfo.hasUIMax && ParmInfo.hasUIMin)
                {
                    Widget = new HE_ParameterWidget_Float(ParmInfo.id, Label.c_str(), ParmFloatValues[0], ParmInfo.UIMin, ParmInfo.UIMax);
                }
                else
                {
                    Widget = new HE_ParameterWidget_Float(ParmInfo.id, Label.c_str(), ParmFloatValues, ParmInfo.size);
                }

                QObject::connect(Widget, SIGNAL(Signal_FloatParmUpdate(HAPI_ParmId, std::vector<float>)),
                                 this, SLOT(Slot_FloatParmUpdate(HAPI_ParmId, std::vector<float>)));
                
                return Widget;
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }

    HE_ParameterWidget_String*
    HE_Viewer::CreateStringWidget(const HAPI_ParmInfo &ParmInfo)
    {
        std::string Label;
        if (GetParameterLabel(ParmInfo, Label))
        {
            std::vector<HAPI_StringHandle> StringHandles(ParmInfo.size);
            if (HAPI_GetParmStringValues(Session, Node, true, &StringHandles.front(), ParmInfo.stringValuesIndex, ParmInfo.size) == HAPI_RESULT_SUCCESS)
            {
                std::vector<std::string> ParmStrings(ParmInfo.size);
                for (int i = 0; i < StringHandles.size(); i++)
                {
                    GetHapiString(StringHandles[i], ParmStrings[i]); 
                }
                
                HE_ParameterWidget_String *Widget = new HE_ParameterWidget_String(ParmInfo.id, Label.c_str(), ParmStrings, ParmInfo.size);

                QObject::connect(Widget, SIGNAL(Signal_StringParmUpdate(HAPI_ParmId, std::vector<std::string>)),
                                 this, SLOT(Slot_StringParmUpdate(HAPI_ParmId, std::vector<std::string>)));

                return Widget;
            }
        }

        return nullptr;
    }

    HE_ParameterWidget_Toggle*
    HE_Viewer::CreateToggleWidget(const HAPI_ParmInfo &ParmInfo)
    {
        std::string Label;
        if (GetParameterLabel(ParmInfo, Label))
        {
            std::vector<int> ParmIntVal(ParmInfo.size);
            if (HAPI_GetParmIntValues(Session, Node, &ParmIntVal.front(), ParmInfo.intValuesIndex, ParmInfo.size) == HAPI_RESULT_SUCCESS)
            {
                HE_ParameterWidget_Toggle *Widget = new HE_ParameterWidget_Toggle(ParmInfo.id, Label.c_str(), ParmIntVal[0]);

                QObject::connect(Widget, SIGNAL(Signal_ToggleParmUpdate(HAPI_ParmId, int)),
                                 this, SLOT(Slot_ToggleParmUpdate(HAPI_ParmId, int)));

                return Widget;
            }
        }

        return nullptr;
    }

    HE_ParameterWidget_Button*
    HE_Viewer::CreateButtonWidget(const HAPI_ParmInfo &ParmInfo)
    {
        std::string Label;
        if (GetParameterLabel(ParmInfo, Label))
        {
            HE_ParameterWidget_Button *Widget = new HE_ParameterWidget_Button(ParmInfo.id, Label.c_str());

            QObject::connect(Widget, SIGNAL(Signal_ButtonParmUpdate(HAPI_ParmId)),
                             this, SLOT(Slot_ButtonParmUpdate(HAPI_ParmId)));

            return Widget;
        }

        return nullptr;
    }

    HE_ParameterWidget_IntegerChoice*
    HE_Viewer::CreateIntegerChoiceWidget(const HAPI_ParmInfo &ParmInfo)
    {
        std::string Label;
        if (GetParameterLabel(ParmInfo, Label))
        {
            HAPI_ParmChoiceInfo* ChoiceInfos = new HAPI_ParmChoiceInfo[ParmInfo.choiceCount];
            std::vector<std::string> ChoiceLabels(ParmInfo.choiceCount);
            if (HAPI_GetParmChoiceLists(Session, Node, ChoiceInfos, ParmInfo.choiceIndex, ParmInfo.choiceCount) == HAPI_RESULT_SUCCESS)
            {
                for (int i = 0; i < ParmInfo.choiceCount; i++)
                {
                    GetHapiString(ChoiceInfos[i].labelSH, ChoiceLabels[i]);
                }

                std::vector<int> CurrentChoice(1);
                if (HAPI_GetParmIntValues(Session, Node, &CurrentChoice.front(), ParmInfo.intValuesIndex, 1) == HAPI_RESULT_SUCCESS)
                {
                    delete[] ChoiceInfos;
                    HE_ParameterWidget_IntegerChoice *Widget =
                        new HE_ParameterWidget_IntegerChoice(ParmInfo.id,
                                                             Label.c_str(),
                                                             ParmInfo.choiceCount,
                                                             ChoiceLabels,
                                                             CurrentChoice[0]);
                    QObject::connect(Widget, SIGNAL(Signal_IntegerChoiceParmUpdate(HAPI_ParmId, int)),
                                     this, SLOT(Slot_IntegerChoiceParmUpdate(HAPI_ParmId, int)));

                    return Widget;
                }
            }

            delete[] ChoiceInfos;
        }

        return nullptr;
    }

    HE_ParameterWidget_StringChoice*
    HE_Viewer::CreateStringChoiceWidget(const HAPI_ParmInfo &ParmInfo)
    {
        std::string Label;
        if (GetParameterLabel(ParmInfo, Label))
        {
            HAPI_ParmChoiceInfo* ChoiceInfos = new HAPI_ParmChoiceInfo[ParmInfo.choiceCount];
            std::vector<std::string> ChoiceLabels(ParmInfo.choiceCount);
            std::vector<std::string> ChoiceValues(ParmInfo.choiceCount);
            if (HAPI_GetParmChoiceLists(Session, Node, ChoiceInfos, ParmInfo.choiceIndex, ParmInfo.choiceCount) == HAPI_RESULT_SUCCESS)
            {
                for (int i = 0; i < ParmInfo.choiceCount; i++)
                {
                    GetHapiString(ChoiceInfos[i].labelSH, ChoiceLabels[i]);
                    GetHapiString(ChoiceInfos[i].valueSH, ChoiceValues[i]);
                }

                std::vector<HAPI_StringHandle> CurrentChoice(1);
                if (HAPI_GetParmStringValues(Session, Node, true, &CurrentChoice.front(), ParmInfo.stringValuesIndex, 1) == HAPI_RESULT_SUCCESS)
                {
                    std::string Selection;
                    GetHapiString(CurrentChoice[0], Selection);

                    delete[] ChoiceInfos;
                    HE_ParameterWidget_StringChoice *Widget =
                        new HE_ParameterWidget_StringChoice(ParmInfo.id,
                                                            Label.c_str(),
                                                            ParmInfo.choiceCount,
                                                            ChoiceLabels,
                                                            ChoiceValues,
                                                            Selection);

                    QObject::connect(Widget, SIGNAL(Signal_StringChoiceParmUpdate(HAPI_ParmId, std::string)),
                                     this, SLOT(Slot_StringChoiceParmUpdate(HAPI_ParmId, std::string)));

                    return Widget;
                }
            }
            delete[] ChoiceInfos;
        }

        return nullptr;
    }

    HE_ParameterWidget_Multi*
    HE_Viewer::CreateMultiWidget(const HAPI_ParmInfo &ParmInfo)
    {
        std::string Label;
        if (GetParameterLabel(ParmInfo, Label))
        {
            return new HE_ParameterWidget_Multi(ParmInfo.id, Label.c_str(), ParmInfo.instanceCount); 
        }

        return nullptr;
    }

    void
    HE_Viewer::Slot_IntegerParmUpdate(HAPI_ParmId ParmId, std::vector<int> Integers)
    {
        emit IntParmChanged(ParmId, Integers);
    }

    void
    HE_Viewer::Slot_FloatParmUpdate(HAPI_ParmId ParmId, std::vector<float> Floats)
    {
        emit FloatParmChanged(ParmId, Floats);
    }

    void
    HE_Viewer::Slot_StringParmUpdate(HAPI_ParmId ParmId, std::vector<std::string> Strings)
    {
        emit StringParmChanged(ParmId, Strings);
    }

    void
    HE_Viewer::Slot_ToggleParmUpdate(HAPI_ParmId ParmId, int On)
    {
        std::vector<int> ParmVal(1);
        ParmVal[0] = On;
        emit IntParmChanged(ParmId, ParmVal);
    }

    void
    HE_Viewer::Slot_ButtonParmUpdate(HAPI_ParmId ParmId)
    {
        std::vector<int> ParmVal(1);
        ParmVal[0] = 1;
        emit IntParmChanged(ParmId, ParmVal);
    }

    void
    HE_Viewer::Slot_IntegerChoiceParmUpdate(HAPI_ParmId ParmId, int Choice)
    {
        std::vector<int> ParmVal(1);
        ParmVal[0] = Choice;
        emit IntParmChanged(ParmId, ParmVal);
    }

    void
    HE_Viewer::Slot_StringChoiceParmUpdate(HAPI_ParmId ParmId, std::string Choice)
    {
        std::vector<std::string> ParmVal(1);
        ParmVal[0] = Choice;
        emit StringParmChanged(ParmId, ParmVal);
    }

    void
    HE_Viewer::Slot_MultiParmUpdate()
    {

    }
#pragma warning(pop)

}

#include "SideFX/moc_HE_Viewer.cpp"
