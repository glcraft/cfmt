includes("xmake/**.lua")

add_rules("mode.debug", "mode.release")

option("asm")
    set_default(false)
    set_showmenu(true)
    set_description("Output assembly code")

target("cstfmt")
    set_kind("headeronly")
    set_languages("cxx20")
    add_includedirs("include", {public = true})
    add_headerfiles("include/(cstfmt/**.h)", "include/(cstfmt/**.inl)")

target("cstfmt_test")
    add_deps("cstfmt")
    set_languages("cxx20")
    add_files("src/test.cpp")
    if has_config("asm") then 
        if not is_plat("windows") then
            add_cxflags("-S")
        else
            add_cxflags("/FA")
        end
        set_kind("phony")
    else
        set_kind("binary")
    end