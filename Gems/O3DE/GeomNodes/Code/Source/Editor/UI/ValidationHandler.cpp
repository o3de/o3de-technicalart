#include "ValidationHandler.h"

//#include "PropertyFuncValLineEdit.h"
#include "PropertyFuncValBrowseEdit.h"

namespace GeomNodes
{
    /*void ValidationHandler::AddValidatorCtrl(PropertyFuncValLineEditCtrl* ctrl)
    {
        m_validators.push_back(ctrl);
    }*/

    void ValidationHandler::AddValidatorCtrl(PropertyFuncValBrowseEditCtrl* ctrl)
    {
        m_browseEditValidators.push_back(ctrl);
    }

    bool ValidationHandler::AllValid()
    {
        // for (PropertyFuncValLineEditCtrl* ctrl : m_validators)
        // {
        //     if (!ctrl->ValidateAndShowErrors())
        //     {
        //         return false;
        //     }
        // }

        for (PropertyFuncValBrowseEditCtrl* ctrl : m_browseEditValidators)
        {
            if (!ctrl->ValidateAndShowErrors())
            {
                return false;
            }
        }

        return true;
    }
} // namespace GeomNodes
