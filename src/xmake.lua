target("primitive")
    set_kind("static")
    add_files("allocator/src/*.cpp")
    add_includedirs("allocator/interface", {public=true})
    
    add_files("utils/src/*.cpp")
    add_includedirs("utils/interface", {public=true})
    
    add_headerfiles("allocator/interface/*.hpp", {prefixdir = "primitive"})
    add_headerfiles("utils/interface/*.hpp", {prefixdir = "primitive"})
    
    add_packages("EASTLLIB")  
    
    -- rules
    add_rules("@xmake-utils/cm-cxflags")
    add_rules("mode-init")
    add_rules("dep-eastl")
    on_load(function(target) 
        import("xmake-utils.foo")
    end)
target_end()