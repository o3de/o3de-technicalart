#ifndef H_ENGINE_PARAMETERWIDGET_FOLDERLIST
#define H_ENGINE_PARAMETERWIDGET_FOLDERLIST

#if !defined(Q_MOC_RUN)
#include "HE_ParameterWidget_Folder.h"

#include <vector>
#endif

class QVBoxLayout;
class QTabWidget;

namespace HoudiniEngine
{
    class HE_ParameterWidget_FolderList : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_FolderList() = delete;
        HE_ParameterWidget_FolderList(HAPI_ParmId ParmId);
        ~HE_ParameterWidget_FolderList();

        void AppendFolder(HE_ParameterWidget_Folder* Folder);

    private:
        QVBoxLayout* Layout;
        QTabWidget* FolderContainer;

        std::vector<HE_ParameterWidget_Folder*> Folders;
    };
}

#endif
