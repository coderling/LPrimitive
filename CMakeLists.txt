cmake_minimum_required(VERSION 3.20)
project(LPrimitive VERSION 0.0.2)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(Project)
include(Common)
InitProject()

AddProjectBuildSetting(LBuilSetting)

add_library(LPrimitiveInterface INTERFACE)
target_link_libraries(LPrimitiveInterface INTERFACE Common)

add_subdirectory(src)
add_subdirectory(test)

EXportPackage(LPrimitive)