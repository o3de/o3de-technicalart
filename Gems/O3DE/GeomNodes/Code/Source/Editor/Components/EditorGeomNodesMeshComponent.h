#pragma once
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzFramework/Visibility/BoundsBus.h>
#include <AzCore/Component/TransformBus.h>
#include <AzToolsFramework/API/ComponentEntitySelectionBus.h>
#include <Editor/EBus/EditorGeomNodesMeshComponentBus.h>

namespace GeomNodes
{
	class GNMeshData;
	class GNRenderMesh;
	class EditorGeomNodesMeshComponent
		: public AzToolsFramework::Components::EditorComponentBase
		, public AzFramework::BoundsRequestBus::Handler
		, public AzToolsFramework::EditorComponentSelectionRequestsBus::Handler
		, private AZ::TransformNotificationBus::Handler
		, private EditorGeomNodesMeshComponentEventBus::Handler
	{
	public:
		AZ_EDITOR_COMPONENT(EditorGeomNodesMeshComponent, "{B9B5FDEB-6D0F-432E-AE19-1F9EC41EF192}", EditorComponentBase);

		static void Reflect(AZ::ReflectContext* context);

		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

		EditorGeomNodesMeshComponent();
		virtual ~EditorGeomNodesMeshComponent();

		void Init() override;
		void Activate() override;
		void Deactivate() override;

		// EditorComponentSelectionRequestsBus overrides ...
		AZ::Aabb GetEditorSelectionBoundsViewport(const AzFramework::ViewportInfo& viewportInfo) override;
		bool EditorSelectionIntersectRayViewport(
			const AzFramework::ViewportInfo& viewportInfo, const AZ::Vector3& src, const AZ::Vector3& dir,
			float& distance) override;
		bool SupportsEditorRayIntersect() override;

		// BoundsRequestBus overrides ...
		AZ::Aabb GetWorldBounds() override;
		AZ::Aabb GetLocalBounds() override;

	private:
		// TransformNotificationBus overrides ...
		void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;

		// EditorGeomNodesMeshComponentEventBus overrides ...
		void OnMeshDataAssigned(const GNMeshData& meshData) override;

		void RebuildRenderMesh();

		AZ::EntityId m_entityId;
		AZ::EntityId m_parentId;

		GNMeshData m_meshData;

		AZStd::unique_ptr<GNRenderMesh> m_renderMesh;
		AZ::Transform m_worldFromLocal = AZ::Transform::CreateIdentity();

		AZStd::optional<AZ::Aabb> m_worldAabb; //!< Cached world aabb (used for selection/view determination).
		AZStd::optional<AZ::Aabb> m_localAabb; //!< Cached local aabb (used for center pivot calculation).
	};
} // namespace GeomNodes