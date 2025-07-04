target("limine-headers")
    set_kind("headeronly")
    add_includedirs("limine", { public = true })

target("limine")
    set_default(false)
    set_kind("binary")

    add_includedirs("limine", { public = true })

    on_config(function (target)
        local binaries = {
            "$(projectdir)/dependencies/limine/limine/limine-uefi-cd.bin"
        }
        local uefi_binaries = { }

        if is_arch("x86_64") then
            table.insert(binaries, "$(projectdir)/dependencies/limine/limine/limine-bios.sys")
            table.insert(binaries, "$(projectdir)/dependencies/limine/limine/limine-bios-cd.bin")
            table.insert(uefi_binaries, "$(projectdir)/dependencies/limine/limine/BOOTX64.EFI")
        else
            raise("unknown limine architecture")
        end

        target:set("values", "binaries", binaries)
        target:set("values", "uefi-binaries", uefi_binaries)
    end)

    on_build(function (target)
        local cc = import("lib.detect.find_tool")("clang")

        if cc == nil then
            cc = import("lib.detect.find_tool")("gcc")
        end

        if cc == nil then
            raise("C compiler not found for building limine executable!")
        end

        os.mkdir(path.join(os.projectdir(), path.directory(target:targetfile())))
        os.execv(cc["program"], {
            path.join(os.projectdir(), "dependencies/limine/limine/limine.c"),
            "-o", path.join(os.projectdir(), target:targetfile())
        })
    end)