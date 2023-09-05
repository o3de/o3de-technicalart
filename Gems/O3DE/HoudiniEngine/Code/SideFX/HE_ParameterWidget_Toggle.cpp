#include "StdAfx.h"
#include "HE_ParameterWidget_Toggle.h"

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qlabel.h>
#include <string>

namespace HoudiniEngine
{
    HE_ParameterWidget_Toggle::HE_ParameterWidget_Toggle(HAPI_ParmId ParmId, const char *ParmLabel, bool Checked)
        : HE_ParameterWidget(ParmId)
    {
        Layout = new QHBoxLayout;
        Toggle = new QCheckBox;
        Label = new QLabel(ParmLabel);
        Spacer = new QSpacerItem(ToggleSpacerItemWidth, 0);

        Toggle->setChecked(Checked);

        Layout->setAlignment(Qt::AlignLeft);
        Layout->addSpacerItem(Spacer);
        Layout->addWidget(Toggle);
        Layout->addWidget(Label);

        this->setLayout(Layout);

        QObject::connect(Toggle, SIGNAL(stateChanged(int)),
                         this, SLOT(ToggleStateChanged(int)));
    }

    HE_ParameterWidget_Toggle::~HE_ParameterWidget_Toggle()
    {
        Layout->removeItem(Spacer);
        delete Spacer;
        delete Label;
        delete Toggle;
        delete Layout;
    }

    void
    HE_ParameterWidget_Toggle::SetHelpToolTip(std::string HelpString)
    {
        if (Label)
        {
            Label->setToolTip(HelpString.c_str());
        }
    }

    void
    HE_ParameterWidget_Toggle::ToggleStateChanged(int /*State*/)
    {
        int On = (Toggle->isChecked() ? 1 : 0);
        emit Signal_ToggleParmUpdate(Id, On);
    }
}

#include "SideFX/moc_HE_ParameterWidget_Toggle.cpp"
