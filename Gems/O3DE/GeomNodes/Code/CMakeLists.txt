
# Currently we are in the Code folder: ${CMAKE_CURRENT_LIST_DIR}
# Get the platform specific folder ${pal_dir} for the current folder: ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME}
# Note: o3de_pal_dir will take care of the details for us, as this may be a restricted platform
#       in which case it will see if that platform is present here or in the restricted folder.
#       i.e. It could here in our gem : Gems/GeomNodes/Code/Platform/<platorm_name>  or
#            <restricted_folder>/<platform_name>/Gems/GeomNodes/Code
o3de_pal_dir(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} "${gem_restricted_path}" "${gem_path}" "${gem_parent_relative_path}")

# Now that we have the platform abstraction layer (PAL) folder for this folder, thats where we will find the
# traits for this platform. Traits for a platform are defines for things like whether or not something in this gem
# is supported by this platform.
include(${pal_dir}/PAL_${PAL_PLATFORM_NAME_LOWERCASE}.cmake)

# The GeomNodes.API target declares the common interface that users of this gem should depend on in their targets
ly_add_target(
    NAME GeomNodes.API INTERFACE
    NAMESPACE Gem
    FILES_CMAKE
        geomnodes_api_files.cmake
        ${pal_dir}/geomnodes_api_files.cmake
    INCLUDE_DIRECTORIES
        INTERFACE
            Include
    BUILD_DEPENDENCIES
        INTERFACE
           AZ::AzCore
)

# The GeomNodes.Private.Object target is an internal target
# It should not be used outside of this Gems CMakeLists.txt
ly_add_target(
    NAME GeomNodes.Private.Object STATIC
    NAMESPACE Gem
    FILES_CMAKE
        geomnodes_private_files.cmake
        ${pal_dir}/geomnodes_private_files.cmake
    TARGET_PROPERTIES
        O3DE_PRIVATE_TARGET TRUE
    INCLUDE_DIRECTORIES
        PRIVATE
            Include
            Source
    BUILD_DEPENDENCIES
        PUBLIC
            AZ::AzCore
            AZ::AzFramework
)

# Here add GeomNodes target, it depends on the Private Object library and Public API interface
ly_add_target(
    NAME GeomNodes ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    FILES_CMAKE
        geomnodes_shared_files.cmake
        ${pal_dir}/geomnodes_shared_files.cmake
    INCLUDE_DIRECTORIES
        PUBLIC
            Include
        PRIVATE
            Source
    BUILD_DEPENDENCIES
        PUBLIC
            Gem::GeomNodes.API
        PRIVATE
            Gem::GeomNodes.Private.Object
)

# By default, we will specify that the above target GeomNodes would be used by
# Client and Server type targets when this gem is enabled.  If you don't want it
# active in Clients or Servers by default, delete one of both of the following lines:
ly_create_alias(NAME GeomNodes.Clients NAMESPACE Gem TARGETS Gem::GeomNodes)
ly_create_alias(NAME GeomNodes.Servers NAMESPACE Gem TARGETS Gem::GeomNodes)

# For the Client and Server variants of GeomNodes Gem, an alias to the GeomNodes.API target will be made
ly_create_alias(NAME GeomNodes.Clients.API NAMESPACE Gem TARGETS Gem::GeomNodes.API)
ly_create_alias(NAME GeomNodes.Servers.API NAMESPACE Gem TARGETS Gem::GeomNodes.API)

