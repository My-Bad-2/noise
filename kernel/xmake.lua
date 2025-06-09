target("noise-headers")
    set_kind("headeronly")
    add_includedirs(
        "$(projectdir)/kernel/include",
        { public = true }
    )

target("noise-dependencies-nolink")
    set_kind("phony")
   
    add_deps("limine-headers")
    add_deps("klibc-headers")
    add_deps("noise-headers")
    add_deps("libstdc++-headers")

target("noise.elf")
    set_default(false)
    set_kind("binary")

    add_files("src/**.S")
    add_files("src/**.cpp")

    add_deps("noise-klibc")
    add_deps("noise-dependencies-nolink")

    add_deps("libs")

    if not is_arch("x86_64") then
        -- remove files from other archs
    end

    if is_arch("x86_64") then
        local flags = {
            "-masm=intel"
        }
      
        add_cxflags(flags, { force = true })
        add_asflags(flags, { force = true })

        add_ldflags(
            "-T" .. "$(projectdir)/kernel/linker-x86_64.ld",
            "-Map=$(projectdir)/build/noise.map",
            { force = true }
        )
    end

    after_load(function (target)
        local libs_dep = target:deps()["libs"]
        local libs_path = libs_dep:get("values")["lib_path"]

        target:add("linkdirs", libs_path)
        target:add("links", "stdc++", "gcc")
    end)

    on_run(function (target)
    end)