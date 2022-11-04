
rule("cm-cxflags")
    on_config(function (target)
        local cx_flags = {
            "-Wno-unused-parameter", 
            "-Wno-unused-command-line-argument"
        }

        if target:has_tool("cxx", "clang_cl") then
            target:add("cxxflags", cx_flags)
        end 
        if target:has_tool("cc", "clang_cl") then
            target:add("cflags", cx_flags)
        end
    end)
rule_end()