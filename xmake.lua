includes("external/EXAGE")

add_rules("mode.debug", "mode.release", "mode.releasedbg")

set_project("PTH")
set_version("0.1.0")
set_languages("c++20")

target("PTH")
    set_kind("static")

    add_deps("EXAGE")

    add_files("src/**.cpp")
    add_headerfiles("src/**.h", {public = false})

    add_headerfiles("include/**.h")
    add_includedirs("include", {public = true})

    if is_mode("debug") or is_mode("releasedbg") then
        add_defines("PTH_DEBUG")
    else
	    add_defines("PTH_RELEASE")
    end

    if is_plat("windows") then
        add_defines("PTH_WINDOWS")
    end

    if is_plat("macosx") then
        add_defines("PTH_MACOS")
    end

    if is_plat("linux") then
        add_defines("PTH_LINUX")
    end

    if is_plat("android") then
        add_defines("PTH_ANDROID")
    end

    set_policy("build.optimization.lto", true)

includes("exec/xmake.lua")