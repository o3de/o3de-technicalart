#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace HoudiniEngine
{
    class IHoudini;
    class IHoudiniAsset;
    class IHoudiniNode;
    class IHoudiniParameter;
    class IInputNodeManager;

    typedef AZStd::shared_ptr<IHoudini> HoudiniPtr;
    typedef AZStd::shared_ptr<IHoudiniAsset> HoudiniAssetPtr;
    typedef AZStd::shared_ptr<IHoudiniNode> HoudiniNodePtr;
    typedef AZStd::shared_ptr<IHoudiniParameter> HoudiniParameterPtr;
    typedef AZStd::shared_ptr<IInputNodeManager> InputNodeManagerPtr;
}