# If we are on a host platform, we want to add the host tools targets like the GeomNodes.Editor MODULE target
if(PAL_TRAIT_BUILD_HOST_TOOLS)
    # The GeomNodes.Editor.API target can be used by other gems that want to interact with the GeomNodes.Editor module
    ly_add_target(
        NAME GeomNodes.Editor.API INTERFACE
        NAMESPACE Gem
        FILES_CMAKE
            geomnodes_editor_api_files.cmake
            ${pal_dir}/geomnodes_editor_api_files.cmake
        INCLUDE_DIRECTORIES
            INTERFACE
                Include
        BUILD_DEPENDENCIES
            INTERFACE
                AZ::AzToolsFramework
    )

    # The GeomNodes.Editor.Private.Object target is an internal target
    # which is only to be used by this gems CMakeLists.txt and any subdirectories
    # Other gems should not use this target
    ly_add_target(
        NAME GeomNodes.Editor.Private.Object STATIC
        NAMESPACE Gem
        AUTOMOC
        FILES_CMAKE
            geomnodes_editor_private_files.cmake
        TARGET_PROPERTIES
            O3DE_PRIVATE_TARGET TRUE
        INCLUDE_DIRECTORIES
            PRIVATE
                Include
                Source
        # COMPILE_DEFINITIONS
        #     PRIVATE
        #         DEBUG_SCRIPT_USING_ID=123456
        BUILD_DEPENDENCIES
            PUBLIC
                AZ::AzToolsFramework
                Gem::Atom_RPI.Public
                Gem::Atom_Feature_Common.Static
                Gem::AtomLyIntegration_CommonFeatures.Static
                $<TARGET_OBJECTS:Gem::GeomNodes.Private.Object>
                GeomNodes::Bridge
    )

    ly_add_target(
        NAME GeomNodes.Editor GEM_MODULE
        NAMESPACE Gem
        AUTOMOC
        FILES_CMAKE
            geomnodes_editor_shared_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
            PUBLIC
                Include
        BUILD_DEPENDENCIES
            PUBLIC
                Gem::GeomNodes.Editor.API
            PRIVATE
                Gem::GeomNodes.Editor.Private.Object
    )

    # By default, we will specify that the above target GeomNodes would be used by
    # Tool and Builder type targets when this gem is enabled.  If you don't want it
    # active in Tools or Builders by default, delete one of both of the following lines:
    ly_create_alias(NAME GeomNodes.Tools    NAMESPACE Gem TARGETS Gem::GeomNodes.Editor)
    ly_create_alias(NAME GeomNodes.Builders NAMESPACE Gem TARGETS Gem::GeomNodes.Editor)

    # For the Tools and Builders variants of GeomNodes Gem, an alias to the GeomNodes.Editor API target will be made
    ly_create_alias(NAME GeomNodes.Tools.API NAMESPACE Gem TARGETS Gem::GeomNodes.Editor.API)
    ly_create_alias(NAME GeomNodes.Builders.API NAMESPACE Gem TARGETS Gem::GeomNodes.Editor.API)

endif()

################################################################################
# Tests
################################################################################
# See if globally, tests are supported
if(PAL_TRAIT_BUILD_TESTS_SUPPORTED)
    # We globally support tests, see if we support tests on this platform for GeomNodes.Tests
    if(PAL_TRAIT_GEOMNODES_TEST_SUPPORTED)
        # We support GeomNodes.Tests on this platform, add dependency on the Private Object target
        ly_add_target(
            NAME GeomNodes.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
            NAMESPACE Gem
            FILES_CMAKE
                geomnodes_tests_files.cmake
            INCLUDE_DIRECTORIES
                PRIVATE
                    Tests
                    Source
            BUILD_DEPENDENCIES
                PRIVATE
                    AZ::AzTest
                    AZ::AzFramework
                    Gem::GeomNodes.Private.Object
        )

        # Add GeomNodes.Tests to googletest
        ly_add_googletest(
            NAME Gem::GeomNodes.Tests
        )
    endif()

    # If we are a host platform we want to add tools test like editor tests here
    if(PAL_TRAIT_BUILD_HOST_TOOLS)
        # We are a host platform, see if Editor tests are supported on this platform
        if(PAL_TRAIT_GEOMNODES_EDITOR_TEST_SUPPORTED)
            # We support GeomNodes.Editor.Tests on this platform, add GeomNodes.Editor.Tests target which depends on
            # private GeomNodes.Editor.Private.Object target
            ly_add_target(
                NAME GeomNodes.Editor.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
                NAMESPACE Gem
                FILES_CMAKE
                    geomnodes_editor_tests_files.cmake
                INCLUDE_DIRECTORIES
                    PRIVATE
                        Tests
                        Source
                BUILD_DEPENDENCIES
                    PRIVATE
                        AZ::AzTest
                        Gem::GeomNodes.Private.Object
            )

            # Add GeomNodes.Editor.Tests to googletest
            ly_add_googletest(
                NAME Gem::GeomNodes.Editor.Tests
            )
        endif()
    endif()
endif()
