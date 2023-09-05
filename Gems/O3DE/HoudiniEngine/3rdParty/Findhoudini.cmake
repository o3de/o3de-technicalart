ly_add_external_target(
    NAME houdini
    VERSION
    3RDPARTY_ROOT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../External/${HOUDINI_CUR_VER}
    INCLUDE_DIRECTORIES include
)