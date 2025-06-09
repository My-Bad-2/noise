set_project("noise")
set_version("v0.0.1")

set_license("GPL-3")

add_rules("plugin.compile_commands.autoupdate", { outputdir = "build" })

set_policy("run.autobuild", true)
set_policy("package.install_locally", true)

set_languages("cxxlatest", "clatest")

set_allowedarchs("x86_64")
set_defaultplat("x86_64")

-- debug
if is_mode("debug") then 
    set_symbols("debug")
    set_optimize("none")
-- optimized debug
elseif is_mode("releasedbg") then 
    set_symbols("debug")
    set_optimize("fastest")
elseif is_mode("release") then 
    set_optimize("fastest")
end

set_defaultmode("releasedbg")

-- options

option("qemu_gdb")
    set_default(false)
    set_showmenu(true)
    set_description("Pass '-s -S' to QEMU when debugging")

option("qemu_accel")
    set_default(true)
    set_showmenu(true)
    set_description("QEMU accelerators. Disabled when debugging")

option("qemu_memory")
    set_default("512M")
    set_showmenu(true)
    set_description("QEMU memory amount")

option("extra_cflags")
    set_default("")
    set_showmenu(true)
    set_description("Extra CFLAGS")

option("extra_cxxflags")
    set_default("")
    set_showmenu(true)
    set_description("Extra CXXFLAGS")

option("extra_qemuflags")
    set_default("")
    set_showmenu(true)
    set_description("Extra QEMU flags")

option("qemu_vnc")
    set_default(false)
    set_showmenu(true)
    set_description("Start headless QEMU VNC server on localhost:5901")

-- variables

