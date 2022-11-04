target("primitive")
    set_kind("static")
    add_files("allocator/src/*.cpp")
    add_includedirs("allocator/interface", {public=true})
    
    add_files("utils/src/*.cpp")
    add_includedirs("utils/interface", {public=true})
    
    -- rules
    add_rules("cm-cxflags")
target_end()