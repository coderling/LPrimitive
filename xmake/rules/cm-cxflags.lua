
rule("cm-cxflags")
    on_config(function (target)
        local cx_flags = {
            "-Wno-unused-parameter", 
            "-Wno-unused-command-line-argument",
            "-Wno-unused-function"
        }
        -- clang cl
        if target:has_tool("cxx", "clang_cl")  then
            target:add("cxxflags", cx_flags)
        end 
        if target:has_tool("cc", "clang_cl") then
            target:add("cflags", cx_flags)
        end
        
        -- msvc
        if target:has_tool("cxx", "cl")  then
            target:add("cxxflags", "/wd4819", "/Zc:preprocessor")
        end 
        if target:has_tool("cc", "cl") then
            target:add("cflags", "/wd4819", "/Zc:preprocessor")
        end
    end)
rule_end()
