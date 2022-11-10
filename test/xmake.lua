add_requires("gtest")
target("api-test")
    set_kind("binary")
    add_files("src/main.cpp")
    add_files("src/test-eastl-lib.cpp")
    add_packages("gtest")  
    add_deps("primitive")  
    add_packages("EASTLLIB")  
    
    -- rules
    add_rules("@xmake-utils/cm-cxflags")
    add_rules("mode-init")
    add_rules("dep-eastl")
target_end()