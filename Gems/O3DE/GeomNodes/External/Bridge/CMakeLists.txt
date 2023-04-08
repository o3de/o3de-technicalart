
# set(LIB_NAME Bridge)
# set(LIB_TYPE SHARED)

# add_library(${LIB_NAME} ${LIB_TYPE} 
#             Bridge.cpp
#             Bridge.h
# )

# install(TARGETS ${LIB_NAME}
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_LIBDIR})

ly_add_target(
    NAME Bridge ${PAL_TRAIT_MONOLITHIC_DRIVEN_LIBRARY_TYPE}
    NAMESPACE GeomNodes
    FILES_CMAKE
        bridge_files.cmake
    INCLUDE_DIRECTORIES
        INTERFACE
            .
        PUBLIC
            .
    BUILD_DEPENDENCIES
        PRIVATE
            AZ::AzCore
        PUBLIC
            AZ::AzToolsFramework
    RUNTIME_DEPENDENCIES
        AZ::AzCore
)

ly_create_alias(NAME BridgeLib NAMESPACE GeomNodes TARGETS GeomNodes::Bridge)