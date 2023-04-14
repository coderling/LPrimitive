target("primitive")
    set_kind("static")
    add_files("memory/private/*.cpp")
    add_files("utils/private/*.cpp")
    
    -- rules
    add_rules("cm-cxflags")
    add_rules("mode-init")
target_end()
