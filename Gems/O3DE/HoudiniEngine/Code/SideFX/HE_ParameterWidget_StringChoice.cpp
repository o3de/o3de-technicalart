#include "StdAfx.h"
#include "HE_ParameterWidget_StringChoice.h"

#include "HE_ParameterWidget.h"

#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qcombobox.h>

#include <string>
#include <vector>

namespace HoudiniEngine
{
    HE_ParameterWidget_StringChoice::HE_ParameterWidget_StringChoice(HAPI_ParmId ParmId, const char *ParmLabel,
                                                                     int ChoiceSize,
                                                                     const std::vector<std::string> &Choices,
                                                                     const std::vector<std::string> &ChoiceVals,
                                                                     std::string CurrentChoice)
        : HE_ParameterWidget(ParmId)
    {
        Layout = new QGridLayout;
        Label = new QLabel(ParmLabel);
        StringChoices = new QComboBox;

        Layout->setAlignment(Qt::AlignTop);
        Label->setAlignment(Qt::AlignCenter);
        Layout->addWidget(Label, 0, 0);
        Layout->addWidget(StringChoices, 0, 1);
        Layout->setColumnMinimumWidth(0, 100);

        this->setLayout(Layout);

        QStringList QChoices;
        int ChoiceIndex = 0;
        for (int i = 0; i < ChoiceSize; i++)
        {
            QChoices << Choices[i].c_str();
            if (ChoiceVals[i] == CurrentChoice)
            {
                ChoiceIndex = i;
            }

            Values.push_back(ChoiceVals[i]);
        }

        StringChoices->addItems(QChoices);

        StringChoices->setCurrentIndex(ChoiceIndex);

        QObject::connect(StringChoices, SIGNAL(currentIndexChanged(int)),
                         this, SLOT(CurrentIndexChanged(int)));
    }

    HE_ParameterWidget_StringChoice::~HE_ParameterWidget_StringChoice()
    {
        delete StringChoices;
        delete Label;
        delete Layout;
    }

    void
    HE_ParameterWidget_StringChoice::SetHelpToolTip(std::string HelpString)
    {
        if (Label)
        {
            Label->setToolTip(HelpString.c_str());
        }
    }

    void
    HE_ParameterWidget_StringChoice::CurrentIndexChanged(int Index)
    {
        emit Signal_StringChoiceParmUpdate(Id, Values[Index]);
    }
}

#include "SideFX/moc_HE_ParameterWidget_StringChoice.cpp"
