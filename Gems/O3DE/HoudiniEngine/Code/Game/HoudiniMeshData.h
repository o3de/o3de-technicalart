#pragma once
#include <AzCore/Component/Component.h>
#include <AzCore/Math/Aabb.h>
#include <AzFramework/Asset/SimpleAsset.h>
//#include <LmbrCentral/Rendering/MaterialAsset.h>
#include <Math/MathHelper.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
#include <HoudiniConstants.h>

namespace HoudiniEngine
{
    using MaterialList = AZStd::vector<AZStd::string>;

    class HoudiniMaterialSettings
    {
    public:
        bool m_dirty = true;
        bool m_dirtyPhysics = true;

        int m_subMaterial = 0;
        bool m_visible = true;
        bool m_physics = false;
        bool m_highPriorityTextureStreaming = false;
        float m_maxViewDistance = 8000;
        AZStd::string m_materialName;
        //AzFramework::SimpleAssetReference<LmbrCentral::MaterialAsset> m_materialAsset;

        void*/*IMaterialRef*/ LoadMaterial()
        {
            // ATOMCONVERT
            /*AZStd::string materialPath = m_materialAsset.GetAssetPath();
            if (materialPath.empty() == false)
            {
                IMaterialRef mat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial(materialPath.c_str(), false);

                if (mat != nullptr && m_subMaterial != 0 && m_subMaterial <= mat->GetSubMtlCount())
                {
                    mat = mat->GetSubMtl(m_subMaterial);
                }

                return mat;
            }
            */
            return nullptr;
        }

        int GetSubMaterialCount()
        {
            // ATOMCONVERT
            /*
            AZStd::string materialPath = m_materialAsset.GetAssetPath();
            if (materialPath.empty() == false)
            {
                IMaterialRef mat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial(materialPath.c_str(), false);

                if (mat != nullptr)
                {
                    return mat->GetSubMtlCount();
                }
            }
            */
            return 0;
        }

        void OnChanged() { m_dirty = true; }
        void OnPhysicsChanged() 
        {
            m_dirty = true; 
            m_dirtyPhysics = true;
        }

        float GetMaxViewDistance() 
        {
            return m_maxViewDistance;
        }

        void SetMaxViewDistance(float maxViewDistance)
        {
            m_maxViewDistance = maxViewDistance;
        }

        HoudiniMaterialSettings() = default;
        virtual ~HoudiniMaterialSettings() {}

        AZ_CLASS_ALLOCATOR(HoudiniMaterialSettings, AZ::SystemAllocator);
        AZ_RTTI(HoudiniMaterialSettings, HOUDINI_MESH_SETTINGS_GUID);

        static bool VersionConverter(AZ::SerializeContext& /*context*/, AZ::SerializeContext::DataElementNode& classElement)
        {
            if (classElement.GetVersion() == 3)
            {
                int elementIndex = classElement.FindElement(AZ_CRC("Spacer", 0xaa2c54ee));
                if (elementIndex != -1)
                {
                    classElement.RemoveElement(elementIndex);
                }
            }

            return true;
        }

