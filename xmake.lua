includes("xmake/**.lua")

add_rules("mode.debug", "mode.release")

option("asm")
    set_default(false)
    set_showmenu(true)
    set_description("Output assembly code (example only)")

target("cfmt")
    set_kind("headeronly")
    set_languages("cxx20")
    add_includedirs("include", {public = true})
    add_headerfiles("include/(cfmt/**.h)")

target("cfmt-example")
    set_kind("binary")
    add_deps("cfmt")
    set_languages("cxx20")
    set_default(false)
    add_files("src/example.cpp")
    if has_config("asm") then 
        if not is_plat("windows") then
            add_cxflags("-S")
        else
            add_cxflags("/FA")
        end
    end