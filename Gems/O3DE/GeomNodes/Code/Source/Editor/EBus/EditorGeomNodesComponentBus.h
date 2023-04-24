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
    //! Bus that EditorGeomNodesComponent handles like UI and parameter changes or sending an IPC message.
    class EditorGeomNodesComponentRequests : public AZ::ComponentBus
    {
    public:
        //! Toggles the state of the component's parameters. true if disabled(working in the background); false if enabled.
        virtual void SetWorkInProgress(bool flag) = 0;
        //! Gets the value of work in progress variable.
        virtual bool GetWorkInProgress() = 0;
        //! Sends an IPC message to the script running on the blender instance.
        virtual void SendIPCMsg(const AZStd::string& msg) = 0;
        //! Tells the component that a parameter value has changed. Usually tied to AZ::Edit::Attributes::ChangeNotify
		virtual void OnParamChange() = 0;

	protected:
		~EditorGeomNodesComponentRequests() = default;
    };

    using EditorGeomNodesComponentRequestBus = AZ::EBus<EditorGeomNodesComponentRequests>;
} // namespace GeomNodes