local function multi_insert(list, ...)
    for idx, val in ipairs({ ... }) do
        list[#list + 1] = val
    end
end

local function get_targetfile(tdir, prefix, ext)
    local ret = path.join(tdir, prefix .. "-" .. get_config("arch") .. ext)
    return ret
end

local logfile = os.projectdir() .. "/log.txt"

local qemu_args = {
    "-rtc", "base=localtime", "-serial", "stdio",
    "-boot", "order=d,menu=on,splash-time=0"
}

local qemu_accel_args = {
    "-M", "accel=kvm:hvf:whpx:haxm:tcg"
}

local qemu_dbg_args = {
    "-no-reboot", "-no-shutdown",
    "-d", "int", "-D", logfile,
}

local bios = false

-- toolchain

toolchain("kernel-clang")
    set_kind("standalone")

    set_toolset("as", "clang")
    set_toolset("cc", "clang")
    set_toolset("cxx", "clang++")
    set_toolset("sh", "clang++", "clang")

    set_toolset("ld", "ld.lld", "lld")
    
    set_toolset("ar", "llvm-ar", "ar")
    set_toolset("strip", "llvm-strip", "strip")
    
    add_defines("LIMINE_API_REVISION=2")

    on_load(function (toolchain)
        local cx_args = {
            "-ffreestanding",
            "-fno-stack-protector",
            "-fno-omit-frame-pointer",
            "-fno-strict-aliasing",
            "-fstrict-vtable-pointers",
            "-fno-pic",
            "-mno-red-zone",
        }

        local c_args = {}

        local cxx_args = {
            "-fno-rtti",
            "-fno-exceptions",
            "-fsized-deallocation",
            "-fcheck-new",
        }

        local ld_args = {
            "-nostdlib",
            "-static",
            "-znoexecstack",
            "-zmax-page-size=0x1000",
        }

        local sh_args = {
            "-fuse-ld=lld",
            "-Wl,-shared"
        }

        local target = ""

        if is_mode("releasesmall") or is_mode("release") then 
            toolchain:add("defines", "NOISE_DEBUG=0");
        else 
            toolchain:add("defines", "NOISE_DEBUG=1");
        end

        if is_arch("x86_64") then 
            target = "x86_64-elf"

            multi_insert(cx_args,
                "-march=x86-64",
                "-mno-red-zone",
                "-mno-mmx",
                "-mno-sse",
                "-mno-sse2",
                "-mno-80387",
                "-mcmodel=kernel"
            )
        end

        table.insert(cx_args, "--target=" .. target)
        table.insert(sh_args, "--target=" .. target)

        table.insert(c_args, get_config("extra_cflags"))
        table.insert(cxx_args, get_config("extra_cxxflags"))

        toolchain:add("cxxflags", cxx_args, { force = true})
        toolchain:add("cxflags", cx_args, { force = true})
        toolchain:add("cflags", c_args, { force = true})

        toolchain:add("asflags", cxx_args, { force = true })
        toolchain:add("asflags", cx_args, { force = true })
        toolchain:add("asflags", c_args, { force = true })

        toolchain:add("ldflags", ld_args, { force = true })
        toolchain:add("shflags", sh_args, { force = true })
    end)
toolchain_end()

toolchain("gcc-x86_64-elf")
    set_kind("standalone")

    set_toolset("as", "x86_64-elf-gcc")
    set_toolset("cc", "x86_64-elf-gcc")
    set_toolset("cxx", "x86_64-elf-g++")
    set_toolset("sh", "x86_64-elf-g++", "x86_64-elf-gcc")

    set_toolset("ld", "x86_64-elf-ld")
    
    set_toolset("ar", "x86_64-elf-ar", "ar")
    set_toolset("strip", "x86_64-elf-strip", "strip")
    
    add_defines("LIMINE_API_REVISION=2")

    on_load(function (toolchain)
        local cx_args = {
            "-ffreestanding",
            "-fno-stack-protector",
            "-fno-omit-frame-pointer",
            "-fno-strict-aliasing",
            "-fno-pic",
            "-mno-red-zone",
        }

        local c_args = {}

        local cxx_args = {
            "-fno-rtti",
            "-fno-exceptions",
            "-fsized-deallocation",
            "-fcheck-new",
        }

        local ld_args = {
            "-nostdlib",
            "-static",
            "-znoexecstack",
            "-zmax-page-size=0x1000",
        }

        local sh_args = {
            "-Wl,-shared"
        }

        if is_mode("releasesmall") or is_mode("release") then 
            toolchain:add("defines", "NOISE_DEBUG=0");
        else 
            toolchain:add("defines", "NOISE_DEBUG=1");
        end

        if is_arch("x86_64") then 
            multi_insert(cx_args,
                "-march=x86-64",
                "-mno-red-zone",
                "-mno-mmx",
                "-mno-sse",
                "-mno-sse2",
                "-mno-80387",
                "-mcmodel=kernel"
            )
        end

        table.insert(c_args, get_config("extra_cflags"))
        table.insert(cxx_args, get_config("extra_cxxflags"))

        toolchain:add("cxxflags", cxx_args, { force = true})
        toolchain:add("cxflags", cx_args, { force = true})
        toolchain:add("cflags", c_args, { force = true})

        toolchain:add("asflags", cxx_args, { force = true })
        toolchain:add("asflags", cx_args, { force = true })
        toolchain:add("asflags", c_args, { force = true })

        toolchain:add("ldflags", ld_args, { force = true })
        toolchain:add("shflags", sh_args, { force = true })
    end)
toolchain_end()

set_toolchains("gcc-x86_64-elf")

-- dependencies

includes("dependencies/xmake.lua")

-- targets.build

includes("klibc/xmake.lua")
includes("kernel/xmake.lua")

target("iso")
    set_default(true)
    set_kind("phony")
    add_deps("limine", "ovmf-binaries", "noise.elf")

    on_clean(function (target)
        os.rm(get_targetfile(target:targetdir(), "image", ".iso"))
    end)

    on_build(function (target) 
        import("core.project.project")
        import("core.project.depend")
        import("lib.detect.find_program")

        targetfile = get_targetfile(target:targetdir(), "image", ".iso")
        target:set("values", "targetfile", targetfile)

        local kernel = project.target("noise.elf")
        
        local iso_dirname = "noise.iso.dir"
        local iso_dir = path.join(os.tmpdir(), iso_dirname)
        local iso_dir_b = path.join(iso_dir, "boot")
        local iso_dir_bl = path.join(iso_dir, "boot/limine")
        local iso_dir_eb = path.join(iso_dir, "EFI/BOOT")

        local limine_dep = target:deps()["limine"]
        local limine_exec = limine_dep:targetfile()

        local binaries = limine_dep:get("values")["binaries"]
        local uefi_binaries = limine_dep:get("values")["uefi-binaries"]

        local limine_files = {
            "$(projectdir)/misc/limine.conf"
        }

        local xorriso_args = {
            "-as", "mkisofs"
        }

        if is_arch("x86_64") then
            multi_insert(xorriso_args,
                "-b", "boot/limine/limine-bios-cd.bin",
                "-no-emul-boot", "-boot-load-size", "4",
                "-boot-info-table"
            )
        end
        
        multi_insert(xorriso_args,
            "--efi-boot", "boot/limine/limine-uefi-cd.bin",
            "-efi-boot-part", "--efi-boot-image",
            "--protective-msdos-label"
        )

        local kernelfile = kernel:targetfile()
        local created = false

        local function create_iso()
            os.tryrm(targetfile)
            os.tryrm(iso_dir)

            os.mkdir(iso_dir)
            os.mkdir(iso_dir_b)
            os.mkdir(iso_dir_bl)
            os.mkdir(iso_dir_eb)

            print(" => copying target files to temporary iso directory...")

            for idx, val in ipairs(limine_files) do
                os.cp(val, iso_dir_bl)
            end

            for idx, val in ipairs(binaries) do
                os.cp(val, iso_dir_bl)
            end

            for idx, val in ipairs(uefi_binaries) do
                os.cp(val, iso_dir_eb)
            end

            os.cp(kernelfile, path.join(iso_dir_b, "kernel.elf"))

            multi_insert(xorriso_args,
                iso_dir, "-o", targetfile
            )

            print(" => building the iso...")
            os.execv(find_program("xorriso"), xorriso_args)

            print(" => installing limine...")
            os.execv(limine_exec, { "bios-install", targetfile })

            created = true
            os.tryrm(iso_dir)
        end

        depend.on_changed(create_iso, { files = { kernelfile } })

        if not created and not os.isfile(targetfile) then
            create_iso()
        end
    end)

-- targets.run

task("qemu")
    on_run(function ()
        import("core.base.option")
        import("core.project.project")
        import("lib.detect.find_program")

        local extra_qemu_args = get_config("extra_qemuflags")
        if extra_qemu_args ~= "" then
            table.insert(qemu_args, extra_qemu_args)
        end

        multi_insert(qemu_args, "-m", get_config("qemu_memory"))

        if get_config("qemu_vnc") then
            multi_insert(qemu_args,
                "-vnc", "127.0.0.1:1"
            )
        end

        local qemu_exec = ""
        if is_arch("x86_64") then
            bios = true

            multi_insert(qemu_args,
                "-cpu", "max,migratable=off,+invtsc,+tsc-deadline", "-M", "q35,smm=off"
                -- "-audiodev", "id=audio,driver=alsa", "-machine", "pcspk-audiodev=audio"
            )

            qemu_exec = find_program("qemu-system-x86_64")
        end

        if not option.get("uefi") and not bios then
            raise("BIOS not supported on this architecture")
        end

        if option.get("debug") then
            multi_insert(qemu_args, unpack(qemu_dbg_args))
            if get_config("qemu_gdb") then
                multi_insert(qemu_args,
                    "-s", "-S"
                )
            end
        elseif get_config("qemu_accel") then
            multi_insert(qemu_args, unpack(qemu_accel_args))
        end

        local iso = project.target("iso")
        local ovmf_fd = iso:deps()["ovmf-binaries"]:get("values")["ovmf-binary"]

        multi_insert(qemu_args,
            "-cdrom", iso:get("values", "targetfile")["targetfile"]
        )

        if option.get("uefi") then
            multi_insert(qemu_args,
                "-bios", ovmf_fd
            )
        end

        print(" => running qemu...")
        os.execv(qemu_exec, qemu_args)
    end)

target("bios")
    set_default(false)
    set_kind("phony")
    add_deps("iso")

    on_run(function (target)
        import("core.project.task")
        import("core.project.project")
        task.run("qemu", { uefi = false })
    end)

target("bios-debug")
    set_default(false)
    set_kind("phony")
    add_deps("iso")

    on_run(function (target)
        import("core.project.task")
        import("core.project.project")
        task.run("qemu", { uefi = false, debug = true })
    end)

target("uefi")
    set_default(true)
    set_kind("phony")
    add_deps("iso")

    on_run(function (target)
        import("core.project.task")
        import("core.project.project")
        task.run("qemu", { uefi = true })
    end)

target("uefi-debug")
    set_default(false)
    set_kind("phony")
    add_deps("iso")

    on_run(function (target)
        import("core.project.task")
        import("core.project.project")
        task.run("qemu", { uefi = true, debug = true })
    end)