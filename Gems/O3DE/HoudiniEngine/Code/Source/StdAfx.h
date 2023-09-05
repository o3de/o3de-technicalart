#pragma once

#include <platform.h> // Many CryCommon files require that this is included first.
//#include <CryName.h>
#include <ISerialize.h>
#include <IGem.h>
#include <MathConversion.h>

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Debug/Profiler.h>
#ifdef HOUDINI_ENGINE_EDITOR
#include <EditorCoreAPI.h>

#include <AzCore/std/containers/stack.h>

//AZ CORE Includes
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Asset/AssetManagerBus.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/ComponentExport.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/EBus/EBus.h>
#include <AzCore/IO/FileIO.h>
#include <AzCore/IO/SystemFile.h>
#include <AzCore/Math/Color.h>
#include <AzCore/Math/MathUtils.h>
#include <AzCore/Math/Quaternion.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Math/Vector2.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Vector4.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/RTTI/TypeInfo.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Script/ScriptProperty.h>
#include <AzCore/Serialization/DataPatch.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Slice/SliceComponent.h>
#include <AzCore/std/string/conversions.h>
#include <AzCore/std/containers/map.h>
#include <AzCore/std/parallel/semaphore.h>
#include <AzCore/std/parallel/atomic.h>
#include <AzCore/std/smart_ptr/enable_shared_from_this.h>
#include <AzCore/XML/rapidxml.h>
#include <AzCore/XML/rapidxml_print.h>

//AZ FRAMEWORK INCLUDES
#include <AzFramework/Entity/EntityDebugDisplayBus.h>
#include <AzFramework/Entity/EntityContextBus.h>
#include <AzFramework/IO/LocalFileIO.h>
#include <AzFramework/Script/ScriptComponent.h>
#include <AzFramework/StringFunc/StringFunc.h>
#include <AzFramework/Asset/AssetCatalogBus.h>
#include <AzFramework/Asset/AssetSystemBus.h>
#include <AzFramework/Asset/SimpleAsset.h>
#include <AzFramework/Viewport/ViewportColors.h>

//LY CENTRAL GEM Includes:
//#include <LmbrCentral/Rendering/MeshComponentBus.h>
#include <LmbrCentral/Shape/SplineComponentBus.h>
//#include <LmbrCentral/Rendering/MaterialOwnerBus.h>
//#include <LmbrCentral/Rendering/MaterialAsset.h>
//#include <LmbrCentral/Rendering/MeshAsset.h>

//TOOLS INCLUDES
#include <AzToolsFramework/API/EditorAssetSystemAPI.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/API/ComponentEntityObjectBus.h>
#include <AzToolsFramework/API/ComponentEntitySelectionBus.h>
#include <AzToolsFramework/Entity/EditorEntityHelpers.h>
#include <AzToolsFramework/Entity/EditorEntityInfoBus.h>
#include <AzToolsFramework/Entity/EditorEntityContextBus.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzToolsFramework/ToolsComponents/EditorSelectionAccentSystemComponent.h>
#include <AzToolsFramework/ToolsComponents/EditorVisibilityBus.h>
#include <AzToolsFramework/ToolsComponents/TransformComponent.h>
// Qt
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <AzQtComponents/Utilities/DesktopUtilities.h>

#else
//Game Only Includes
#include <AzFramework/StringFunc/StringFunc.h>
#endif
