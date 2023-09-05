#include "StdAfx.h"
#include "HE_ParameterWidget_Folder.h"

#include <QtWidgets/qgridlayout.h>
#include <vector>

namespace HoudiniEngine
{
    HE_ParameterWidget_Folder::HE_ParameterWidget_Folder(HAPI_ParmId ParmId, const std::string &FolderName)
        : HE_ParameterWidget(ParmId)
    {
        Layout = new QGridLayout;
        Layout->setAlignment(Qt::AlignTop);

        this->setLayout(Layout);

        RowCount = 0;

        Name = FolderName;
    }

    HE_ParameterWidget_Folder::~HE_ParameterWidget_Folder()
    {
        for (std::vector<HE_ParameterWidget*>::size_type i = 0; i < ParameterWidgets.size(); i++)
        {
            if (ParameterWidgets[i])
            {
                delete ParameterWidgets[i];
            }
        }

        for (std::vector<QHBoxLayout*>::size_type i = 0; i < Rows.size(); i++)
        {
            if (Rows[i])
            {
                delete Rows[i];
            }
        }

        delete Layout;
    }

    void
    HE_ParameterWidget_Folder::AppendNewRow(HE_ParameterWidget* Widget)
    {
        ParameterWidgets.push_back(Widget);

        QHBoxLayout* NewRow = new QHBoxLayout;
        Rows.push_back(NewRow);

        NewRow->addWidget(Widget);

        Layout->addLayout(NewRow, RowCount, 0);

        RowCount++;
    }

    void
    HE_ParameterWidget_Folder::AddWidgetToRow(HE_ParameterWidget* Widget, int Row)
    {
        if (Row < RowCount)
        {
            ParameterWidgets.push_back(Widget);
            Rows[Row]->addWidget(Widget);
        }
    }

    std::string
    HE_ParameterWidget_Folder::GetName() const
    {
        return Name;
    }
}

#include "SideFX/moc_HE_ParameterWidget_Folder.cpp"
