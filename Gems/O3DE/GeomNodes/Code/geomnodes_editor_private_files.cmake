#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Source/Editor/Commons.h
    Source/Editor/Math/MathHelper.cpp
    Source/Editor/Math/MathHelper.h
    Source/Editor/Common/GNAPI.cpp
    Source/Editor/Common/GNAPI.h
    Source/Editor/Common/GNEvents.h
    Source/Editor/Common/GNConstants.h
    Source/Editor/Components/EditorGeomNodesSystemComponent.cpp
    Source/Editor/Components/EditorGeomNodesSystemComponent.h
    Source/Editor/Components/EditorGeomNodesComponent.cpp
    Source/Editor/Components/EditorGeomNodesComponent.h
    Source/Editor/Configuration/GNConfiguration.cpp
    Source/Editor/Configuration/GNConfiguration.h
    Source/Editor/Configuration/GNEditorSettingsRegistryManager.cpp
    Source/Editor/Configuration/GNEditorSettingsRegistryManager.h
    Source/Editor/Configuration/GNSettingsRegistryManager.cpp
    Source/Editor/Configuration/GNSettingsRegistryManager.h
    Source/Editor/EBus/EditorGeomNodesComponentBus.h
    Source/Editor/EBus/GeomNodesBus.h
    Source/Editor/EBus/ValidatorBus.h
    Source/Editor/EBus/IpcHandlerBus.h
    Source/Editor/UI/ConfigurationWidget.cpp
    Source/Editor/UI/ConfigurationWidget.h
    Source/Editor/UI/EditorWindow.cpp
    Source/Editor/UI/EditorWindow.h
    Source/Editor/UI/EditorWindow.ui
    Source/Editor/UI/Utils.cpp
    Source/Editor/UI/Utils.h
    Source/Editor/UI/UI_common.h
    Source/Editor/UI/FunctorValidator.cpp
    Source/Editor/UI/FunctorValidator.h
    Source/Editor/UI/GeomNodesValidator.cpp
    Source/Editor/UI/GeomNodesValidator.h
    Source/Editor/UI/Validators.cpp
    Source/Editor/UI/Validators.h
    Source/Editor/UI/ValidationHandler.cpp
    Source/Editor/UI/ValidationHandler.h
    Source/Editor/UI/PropertyFileSelect.cpp
    Source/Editor/UI/PropertyFileSelect.h
    Source/Editor/UI/PropertyFuncValBrowseEdit.cpp
    Source/Editor/UI/PropertyFuncValBrowseEdit.h
    Source/Editor/UI/SettingsWidget.cpp
    Source/Editor/UI/SettingsWidget.h
    Source/Editor/Rendering/Atom/GNAttributeBuffer.h
    Source/Editor/Rendering/Atom/GNBuffer.h
    Source/Editor/Rendering/GNMeshController.cpp
    Source/Editor/Rendering/GNMeshController.h
    Source/Editor/Rendering/GNMeshData.cpp
    Source/Editor/Rendering/GNMeshData.h
    Source/Editor/Rendering/GNModelData.cpp
    Source/Editor/Rendering/GNModelData.h
    Source/Editor/Rendering/GNRenderMesh.cpp
    Source/Editor/Rendering/GNRenderMesh.h
    Source/Editor/Rendering/GNRenderMeshInterface.h
    Source/Editor/Systems/GeomNodesSystem.cpp
    Source/Editor/Systems/GeomNodesSystem.h
    Source/Editor/Systems/GNInstance.cpp
    Source/Editor/Systems/GNInstance.h
    Source/Editor/Systems/GNParamContext.cpp
    Source/Editor/Systems/GNParamContext.h
    Source/Editor/Systems/GNProperty.cpp
    Source/Editor/Systems/GNProperty.h
)
