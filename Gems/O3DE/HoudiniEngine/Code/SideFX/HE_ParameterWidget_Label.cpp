#include "StdAfx.h"
#include "HE_ParameterWidget_Label.h"

#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>

namespace HoudiniEngine
{
    HE_ParameterWidget_Label::HE_ParameterWidget_Label(HAPI_ParmId ParmId, const char *LabelText)
        : HE_ParameterWidget(ParmId)
    {
        Layout = new QGridLayout;
        Label = new QLabel(LabelText);
        Layout->setAlignment(Qt::AlignTop);
        Layout->addWidget(Label, 0, 0);
        this->setLayout(Layout);
    }

    HE_ParameterWidget_Label::~HE_ParameterWidget_Label()
    {
        delete Label;
        delete Layout;
    }

    void
    HE_ParameterWidget_Label::SetHelpToolTip(std::string HelpString)
    {
        if (Label)
        {
            Label->setToolTip(HelpString.c_str());
        }
    }
}

#include "SideFX/moc_HE_ParameterWidget_Label.cpp"
