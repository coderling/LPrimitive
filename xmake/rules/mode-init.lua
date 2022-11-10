rule("mode-init")
    on_config(function(target)
        if is_mode("debug")  then
            target:add("defines", "DEBUG")
            target:add("defines", "_DEBUG")
            target:add("defines", "CDL_DEBUG")
            target:add("defines", "CDL_DEVELOPMENT")
            target:add("defines", "CDL_NAME_ENABLED")
        elseif is_mode("releasedbg") then
            target:add("defines", "CDL_DEVELOPMENT")
            target:add("defines", "CDL_NAME_ENABLED")
        elseif is_mode("release") then
            target:add("defines", "NDEBUG")
            target:add("defines", "RELEASE")
            target:add("defines", "CDL_RELEASE")
        end
    end)
rule_end()