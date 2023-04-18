
# Query the gem name from the gem.json file if possible
# otherwise fallback to using DreamStudioAI
o3de_find_ancestor_gem_root(gempath gem_name "${CMAKE_CURRENT_SOURCE_DIR}")
if (NOT gem_name)
    set(gem_name "DreamStudioAI")
endif()

# Fallback to using the current source CMakeLists.txt directory as the gem root path
if (NOT gem_path)
    set(gem_path ${CMAKE_CURRENT_SOURCE_DIR})
endif()

update_pip_requirements(${gem_path}/requirements.txt ${gem_name})

set(gem_json ${gem_path}/gem.json)

o3de_restricted_path(${gem_json} o3de_gem_restricted_path)

add_subdirectory(Code)
