set_project("LPrimitive")

set_version("0.0.1")

add_rules("mode.debug", "mode.release", "mode.releasedbg")

set_languages("c99", "c++20")
set_warnings("all", "error")

includes("xmake/options.lua")
includes("xmake/rules.lua")
if is_plat("windows") then
    add_defines("_WINDOWS")
    add_defines("NOMINMAX")
    add_defines("UNICODE")
    add_defines("_UNICODE")
    if is_mode("release") then
        set_runtimes("MD")
    else
        set_runtimes("MDd")
    end
end

includes("src/xmake.lua")

if has_config("build-tests") then
    includes("test/xmake.lua")
end