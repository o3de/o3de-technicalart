/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

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
        void AddValidatorCtrl(PropertyFuncValBrowseEditCtrl* ctrl);
        bool AllValid();

    private:
        AZStd::vector<PropertyFuncValBrowseEditCtrl*> m_browseEditValidators;
    };
} // namespace GeomNodes
