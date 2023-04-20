/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

namespace GeomNodes
{
    // System Component TypeIds
    inline constexpr const char* GeomNodesSystemComponentTypeId = "{E9264AFB-2C90-4438-B689-52AD2A9A0D67}";
    inline constexpr const char* GeomNodesEditorSystemComponentTypeId = "{EACD840D-4AEC-44FB-A6BD-BCF091E07761}";

    // Module derived classes TypeIds
    inline constexpr const char* GeomNodesModuleInterfaceTypeId = "{835A3807-95B6-4420-948E-2790015DDA39}";
    inline constexpr const char* GeomNodesModuleTypeId = "{0B4471BC-6720-4B1A-846E-CFA27F5C64CB}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* GeomNodesEditorModuleTypeId = GeomNodesModuleTypeId;

    // Interface TypeIds
    inline constexpr const char* GeomNodesRequestsTypeId = "{A5ECC740-0242-4ED8-A4BB-DFC9B5CBAD44}";

    // Component TypeIds
    inline constexpr const char* EditorGeomNodesComponentTypeId = "{E59507EF-9EBB-4F6C-8D89-92DCA57722E5}";

    // Other TypeIds
    inline constexpr const char* GNRenderMeshTypeId = "{4E293CD2-F9E6-417C-92B7-DDAF312F46CF}";
    inline constexpr const char* GNRenderMeshInterfaceTypeId = "{908CB056-4814-42FF-9D60-2D67A720D829}";
    inline constexpr const char* GeomNodesSystemTypeId = "{23791BF8-D9DE-4827-88D8-37DA39258570}";
    inline constexpr const char* GNPropertyTypeId = "{71904E43-F0A1-45EA-B87F-4CC5234E1E52}";
    inline constexpr const char* GNParamNilTypeId = "{519D98C7-054A-4047-BCEB-28DCD38CFCD4}";
    inline constexpr const char* GNParamBooleanTypeId = "{6A05BCAB-50F7-4988-96E1-0EDB6B76C3A3}";
    inline constexpr const char* GNParamIntTypeId = "{B2457A3D-F30C-43F9-90F0-5BFAFFFD0F59}";
    inline constexpr const char* GNParamValueTypeId = "{4790660B-B942-4421-B942-AE27DF67BF4F}";
    inline constexpr const char* GNParamStringTypeId = "{9296C827-0281-4DBF-AC1A-B6636BCEC716}";
    inline constexpr const char* GNSystemInterfaceTypeId = "{83173679-DAF6-4496-BEFA-B0D252C40366}";
    inline constexpr const char* GNPropertyGroupTypeId = "{439E8395-77B5-4BC6-94D6-5A0F51DBE9FD}";
    inline constexpr const char* GNParamContextTypeId = "{AA9713B7-70F1-43CB-9F95-5BEC9F44F556}";
    inline constexpr const char* GNParamDataContextTypeId = "{61ED88BA-210A-458B-A5E5-C71C05C05411}";
    inline constexpr const char* GNConfigurationTypeId = "{828F21E8-D1C4-480E-A29B-33B618695874}";
} // namespace GeomNodes