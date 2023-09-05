#include "StdAfx.h"
#include "HE_ParameterWidget_Multi.h"

#include "HE_ParameterWidget.h"
#include "HE_ParameterWidget_MultiInstance.h"

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>

namespace HoudiniEngine
{
    HE_ParameterWidget_Multi::HE_ParameterWidget_Multi(HAPI_ParmId ParmId, const char *Label, int InstanceCount)
        : HE_ParameterWidget(ParmId)
    {
        NumOfInstances = InstanceCount;

        Layout = new QVBoxLayout;
        ControlWidget = new QWidget;
        ControlLayout = new QHBoxLayout;
        ParameterLabel = new QLabel(Label);
        InstanceCountEdit = new QLineEdit;
        ClearButton = new QPushButton("Clear");
        AddInstanceButton = new QPushButton("+");
        RemoveInstanceButton = new QPushButton("-");

        Layout->setAlignment(Qt::AlignTop);
        InstanceCountEdit->setText(std::to_string(NumOfInstances).c_str());

        ControlLayout->addWidget(ParameterLabel);
        ControlLayout->addWidget(InstanceCountEdit);
        ControlLayout->addWidget(AddInstanceButton);
        ControlLayout->addWidget(RemoveInstanceButton);
        ControlLayout->addWidget(ClearButton);

        ControlWidget->setLayout(ControlLayout);

        Layout->addWidget(ControlWidget);

        this->setLayout(Layout);

        QObject::connect(InstanceCountEdit, SIGNAL(returnPressed()), this, SLOT(InstanceCountEdit_returnPressed()));
        QObject::connect(AddInstanceButton, SIGNAL(clicked()), this, SLOT(AddInstanceButton_clicked()));
        QObject::connect(RemoveInstanceButton, SIGNAL(clicked()), this, SLOT(RemoveInstanceButton_clicked()));
        QObject::connect(ClearButton, SIGNAL(clicked()), this, SLOT(ClearButton_clicked()));
    }

    HE_ParameterWidget_Multi::~HE_ParameterWidget_Multi()
    {
        delete RemoveInstanceButton;
        delete AddInstanceButton;
        delete ClearButton;
        delete InstanceCountEdit;
        delete ParameterLabel;
        delete ControlLayout;
        delete ControlWidget;
        delete Layout;
    }

    void
    HE_ParameterWidget_Multi::AppendInstance(HE_ParameterWidget_MultiInstance* Instance)
    {
        Instances.push_back(Instance);
        Layout->addWidget(Instance);
    }

    HE_ParameterWidget_MultiInstance*
    HE_ParameterWidget_Multi::GetInstance(int Index)
    {
        if (Index < Instances.size())
        {
            return Instances[Index];
        }
        else
        {
            return nullptr;
        }
    }

    void
    HE_ParameterWidget_Multi::InstanceCountEdit_returnPressed()
    {

    }

    void
    HE_ParameterWidget_Multi::AddInstanceButton_clicked()
    {

    }

    void
    HE_ParameterWidget_Multi::RemoveInstanceButton_clicked()
    {

    }

    void
    HE_ParameterWidget_Multi::ClearButton_clicked()
    {

    }
}

#include "SideFX/moc_HE_ParameterWidget_Multi.cpp"
