
# Query the gem name from the gem.json file if possible
# otherwise fallback to using urdf-exporter-gem
if (NOT gem_name)
    set(gem_name "urdf-exporter-gem")
endif()

# Fallback to using the current source CMakeLists.txt directory as the gem root path
if (NOT gem_path)
    set(gem_path ${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(gem_json ${gem_path}/gem.json)

o3de_restricted_path(${gem_json} o3de_gem_restricted_path)

add_subdirectory(Code)
