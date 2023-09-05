#pragma once

#include <HAPI/HAPI.h>

#include <AzCore/Script/ScriptProperty.h>

namespace HoudiniEngine
{
    class HoudiniScriptProperty;
    class HoudiniScriptPropertyFactory
    {
        public:
            static HoudiniScriptProperty* CreateNew(HoudiniParameterPtr param );
    };

    class HoudiniScriptProperty : public AZ::ScriptProperty
    {
        friend class Houdini;
        friend class HoudiniScriptPropertyBoolean;
        friend class HoudiniScriptPropertyInt;
        friend class HoudiniScriptPropertyIntChoice;
        friend class HoudiniScriptPropertyFloat;
        friend class HoudiniScriptPropertyVector2;
        friend class HoudiniScriptPropertyVector3;
        friend class HoudiniScriptPropertyVector4;
        friend class HoudiniScriptPropertyString;
        friend class HoudiniScriptPropertyEntity;
        friend class HoudiniScriptPropertyFile;
        friend class HoudiniScriptPropertyInput;
        friend class HoudiniScriptPropertyColor; // FL[FD-11758] Add support for color parameter to lumberyard houdini engine
        friend class HoudiniScriptPropertyMultiparm;  // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
        friend class HoudiniScriptPropertyButton;  // FL[FD-12463] Implement button interface parm in houdini engine for lumberyard

        protected:
            IHoudini* m_hou;
            HAPI_Session* m_session;
            HoudiniNodePtr m_node;
            HoudiniParameterPtr m_parameter;            

            AZStd::string m_label;
            AZStd::string m_help;
            AZStd::string m_desc;
            AZStd::string m_groupName;
                        
            int m_min_int;
            int m_max_int;
            int m_step_int;

            double m_min;
            double m_max;
            double m_step;

        public:
            AZ_CLASS_ALLOCATOR(HoudiniScriptProperty, AZ::SystemAllocator);
            AZ_RTTI(HoudiniScriptProperty, "{A39487BF-5082-44ED-ADF1-CDED4A6A8789}", AZ::ScriptProperty);
            static void Reflect(AZ::ReflectContext* reflection);
            static bool VersionConverter(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& classElement);

            HoudiniScriptProperty() = default;
            HoudiniScriptProperty(HoudiniParameterPtr parameter);
            HoudiniScriptProperty(HoudiniNodePtr node, int index);

            void SetGroupName(const AZStd::string& groupName ) { m_groupName = groupName; }
            AZStd::string GetGroupName() { return m_groupName; }

            virtual AZ::u32 ScriptHasChanged();
            virtual void Update() { }

            HoudiniScriptProperty* Clone(const char* /*name*/) const override{return aznew HoudiniScriptProperty(m_parameter);}
            const void* GetDataAddress() const override { return nullptr; }
            AZ::TypeId GetDataTypeUuid() const override
            {
                return AZ::SerializeTypeInfo<double>::GetUuid();
            }
            bool Write(AZ::ScriptContext& /*context*/) override { return true;}
            bool TryRead(AZ::ScriptDataContext& /*context*/, int /*valueIndex*/) override { return true; }
            void CloneDataFrom(const AZ::ScriptProperty* /*scriptProperty*/){}

            virtual void SetNode(HoudiniNodePtr node)  // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
            {
                m_hou = node->GetHou();
                m_node = node;
            }

            void SetLabel(const AZStd::string& label)
            {
                m_label = label;
            }

            void SetParameter(HoudiniParameterPtr parm)
            {
                m_hou = parm->GetHou();
                m_parameter = parm;
                m_session = const_cast<HAPI_Session*>(&m_hou->GetSession());
                m_name = parm->GetName();
                m_help = parm->GetHelp();
                m_desc = m_name + ": " + m_help;
                m_label = parm->GetLabel();

                m_min = (double)(parm->GetInfo().min);
                m_max = (double)(parm->GetInfo().max);
                m_min_int = (int)lroundf(parm->GetInfo().min);
                m_max_int = (int)lroundf(parm->GetInfo().max);
                m_step = 1.0;
                m_step_int = 1;
            }
    };

    class HoudiniScriptPropertyBoolean : public HoudiniScriptProperty
    {
    public:
        bool m_value;

