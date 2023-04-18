/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace GeomNodes
{
    
    class EditorGeomNodesComponentRequests : public AZ::ComponentBus
    {
    public:
        virtual void SetWorkInProgress(bool flag) = 0;
        virtual bool GetWorkInProgress() = 0;
        virtual void SendIPCMsg(const AZStd::string& msg) = 0;
		virtual void OnParamChange() = 0;

	protected:
		~EditorGeomNodesComponentRequests() = default;
    };

    using EditorGeomNodesComponentRequestBus = AZ::EBus<EditorGeomNodesComponentRequests>;
} // namespace GeomNodes