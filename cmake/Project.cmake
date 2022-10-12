macro(InitProject)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
    endif()

    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        set(ARCH 64 CACHE INTERNAL "64-bit architecture")
    else()
        set(ARCH 32 CACHE INTERNAL "32-bit architecture")
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(IS_DEBUG_BUILD TRUE CACHE INTERNAL "")
    else()
        set(IS_RELEASE_BUILD TRUE CACHE INTERNAL "")
    endif()

    set(DEBUG_DLL_SUFFIX _x${ARCH}_d CACHE INTERNAL "")
    set(RELEASE_DLL_SUFFIX _x${ARCH}_r CACHE INTERNAL "")

    set(TARGET_PLATFORM_WIN FALSE CACHE INTERNAL "")

    if(WIN32)
        set(TARGET_PLATFORM_WIN TRUE CACHE INTERNAL "set default target platform")
        message("build target platform: Win32 arc: " ${ARCH} " " ${CMAKE_BUILD_TYPE})
    else()
        message("Not support platform current platform")
    endif()

    set(ENGINE_STATIC_LIBS_LIST "" CACHE INTERNAL "Libraries for install")
endmacro()

function(AddProjectBuildSetting target_name)
    add_library(${target_name} INTERFACE)
    target_compile_definitions(${target_name} INTERFACE "$<$<CONFIG:DEBUG>:_DEBUG;DEBUG;DEVELOPMENT>")
    target_compile_definitions(${target_name} INTERFACE "$<$<CONFIG:RELWITHDEBINFO>:DEVELOPMENT;>")

    if(TARGET_PLATFORM_WIN)
        target_compile_definitions(${target_name} INTERFACE PLATFORM_WINDOWS=1)
    endif()

    if(MSVC)
        target_compile_options(${target_name} INTERFACE /Zc:preprocessor /W4 /wd4100 /wd4201 /wd4505 /wd4819 /MP)
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${target_name} INTERFACE

            # make all warning as error
            -Werror

            # enable some warning
            -Wall -Wextra -Wuninitialized -Wconditional-uninitialized -Wextra-tokens -Wpointer-arith -Wloop-analysis -Wunused -Wconversion

            # disable some warning
            -Wno-unused-parameter
        )
    endif()

    set(DEBUG_CONFIGS DEBUG CACHE INTERNAL "debug configs")
    set(RELEASE_CONFIGS RELEASE RELWITHDEBINFO MINSIZEREL CACHE INTERNAL "release configs")

    foreach(r_config ${RELEASE_CONFIGS})
        target_compile_definitions(${target_name} INTERFACE "$<$<CONFIG:${r_config}>:NDEBUG;RELEASE>")
    endforeach()
endfunction()

macro(EXportPackage)
    include(CMakePackageConfigHelpers)
    set(package_name ${CMAKE_PROJECT_NAME})
    set(tname ${package_name}Targets)

    if(ARG_TARGET)
        export(EXPORT ${tname}
            NAMESPACE L::
        )
        install(EXPORT ${tname}
            FILE ${tname}.cmake
            NAMESPACE L::
            DESTINATION ${package_name}/cmake
        )
    endif()

    #
    include(CmakePackageConfigHelpers)

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${package_name}ConfigVersion.cmake"
        VERSION "${CMAKE_PROJECT_VERSION}"
        COMPATIBILITY SameMinorVersion
    )

    configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/config/${package_name}.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${package_name}Config.cmake"
        INSTALL_DESTINATION ${package_name}/cmake
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${package_name}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${package_name}ConfigVersion.cmake"
        DESTINATION ${package_name}/cmake
    )
endmacro()