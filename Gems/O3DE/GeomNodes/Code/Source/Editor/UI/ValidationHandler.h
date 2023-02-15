#pragma once

#include <AzCore/std/containers/vector.h>

namespace GeomNodes
{
    // Forward Declare
    class PropertyFuncValLineEditCtrl;
    class PropertyFuncValBrowseEditCtrl;

    class ValidationHandler
    {
    public:
        //void AddValidatorCtrl(PropertyFuncValLineEditCtrl* ctrl);
        void AddValidatorCtrl(PropertyFuncValBrowseEditCtrl* ctrl);
        bool AllValid();
    private:
        //AZStd::vector<PropertyFuncValLineEditCtrl*> m_validators;
        AZStd::vector<PropertyFuncValBrowseEditCtrl*> m_browseEditValidators;
    };
} // namespace GeomNodes
