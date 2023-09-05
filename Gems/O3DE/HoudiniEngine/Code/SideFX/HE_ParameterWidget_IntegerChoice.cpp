#include "StdAfx.h"
#include "HE_ParameterWidget_IntegerChoice.h"

#include "HE_ParameterWidget.h"

#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qcombobox.h>
#include <string>
#include <vector>

namespace HoudiniEngine
{
    HE_ParameterWidget_IntegerChoice::HE_ParameterWidget_IntegerChoice(HAPI_ParmId ParmId, const char *ParmLabel,
                                                                        int ChoiceSize,
                                                                        const std::vector<std::string> &Choices,
                                                                        int CurrentChoice)
        : HE_ParameterWidget(ParmId)
    {
        Layout = new QGridLayout;
        Label = new QLabel(ParmLabel);
        IntChoices = new QComboBox;

        Layout->setAlignment(Qt::AlignTop);
        Label->setAlignment(Qt::AlignCenter);
        Layout->addWidget(Label, 0, 0);
        Layout->addWidget(IntChoices, 0, 1);
        Layout->setColumnMinimumWidth(0, 100);

        this->setLayout(Layout);

        QStringList QChoices;
        for (int i = 0; i < ChoiceSize; i++)
        {
            QChoices << Choices[i].c_str();
        }

        IntChoices->addItems(QChoices);
        IntChoices->setCurrentIndex(CurrentChoice);

        QObject::connect(IntChoices, SIGNAL(currentIndexChanged(int)),
                         this, SLOT(CurrentIndexChanged(int)));
    }

    HE_ParameterWidget_IntegerChoice::~HE_ParameterWidget_IntegerChoice()
    {
        delete IntChoices;
        delete Label;
        delete Layout;
    }

    void
    HE_ParameterWidget_IntegerChoice::SetHelpToolTip(std::string HelpString)
    {
        if (Label)
        {
            Label->setToolTip(HelpString.c_str());
        }
    }

    void
    HE_ParameterWidget_IntegerChoice::CurrentIndexChanged(int Index)
    {
       emit Signal_IntegerChoiceParmUpdate(Id, Index); 
    }
}

#include "SideFX/moc_HE_ParameterWidget_IntegerChoice.cpp"
