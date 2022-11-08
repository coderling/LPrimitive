target("primitive")
    set_kind("static")
    add_files("allocator/src/*.cpp")
    add_includedirs("allocator/interface", {public=true})
    
    add_files("utils/src/*.cpp")
    add_includedirs("utils/interface", {public=true})
    
    add_headerfiles("allocator/interface/*.hpp", {prefixdir = "primitive"})
    add_headerfiles("utils/interface/*.hpp", {prefixdir = "primitive"})
    -- rules
    add_rules("@xmake-utils/cm-cxflags")
    add_rules("@xmake-utils/unit-test")

    on_load(function(target) 
        import("xmake-utils.foo")
    end)
target_end()