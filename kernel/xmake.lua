target("noise.headers")
    set_kind("headeronly")
    add_includedirs(
        "$(projectdir)/kernel/include",
        { public = true }
    )

target("noise.dependencies.base")
    set_kind("phony")

    add_deps("noise.headers")
    
    if is_arch("x86_64") then 
        local flags = {
            "-masm=intel"
        }
      
        add_cxflags(flags, { force = true, public = true })
        add_asflags(flags, { force = true, public = true })
    end

target("noise.dependencies")
    set_kind("phony")

    add_deps("noise.dependencies.base")

target("noise.dependencies.nolink")
    set_kind("phony")
   
    add_deps("noise.dependencies.base")
    add_deps("limine-headers")

target("noise.elf")
    set_default(false)
    set_kind("binary")

    add_files("src/**.S")
    add_files("src/**.c")
    add_files("src/**.cpp")

    if not is_arch("x86_64") then
        -- remove files from other archs
    end

    if is_arch("x86_64") then
        add_ldflags(
            "-T" .. "$(projectdir)/kernel/linker-x86_64.ld",
            { force = true }
        )
    end

    on_run(function (target)
    end)