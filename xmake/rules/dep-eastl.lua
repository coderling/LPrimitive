rule("dep-eastl")
    on_config(function(target)
        if is_mode("debug") or is_mode("releasedbg") then
            target:add("defines", "EASTL_NAME_ENABLED")
        end
        
        target:add("defines", "EASTL_USER_DEFINED_ALLOCATOR")
    end)
rule_end()