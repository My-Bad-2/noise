target("libstdc++-headers")
    set_default(false)
    set_kind("headeronly")

    add_includedirs(
        "$(projectdir)/dependencies/freestnd-libs/freestnd-libs/include",
        { public = true }
    )

    if is_arch("x86_64") then
        -- TODO: check if `-mno-red-zone` flag is set or not before using this directory
        add_includedirs(
            "$(projectdir)/dependencies/freestnd-libs/freestnd-libs/include/x86_64-elf/no-red-zone/",
            { public = true }
        )
    end

-- TODO: Find a way to link crt files in the correct order
-- target("crti")
--     set_default(false)
--     set_kind("object")

--     if is_arch("x86_64") then
--         add_files("freestnd-libs/src/x86_64/crti.s")
--     end

-- target("crtn")
--     set_default(false)
--     set_kind("object")

--     if is_arch("x86_64") then
--         add_files("freestnd-libs/src/x86_64/crtn.s")
--     end

target("libs")
    set_default(false)
    set_kind("phony")

    if is_arch("x86_64") then
        local path = "$(projectdir)/dependencies/freestnd-libs/freestnd-libs/lib/x86_64"
        set_values("lib_path", path)
    end