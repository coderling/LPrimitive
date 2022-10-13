macro(L_init_project)
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

function(L_get_packagename ret)
    set(${ret} ${PROJECT_NAME} PARENT_SCOPE)
endfunction()

function(L_get_packagename_with_version ret)
    set(tmp ${PROJECT_NAME}_${PROJECT_VERSION})
    string(REPLACE "." "_" tmp "${tmp}")
    set(${ret} ${tmp} PARENT_SCOPE)
endfunction()

macro(L_add_buildsetting_interface_lib target_name)
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
endmacro()

macro(L_export_package)
    L_get_packagename(package_name)
    include(CMakePackageConfigHelpers)

    #
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${package_name}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
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
    L_set_package_init()
endmacro()

macro(L_set_package_init)
    set(L_package_init "
        message(\"package inti hahaha\")
    ")
endmacro()

function(L_install_libraries _target)
    get_target_property(TARGET_TYPE ${_target} TYPE)

    if(TARGET_TYPE STREQUAL STATIC_LIBRARY)
        list(APPEND ENGINE_STATIC_LIBS_LIST ${_target})
        set(ENGINE_STATIC_LIBS_LIST ${ENGINE_STATIC_LIBS_LIST} CACHE INTERNAL "common libraries installation list")
    else()
        install(TARGETS ${_target}
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/$<CONFIG>"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/$<CONFIG>"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/$<CONFIG>"
        )

        install(FILES $<TARGET_PDB_FILE:${_target}> DESTINATION "${CMAKE_INSTALL_BINDIR}/$<CONFIG>" OPTIONAL)
    endif()

    # include install
    get_target_property(target_source_dir ${_target} SOURCE_DIR)
    file(RELATIVE_PATH target_relative_path "${CMAKE_SOURCE_DIR}" "${target_source_dir}")

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/interface")
        install(DIRECTORY interface
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${target_relative_path}"
        )
    endif()
endfunction()

function(InstallCoombinedStaticLib final_lib_name libs_list target_name target_folder install_path)
    foreach(lib ${libs_list})
        list(APPEND full_path_libs $<TARGET_FILE:${lib}>)
    endforeach()

    message("all libs: " ${full_path_libs})
    add_custom_command(
        OUTPUT ${final_lib_name}

        # Delete all object files from current directory
        COMMAND ${CMAKE_COMMAND} -E remove "*${CMAKE_C_OUTPUT_EXTENSION}"
        DEPENDS ${libs_list}
        COMMENT "Combining libraries..."
    )

    # combined libs
    # unpack libs to ${final_lib_name}
    foreach(lib_target ${full_path_libs})
        add_custom_command(
            OUTPUT ${final_lib_name}
            COMMAND ${CMAKE_AR} -x ${lib_target}
            APPEND
        )
    endforeach()

    # pack object to final
    add_custom_command(
        OUTPUT ${final_lib_name}
        COMMAND ${CMAKE_AR} -crs ${final_lib_name} "*${CMAKE_C_OUTPUT_EXTENSION}"
        APPEND
    )

    # install
    add_custom_target(${target_name} ALL DEPENDS ${final_lib_name})
    install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${final_lib_name}"
        DESTINATION
        ${install_path}
    )

    if(TARGET ${target_name})
        set_target_properties(${target_name} PROPERTIES
            FOLDER ${target_folder}
        )
    else()
        message("Unable to find librarian tool. Combined ${COMBINED_LIB_NAME} static library will not be produced.")
    endif()
endfunction()

function(SetTargetOutputName target_name out_name)
    foreach(d_config ${DEBUG_CONFIGS})
        set_target_properties(${target_name} PROPERTIES
            OUTPUT_NAME_${d_config} ${out_name}${DEBUG_DLL_SUFFIX}
        )
    endforeach()

    foreach(r_config ${RELEASE_CONFIGS})
        set_target_properties(${target_name} PROPERTIES
            OUTPUT_NAME_${r_config} ${out_name}${RELEASE_DLL_SUFFIX}
        )
    endforeach()
endfunction()

function(CopyRequiredDLLs target_name)
    if(SUPPORT_D3D12)
        list(APPEND Required_DLLs Graphics-D3D12-Shared)
    endif()

    if(TARGET DXC)
        list(APPEND Required_DLLs DXC)
        get_target_property(dxil_path DXC DXIL_PATH)
        add_custom_command(TARGET ${target_name}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${dxil_path}"
            "\"$<TARGET_FILE_DIR:${target_name}>\""
        )
    endif()

    foreach(DLL_PJ ${Required_DLLs})
        add_custom_command(TARGET ${target_name}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "\"$<TARGET_FILE:${DLL_PJ}>\""
            "\"$<TARGET_FILE_DIR:${target_name}>\""
        )
    endforeach()
endfunction()