        static void Reflect(AZ::ReflectContext* context)
        {
            if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serialize->Class<HoudiniMaterialSettings>()
                    ->Version(5, VersionConverter)
                    ->Field("Visible", &HoudiniMaterialSettings::m_visible)
                    ->Field("ForceHighPriTexStreaming", &HoudiniMaterialSettings::m_highPriorityTextureStreaming)
                    ->Field("MaxViewDistance", &HoudiniMaterialSettings::m_maxViewDistance)
                    ->Field("Physics", &HoudiniMaterialSettings::m_physics)
                    ->Field("MaterialSub", &HoudiniMaterialSettings::m_subMaterial)
                    ->Field("MaterialName", &HoudiniMaterialSettings::m_materialName)
                    //->Field("Material", &HoudiniMaterialSettings::m_materialAsset)
                    ;

                if (AZ::EditContext* ec = serialize->GetEditContext())
                {
                    ec->Class<HoudiniMaterialSettings>("HoudiniMeshSettings", "HoudiniMeshSettings")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)

                        ->DataElement(AZ::Edit::UIHandlers::Slider, &HoudiniMaterialSettings::m_subMaterial, "SubMaterial Index", "If using a sub material enter a value here.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0)
                        ->Attribute(AZ::Edit::Attributes::Max, 100)
                        ->Attribute(AZ::Edit::Attributes::SoftMin, 0)
                        ->Attribute(AZ::Edit::Attributes::SoftMax, &HoudiniMaterialSettings::GetSubMaterialCount)
                        ->Attribute(AZ::Edit::Attributes::Step, 1)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniMaterialSettings::OnChanged)

                        ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniMaterialSettings::m_materialName, "Material Name", "")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniMaterialSettings::OnChanged)

                        ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniMaterialSettings::m_visible, "Visible", "")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniMaterialSettings::OnChanged)

                        ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniMaterialSettings::m_highPriorityTextureStreaming, "High Priority Texture Streaming", "")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniMaterialSettings::OnChanged)

                        ->DataElement(AZ::Edit::UIHandlers::Slider, &HoudiniMaterialSettings::m_maxViewDistance, "Max View Distance", "")
                        ->Attribute(AZ::Edit::Attributes::Min, 1.0f)
                        ->Attribute(AZ::Edit::Attributes::Max, 100000000.0f)
                        ->Attribute(AZ::Edit::Attributes::SoftMin, 1.0f)
                        ->Attribute(AZ::Edit::Attributes::SoftMax, 4000.0f)
                        ->Attribute(AZ::Edit::Attributes::Step, 1.0f)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniMaterialSettings::OnChanged)

                        ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniMaterialSettings::m_physics, "Physics", "")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniMaterialSettings::OnPhysicsChanged)

                        /*->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniMaterialSettings::m_materialAsset, "Material", "")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniMaterialSettings::OnChanged)*/

                        ;
                }
            }
        }
    };

    class HoudiniMeshData : public AZ::SerializeContext::IDataSerializer
    {
        friend class HoudiniMaterialTranslator;
        public:
			struct DataRange
			{
				AZ::u32 offset = 0;
				AZ::u32 count = 0;
			};

			AZ_CLASS_ALLOCATOR(HoudiniMeshData, AZ::SystemAllocator, 0);
			AZ_RTTI(HoudiniMeshData, HOUDINI_MESH_DATA_GUID);
			
            HoudiniMeshData() = default;
            virtual ~HoudiniMeshData();

			static void Reflect(AZ::ReflectContext* context)
			{
				if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
				{
					serialize->Class<HoudiniMeshData>()
						->Version(8)
						->Serializer(&AZ::Serialize::StaticInstance<HoudiniMeshData>::s_instance);
					;

					if (AZ::EditContext* ec = serialize->GetEditContext())
					{
						ec->Class<HoudiniMeshData>("HoudiniMeshData", "HoudiniMeshData")
							->ClassElement(AZ::Edit::ClassElements::EditorData, "")
							->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
							;
					}
				}
			}

            
            //UpdateTransform on RenderNode
            void UpdateWorldTransform(const AZ::Transform& entityTransform);

            /// Store the class data into a stream.
            size_t  Save(const void* classPtr, AZ::IO::GenericStream& stream, bool isDataBigEndian = false) override;
            bool    Load(void* classPtr, AZ::IO::GenericStream& stream, unsigned int version, bool isDataBigEndian = false) override;
            size_t  DataToText(AZ::IO::GenericStream& in, AZ::IO::GenericStream& out, bool isDataBigEndian /*= false*/) override;
            size_t  TextToData(const char* text, unsigned int textVersion, AZ::IO::GenericStream& stream, bool isDataBigEndian = false) override;
            bool    CompareValueData(const void* lhs, const void* rhs) override;
            
            void UpdateStatObject();
            
            void ClearSerializedMeshData();

            void SetVisibilityInEditor(bool state);
            void SetVisible(bool state);
            
            void SetMaxViewDistance(float maxViewDistance);
            void SetHighPriorityTextureStreaming(bool value);
            void SetPhysics(bool state);

            void SetMaterial(void*/*IMaterialRef*/ material);
            //IStatObj* GetStatObject() const;
            
            bool IsRenderNodeDirty() const;
            
            size_t GetBytesEstimate() const
            {
                size_t totalSize = 0;
                totalSize += m_indices.size() * sizeof(int);
                totalSize += m_positions.size() * sizeof(Vector3f);
                totalSize += m_normals.size() * sizeof(Vector3f);
                totalSize += m_colors.size() * sizeof(Vector4f);
                totalSize += m_uvs.size() * sizeof(Vector2f);
                totalSize += m_tangents.size() * sizeof(Vector4f);
                totalSize += m_bitangents.size() * sizeof(Vector3f);
                return totalSize;
            }

            //Physics
            bool IsSafeForCryPhysics(/*strided_pointer<const Vec3> pVertices, strided_pointer<unsigned int> pIndices, int nTris*/);
            void EnablePhysics();
            void DisablePhysics();
            //AZStd::shared_ptr<IPhysicalEntity> m_physicalEntity; //O3DECONVERT

			AZ::u32 GetIndexCount() const;
			AZ::u32 GetIndexOffset() const;
			void SetIndexOffset(AZ::u32 offset);
			void SetIndexOffset(AZ::u64 offset);
			void SetIndexCount(AZ::u32 count);
			void SetIndexCount(AZ::u64 count);

			template<AttributeType AttributeTypeT>
			AZ::u32 GetCount() const;
			template<AttributeType AttributeTypeT>
			AZ::u32 GetOffset() const;
			template<AttributeType AttributeTypeT>
			void SetCount(AZ::u32 count);
			template<AttributeType AttributeTypeT>
			void SetCount(AZ::u64 count);
			template<AttributeType AttributeTypeT>
			void SetOffset(AZ::u32 offset);
			template<AttributeType AttributeTypeT>
			void SetOffset(AZ::u64 offset);

			const U32Vector& GetIndices() const;
			void SetIndices(const U32Vector& indices);

			template<AttributeType AttributeTypeT>
			const auto& GetDataBuffer() const;

            void SetMaterialIndex(AZ::u32 materialIndex);
            AZ::u32 GetMaterialIndex() const;
            U32Vector GetIndicesByMaterialIndex(int materialIndex);
            void ClearMaterialList();

            AZ::Aabb GetAabb() const;
			void CalculateTangents();
			void CalculateAABB();

            HoudiniMeshData& operator+=(const HoudiniMeshData& rhs);
            void ClearBuffers();

			int m_materialIndex = 0;
			bool m_physics = false;
			bool m_highPriorityTextureStreaming = false;
			float m_maxViewDistance = 8000;

			static AZStd::mutex m_dataLock;

			/*AZStd::vector<int> Indices;
			AZStd::vector<AZ::Vector3> Positions;
			AZStd::vector<AZ::Vector3> Normals;
			AZStd::vector<AZ::Vector4> Colors;
			AZStd::vector<AZ::Vector3> UVs;
			AZStd::vector<AZ::Vector3> Tangents;
			AZStd::vector<AZ::Vector3> Bitangents;*/

			U32Vector m_indices;
            S32Vector m_materialIndices;
            AZStd::map<AZStd::string, AZ::s32> m_materialMap; // this maps the material id to a material name
			Vert3Vector m_positions;
			Vert3Vector m_normals;
			Vert4Vector m_tangents;
			Vert3Vector m_bitangents;
			Vert2Vector m_uvs;
			Vert4Vector m_colors;

			//! As meshes are eventually merged to common buffers we need to keep track of the offset and element count.
		    //! When we create the Atom Buffers we will need these infos while the mesh buffers will be held in GNModelData.
			DataRange m_indicesRange;
			DataRange m_positionsRange;
			DataRange m_normalsRange;
			DataRange m_tangentsRange;
			DataRange m_bitangentsRange;
			DataRange m_uvsRange;
			DataRange m_colorsRange;

			AZ::Transform m_entityTransform = AZ::Transform::CreateIdentity();
			AZStd::string m_meshName;
			AZ::EntityId m_entityId;

            AZ::Aabb m_aabb = AZ::Aabb::CreateNull();

            MaterialList m_materialNames;
    };
    
}
