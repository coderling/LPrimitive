cmake_minimum_required(VERSION 3.20)
project(LPrimitive VERSION 0.0.2)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(Project)
include(utils)
L_init_project()

L_add_buildsetting_interface_lib(BuildSetting)

L_add_subdirectory_recurse(src)
L_add_subdirectory_recurse(test)

L_export_package()