#include "StdAfx.h"
#include "HE_ParameterWidget.h"

#include <HAPI/HAPI.h>
#include <string>

namespace HoudiniEngine
{
    HE_ParameterWidget::HE_ParameterWidget(HAPI_ParmId ParmId)
    {
        Id = ParmId;
    }

    HE_ParameterWidget::~HE_ParameterWidget()
    {

    }

    void
    HE_ParameterWidget::SetHelpToolTip(std::string HelpString)
    {
        this->setToolTip(HelpString.c_str());
    }
}

#include "SideFX/moc_HE_ParameterWidget.cpp"
