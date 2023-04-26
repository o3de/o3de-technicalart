/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "ValidationHandler.h"

//#include "PropertyFuncValLineEdit.h"
#include "PropertyFuncValBrowseEdit.h"

namespace GeomNodes
{
    void ValidationHandler::AddValidatorCtrl(PropertyFuncValBrowseEditCtrl* ctrl)
    {
        m_browseEditValidators.push_back(ctrl);
    }

    bool ValidationHandler::AllValid()
    {
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
