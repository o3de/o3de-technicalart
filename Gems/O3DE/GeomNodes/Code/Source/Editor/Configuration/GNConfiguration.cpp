/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Editor/Configuration/GNConfiguration.h>

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace GeomNodes
{
    AZ_CLASS_ALLOCATOR_IMPL(GNConfiguration, AZ::SystemAllocator);

    void GNConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GNConfiguration>()
                ->Version(2)
                ->Field("Blender Path", &GNConfiguration::m_blenderPath)
                ->Field("Last Selected Path", &GNConfiguration::m_lastFilePath);

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<GNConfiguration>("GeomNodes Configuration", "Default GeomNodes configuration")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::ExeSelectBrowseEdit, &GNConfiguration::m_blenderPath, "Blender Path", "Blender Path")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GNConfiguration::OnBlenderPathChanged);
            }
        }
    }

    GNConfiguration GNConfiguration::CreateDefault()
    {
        return GNConfiguration();
    }

    bool GNConfiguration::operator==(const GNConfiguration& other) const
    {
        return m_blenderPath == other.m_blenderPath && m_lastFilePath == other.m_lastFilePath;
    }

    bool GNConfiguration::operator!=(const GNConfiguration& other) const
    {
        return !(*this == other);
    }

    AZ::u32 GNConfiguration::OnBlenderPathChanged()
    {
        return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
    }
} // namespace GeomNodes