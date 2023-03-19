target("primitive")
    set_kind("static")
    add_files("allocator/src/*.cpp")
    add_files("utils/src/*.cpp")
    add_includedirs("./interface", {public=true})
    
    add_packages("EASTLLIB")  
    
    -- rules
    add_rules("cm-cxflags")
    add_rules("mode-init")
    add_rules("dep-eastl")
target_end()