    public:
        AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyBoolean, AZ::SystemAllocator);
        AZ_RTTI(HoudiniScriptPropertyBoolean, "{66286CF2-C6FD-4821-A75D-9D1402F1568A}", HoudiniScriptProperty);
        static void Reflect(AZ::ReflectContext* reflection);

        HoudiniScriptPropertyBoolean() = default;
        HoudiniScriptPropertyBoolean(HoudiniParameterPtr parameter);

        AZ::u32 ScriptHasChanged() override;
    };

    class HoudiniScriptPropertyString : public HoudiniScriptProperty
    {
    public:
        AZStd::string m_value;

    public:
        AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyString, AZ::SystemAllocator);
        AZ_RTTI(HoudiniScriptPropertyString, "{6BB0005F-82FA-4F95-A9CA-A3EDFB15CF4E}", HoudiniScriptProperty);
        static void Reflect(AZ::ReflectContext* reflection);

        HoudiniScriptPropertyString() = default;
        HoudiniScriptPropertyString(HoudiniParameterPtr parameter);

        AZ::u32 ScriptHasChanged() override;
    };

    class HoudiniScriptPropertyInt : public HoudiniScriptProperty
    {
        public:
            int m_value;

        public:
            AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyInt, AZ::SystemAllocator);
            AZ_RTTI(HoudiniScriptPropertyInt, "{80BDB942-C6D0-4D76-BE8B-F5F559A6C98F}", HoudiniScriptProperty);
            static void Reflect(AZ::ReflectContext* reflection);

            HoudiniScriptPropertyInt() = default;
            HoudiniScriptPropertyInt(HoudiniParameterPtr parameter);

            AZ::u32 ScriptHasChanged() override;
    };

	class HoudiniScriptPropertyIntChoice : public HoudiniScriptProperty
	{
	public:
		int m_value;
        AZStd::string m_choice;
        AZStd::vector<AZStd::string> m_choices;

	public:
		AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyIntChoice, AZ::SystemAllocator);
		AZ_RTTI(HoudiniScriptPropertyIntChoice, "{65C109CE-88AE-420E-BCCC-15F78C64EFAF}", HoudiniScriptProperty);
		static void Reflect(AZ::ReflectContext* reflection);

        HoudiniScriptPropertyIntChoice() = default;
        HoudiniScriptPropertyIntChoice(HoudiniParameterPtr parameter);

		AZ::u32 ScriptHasChanged() override;

        AZStd::vector<AZStd::string> GetChoices();
        int GetChoiceIndex(const AZStd::string& choice);
	};

    class HoudiniScriptPropertyFloat : public HoudiniScriptProperty
    {
        public:
            double m_value;

        public:
            AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyFloat, AZ::SystemAllocator);
            AZ_RTTI(HoudiniScriptPropertyFloat, "{8EFE89AB-8C17-4582-83B8-8DC890FFBB6E}", HoudiniScriptProperty);
            static void Reflect(AZ::ReflectContext* reflection);

            HoudiniScriptPropertyFloat() = default;
            HoudiniScriptPropertyFloat(HoudiniParameterPtr parameter);

            AZ::u32 ScriptHasChanged() override;
    };

    class HoudiniScriptPropertyVector2 : public HoudiniScriptProperty
    {
    public:
        AZ::Vector2 m_value;

    public:
        AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyVector2, AZ::SystemAllocator);
        AZ_RTTI(HoudiniScriptPropertyVector2, "{300AEC50-88E3-4BAC-8CD1-A7A5BEF7371B}", HoudiniScriptProperty);
        static void Reflect(AZ::ReflectContext* reflection);

        HoudiniScriptPropertyVector2() = default;
        HoudiniScriptPropertyVector2(HoudiniParameterPtr parameter);

        AZ::u32 ScriptHasChanged() override;
    };

    class HoudiniScriptPropertyVector3 : public HoudiniScriptProperty
    {
    public:
        AZ::Vector3 m_value;

    public:
        AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyVector3, AZ::SystemAllocator);
        AZ_RTTI(HoudiniScriptPropertyVector3, "{E2D3DF58-BD98-4CCC-9A68-DD049C64E3B8}", HoudiniScriptProperty);
        static void Reflect(AZ::ReflectContext* reflection);

        HoudiniScriptPropertyVector3() = default;
        HoudiniScriptPropertyVector3(HoudiniParameterPtr parameter);

        AZ::u32 ScriptHasChanged() override;
    };

    class HoudiniScriptPropertyVector4 : public HoudiniScriptProperty
    {
    public:
        AZ::Vector4 m_value;

    public:
        AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyVector4, AZ::SystemAllocator);
        AZ_RTTI(HoudiniScriptPropertyVector4, "{980B7034-4652-4B87-83D4-2B62F7A48F6C}", HoudiniScriptProperty);
        static void Reflect(AZ::ReflectContext* reflection);

        HoudiniScriptPropertyVector4() = default;
        HoudiniScriptPropertyVector4(HoudiniParameterPtr parameter);

        AZ::u32 ScriptHasChanged() override;
    };


    class HoudiniScriptPropertyEntity : public HoudiniScriptProperty
    {
    public:
        bool m_dirty = true;
        AZ::EntityId m_value;

    public:
        AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyEntity, AZ::SystemAllocator);
        AZ_RTTI(HoudiniScriptPropertyEntity, "{0742B384-7C3B-4D02-94EE-E1C1D4F5B70F}", HoudiniScriptProperty);
        static void Reflect(AZ::ReflectContext* reflection);

        HoudiniScriptPropertyEntity() = default;
        HoudiniScriptPropertyEntity(HoudiniParameterPtr parameter);

        AZ::u32 ScriptHasChanged() override;
        void Update() override;
    };

    class AnyAsset
    {
    public:
        AZ_TYPE_INFO(AnyAsset, "{DB5A9E51-590C-4355-A2E7-DBAD42CB0805}");
        static const char* GetFileFilter()
        {
            return "*.txt; *.json; *.xml; *.dat; *.dds; *.tif; *.gif; *.jpg; *.jpeg; *.jpe; *.tga; *.png; *.sprite; *.obj; *.fbx; *.bgeo";
        }
    };

    // SUPPORT FOR ANY MESH TYPE
    class HoudiniScriptPropertyFile : public HoudiniScriptProperty
    {
    public:
        bool m_dirty = true;
        //AZ::Data::Asset<LmbrCentral::MeshAsset> m_value; //ATOMCONVERT

    public:
        AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyFile, AZ::SystemAllocator);
        AZ_RTTI(HoudiniScriptPropertyFile, "{C9BC4A56-95AD-4FD8-B8EF-8F4418400D0E}", HoudiniScriptProperty);
        static void Reflect(AZ::ReflectContext* reflection);

        HoudiniScriptPropertyFile() = default;
        HoudiniScriptPropertyFile(HoudiniParameterPtr parameter);

        AZ::u32 ScriptHasChanged() override;
        void Update() override;
    };


    // SUPPORT FOR ANY IMAGE TYPE
    class HoudiniScriptPropertyFileImage : public HoudiniScriptProperty
    {
    public:
        bool m_dirty = true;
        //AzFramework::SimpleAssetReference<LmbrCentral::TextureAsset> m_value; //ATOMCONVERT

    public:
        AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyFileImage, AZ::SystemAllocator);
        AZ_RTTI(HoudiniScriptPropertyFileImage, "{C9BC4A56-95AD-4FD8-B8EF-8F4418400D0F}", HoudiniScriptProperty);
        static void Reflect(AZ::ReflectContext* reflection);

        HoudiniScriptPropertyFileImage() = default;
        HoudiniScriptPropertyFileImage(HoudiniParameterPtr parameter);

        AZ::u32 ScriptHasChanged() override;
        void Update() override;
    };

    // SUPPORT FOR ANY FILE TYPE
    class HoudiniScriptPropertyFileAny : public HoudiniScriptProperty
    {
    public:
        bool m_dirty = true;
        //AzFramework::SimpleAssetReference<AnyAsset> m_value; //This doesn't work at this time:
        //AzFramework::SimpleAssetReference<LmbrCentral::TextureAsset> m_value; //ATOMCONVERT
        
    public:
        AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyFileAny, AZ::SystemAllocator);
        AZ_RTTI(HoudiniScriptPropertyFileAny, "{510B9D5E-A3BD-42F6-8454-700C6657E0D0}", HoudiniScriptProperty);
        static void Reflect(AZ::ReflectContext* reflection);

        HoudiniScriptPropertyFileAny() = default;
        HoudiniScriptPropertyFileAny(HoudiniParameterPtr parameter);

        AZ::u32 ScriptHasChanged() override;
        void Update() override;
    };


    class HoudiniScriptPropertyInput : public HoudiniScriptProperty
    {
    public:
        int m_index;
        AZ::EntityId m_value;

    public:
        AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyInput, AZ::SystemAllocator);
        AZ_RTTI(HoudiniScriptPropertyInput, "{D17BFF9A-FC9F-40DC-AE2D-E1FDB69FA2DF}", HoudiniScriptProperty);
        static void Reflect(AZ::ReflectContext* reflection);

        HoudiniScriptPropertyInput() = default;
        HoudiniScriptPropertyInput(HoudiniNodePtr nodeInput, int index);

        void SetInputIndex(int index) 
        {
            m_index = index;
        }

        AZ::u32 ScriptHasChanged() override;
    };

    // FL[FD-11758] Add support for color parameter to lumberyard houdini engine
    class HoudiniScriptPropertyColor : public HoudiniScriptProperty
    {
    public:
        AZ::Color m_value{ AZ::Color::CreateZero() };

    public:
        AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyColor, AZ::SystemAllocator);
        AZ_RTTI(HoudiniScriptPropertyColor, "{99704DF2-2715-4C42-AE08-CD16CD6CA116}", HoudiniScriptProperty);
        static void Reflect(AZ::ReflectContext* reflection);

        HoudiniScriptPropertyColor() = default;
        HoudiniScriptPropertyColor(HoudiniParameterPtr parameter);

        AZ::u32 ScriptHasChanged() override;
    };

    // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
    class HoudiniMultiparamInstance : public HoudiniScriptProperty
    {
        friend class HoudiniScriptPropertyMultiparm;

    public:
        AZ_CLASS_ALLOCATOR(HoudiniMultiparamInstance, AZ::SystemAllocator);
        AZ_RTTI(HoudiniMultiparamInstance, "{7F4D37D5-D2D6-41F4-BBD6-08A55B8CE9FD}", HoudiniScriptProperty);
        static void Reflect(AZ::ReflectContext* reflection);

        HoudiniMultiparamInstance() = default;
        HoudiniMultiparamInstance(HoudiniParameterPtr parameter);

        AZ::u32 ScriptHasChanged();

        void SetParentParam(HoudiniParameterPtr parameter);
        void SetNode(HoudiniNodePtr node) override;

        AZStd::string GetLabelOverride();

    private:
        AZStd::vector<HoudiniScriptProperty*> m_params;
        int m_instanceNum{ 0 };
    };

    class HoudiniScriptPropertyMultiparm : public HoudiniScriptProperty
    {
    public:
        AZStd::vector<HoudiniMultiparamInstance*> m_instances;
        unsigned int m_value{ 0 };

    public:
        AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyMultiparm, AZ::SystemAllocator);
        AZ_RTTI(HoudiniScriptPropertyMultiparm, "{D55902E2-5C81-4359-BCBE-1DE65E397C44}", HoudiniScriptProperty);
        static void Reflect(AZ::ReflectContext* reflection);

        HoudiniScriptPropertyMultiparm() = default;
        HoudiniScriptPropertyMultiparm(HoudiniParameterPtr parameter);

        void SetNode(HoudiniNodePtr node) override;

        AZ::u32 ScriptHasChanged() override;
        AZ::u32 InstanceNumHasChanged();
        AZStd::string GetInstanceSlotLabel(int index);
    };

    // FL[FD-12463] Implement button interface parm in houdini engine for lumberyard
    class HoudiniScriptPropertyButton : public HoudiniScriptProperty
    {
    public:
        bool m_callbackButton{ false };

    public:
        AZ_CLASS_ALLOCATOR(HoudiniScriptPropertyButton, AZ::SystemAllocator);
        AZ_RTTI(HoudiniScriptPropertyButton, "{4A9E89F8-774B-4F1D-B005-5836AB1D64BE}", HoudiniScriptProperty);
        static void Reflect(AZ::ReflectContext* reflection);

        HoudiniScriptPropertyButton() = default;
        HoudiniScriptPropertyButton(HoudiniParameterPtr parameter);

        AZ::u32 ScriptHasChanged() override;
    };
}
