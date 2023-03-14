#include "EditorGeomNodesMeshComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include <Editor/Rendering/GNMeshData.h>
#include <AzCore/Debug/Profiler.h>
#include <AzCore/Component/NonUniformScaleBus.h>
#include <Editor/Rendering/GNRenderMesh.h>

namespace GeomNodes
{
	EditorGeomNodesMeshComponent::EditorGeomNodesMeshComponent()
	{
	}

	EditorGeomNodesMeshComponent::~EditorGeomNodesMeshComponent()
	{
	}

	void EditorGeomNodesMeshComponent::Reflect(AZ::ReflectContext* context)
	{
		if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
		{
			serializeContext->Class<EditorGeomNodesMeshComponent, EditorComponentBase>()
				->Version(1);

			AZ::EditContext* ec = serializeContext->GetEditContext();
			if (ec)
			{
				ec->Class<EditorGeomNodesMeshComponent>(
					"Geometry Node Mesh",
					"The Geometry Node Mesh component is equivalent to a normal Mesh component but the model data comes from blender. ")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::Category, "Blender")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("UI", 0x27ff46b0))
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Level", 0x9aeacc13))
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Layer", 0xe4db211a))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true);
			}
		}
	}

	void EditorGeomNodesMeshComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
	{
		dependent.push_back(AZ_CRC("TransformService", 0x8ee22c50));
	}

	void EditorGeomNodesMeshComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
	{
		provided.push_back(AZ_CRC_CE("EditorGeomNodesMeshService"));
		provided.push_back(AZ_CRC_CE("MaterialConsumerService"));
	}

	void EditorGeomNodesMeshComponent::GetIncompatibleServices(
		[[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
	{
		incompatible.push_back(AZ_CRC("EditorGeomNodesMeshService"));
	}

	void EditorGeomNodesMeshComponent::Init()
	{
		if (m_renderMesh)
		{
			return;
		}

		m_entityId = GetEntityId();
		m_renderMesh = AZStd::make_unique<GNRenderMesh>(m_entityId);
	}

	void EditorGeomNodesMeshComponent::Activate()
	{
		AzToolsFramework::Components::EditorComponentBase::Activate();
		AZ::TransformNotificationBus::Handler::BusConnect(m_entityId);
		EditorGeomNodesMeshComponentEventBus::Handler::BusConnect(m_entityId);

		AZ::TransformBus::EventResult(m_parentId, m_entityId, &AZ::TransformBus::Events::GetParentId);

		
		m_worldFromLocal = AZ::Transform::CreateIdentity();
		AZ::TransformBus::EventResult(m_worldFromLocal, m_entityId, &AZ::TransformBus::Events::GetWorldTM);
	}

	void EditorGeomNodesMeshComponent::Deactivate()
	{
		AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusDisconnect();
		AzFramework::BoundsRequestBus::Handler::BusDisconnect();
		EditorGeomNodesMeshComponentEventBus::Handler::BusDisconnect();
		AZ::TransformNotificationBus::Handler::BusDisconnect();
		AzToolsFramework::Components::EditorComponentBase::Deactivate();
		m_renderMesh.reset();
	}

	AZ::Aabb EditorGeomNodesMeshComponent::GetEditorSelectionBoundsViewport([[maybe_unused]] const AzFramework::ViewportInfo& viewportInfo)
	{
		return GetWorldBounds();
	}

	bool EditorGeomNodesMeshComponent::EditorSelectionIntersectRayViewport(const AzFramework::ViewportInfo& /*viewportInfo*/, const AZ::Vector3& src, const AZ::Vector3& dir, float& distance)
	{
		AZ_PROFILE_FUNCTION(AzToolsFramework);

		if (!m_renderMesh->GetModel())
		{
			return false;
		}

		AZ::Transform transform = AZ::Transform::CreateIdentity();
		AZ::TransformBus::EventResult(transform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

		AZ::Vector3 nonUniformScale = AZ::Vector3::CreateOne();
		AZ::NonUniformScaleRequestBus::EventResult(nonUniformScale, m_entityId, &AZ::NonUniformScaleRequests::GetScale);

		float t;
		AZ::Vector3 ignoreNormal;
		constexpr float rayLength = 1000.0f;
		if (m_renderMesh->GetModel()->RayIntersection(transform, nonUniformScale, src, dir * rayLength, t, ignoreNormal))
		{
			distance = rayLength * t;
			return true;
		}

		return false;
	}

	bool EditorGeomNodesMeshComponent::SupportsEditorRayIntersect()
	{
		return true;
	}

	AZ::Aabb EditorGeomNodesMeshComponent::GetWorldBounds()
	{
		AZ_PROFILE_FUNCTION(GeomNodes);

		if (!m_worldAabb.has_value())
		{
			m_worldAabb = GetLocalBounds();
			m_worldAabb->ApplyTransform(m_worldFromLocal);
		}

		return m_worldAabb.value();
	}

	AZ::Aabb EditorGeomNodesMeshComponent::GetLocalBounds()
	{
		AZ_PROFILE_FUNCTION(GeomNodes);

		if (!m_localAabb.has_value() && m_meshData.VertexCount() > 0)
		{
			m_localAabb = m_meshData.GetAabb();
		}

		return m_localAabb.value();
	}
	
	void EditorGeomNodesMeshComponent::OnTransformChanged([[maybe_unused]] const AZ::Transform& local, const AZ::Transform& world)
	{
		AZ_PROFILE_FUNCTION(GeomNodes);

		m_worldAabb.reset();
		m_localAabb.reset();

		m_worldFromLocal = world;

		if (m_renderMesh)
		{
			m_renderMesh->UpdateTransform(world);
		}
	}

	void EditorGeomNodesMeshComponent::RebuildRenderMesh()
	{
		AZ_PROFILE_FUNCTION(GeomNodes);

		m_worldAabb.reset();
		m_localAabb.reset();

		// EditorGeomNodesComponentRequestBus::EventResult(m_meshData, m_parentId, &EditorGeomNodesComponentRequests::GetMeshData, (AZ::u64)m_entityId);

		if (m_meshData.VertexCount() > 0)
		{
			m_renderMesh->BuildMesh(m_meshData, m_worldFromLocal);
			m_renderMesh->UpdateTransform(m_worldFromLocal);
		}
	}
	
	void EditorGeomNodesMeshComponent::OnMeshDataAssigned(const GNMeshData& meshData)
	{
		m_meshData = meshData;
		AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusDisconnect();
		AzFramework::BoundsRequestBus::Handler::BusDisconnect();
		RebuildRenderMesh();
		AzFramework::BoundsRequestBus::Handler::BusConnect(m_entityId);
		AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusConnect(m_entityId);

	}
} // namespace GeomNodes