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

	protected:
		~EditorGeomNodesComponentRequests() = default;
    };

    using EditorGeomNodesComponentRequestBus = AZ::EBus<EditorGeomNodesComponentRequests>;
} // namespace GeomNodes