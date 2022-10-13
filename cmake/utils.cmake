
macro(L_add_subdirectory_recurse path)
    file(GLOB_RECURSE list_items LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/${path}/*)
    list(APPEND list_items "${CMAKE_CURRENT_SOURCE_DIR}/${path}")
    set(dirs "")

    foreach(ld ${list_items})
        if(IS_DIRECTORY ${ld} AND EXISTS "${ld}/CMakeLists.txt")
            list(APPEND dirs ${ld})
        endif()
    endforeach()

    foreach(d ${dirs})
        message("--> add subdirectory: ${d}")
        add_subdirectory(${d})
    endforeach()
endmacro()

function(L_target_set_cpp target version)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD ${version}
        CXX_STANDARD_REQUIRED ON
    )
endfunction()