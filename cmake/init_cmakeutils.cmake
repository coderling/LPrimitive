macro(L_InitCMakeUtils)
    set(name CMakeUtils)
    set(address "https://github.com/coderling/${name}.git")
    set(version ${ARGV1})
    message("find packae ${name} version ${tag}")

    set(is_already_add FALSE)

    if(${self_package_name}_depend_list)
        list(FIND ${self_package_name}_depend_list "${name}" _index)

        if(index GREATER_EQUAL 0)
            set(is_already_add TRUE)
        endif()
    endif()

    if(NOT is_already_add)
        find_package(${name} QUIET)

        if(${${name}_FOUND})
            message("Found ${name} ${version} local")
        else()
            if(NOT FetchContent_FOUND)
                include(FetchContent)
            endif()

            if(${version})
                message("update repo ${name} ${version}")
                FetchContent_Declare(
                    ${name}
                    GIT_REPOSITORY ${address}
                    GIT_TAG ${version}
                )
            else()
                message("update repo ${name} master")
                FetchContent_Declare(
                    ${name}
                    GIT_REPOSITORY ${address}
                    GIT_TAG main
                )
            endif()

            FetchContent_MakeAvailable(${name})
        endif()
    endif()
endmacro()