#include "StdAfx.h"
#include "HE_ParameterWidget_String.h"

#include "HE_ParameterWidget.h"

#include <string>
#include <vector>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>

namespace HoudiniEngine
{
    HE_ParameterWidget_String::HE_ParameterWidget_String(HAPI_ParmId ParmId, const char *ParmLabel,
                                                            const std::vector<std::string>& Values, int ParmSize)
        : HE_ParameterWidget(ParmId)
    {
        Layout = new QGridLayout;
        Label = new QLabel(ParmLabel);

        Layout->setAlignment(Qt::AlignTop);
        Label->setAlignment(Qt::AlignCenter);
        Layout->setColumnMinimumWidth(0, 100);
        Layout->addWidget(Label, 0, 0);

        Strings.resize(ParmSize);
        for (int i = 0; i < ParmSize; i++)
        {
            Strings[i] = new QLineEdit(Values[i].c_str());
            Layout->addWidget(Strings[i], 0, i + 1);

            QObject::connect(Strings[i], SIGNAL(editingFinished()), this, SLOT(EditingFinished()));
        }

        this->setLayout(Layout);
    }

    HE_ParameterWidget_String::~HE_ParameterWidget_String()
    {
        for (int i = 0; i < Strings.size(); i++)
        {
            if (Strings[i])
            {
                delete Strings[i];
            }
        }

        delete Label;
        delete Layout;
    }

    void
    HE_ParameterWidget_String::SetHelpToolTip(std::string HelpString)
    {
        if (Label)
        {
            Label->setToolTip(HelpString.c_str());
        }
    }

    void
    HE_ParameterWidget_String::EditingFinished()
    {
        std::vector<std::string> StringVals;
        for (int i = 0; i < Strings.size(); i++)
        {
            StringVals.push_back(Strings[i]->text().toStdString());
        }
        emit Signal_StringParmUpdate(Id, StringVals);
    }
}

#include "SideFX/moc_HE_ParameterWidget_String.cpp"
