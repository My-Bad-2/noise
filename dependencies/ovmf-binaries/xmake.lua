target("ovmf-binaries")
    set_kind("phony")

    on_build(function (target)
        if is_arch("x86_64") then
            target:set("values", "ovmf-binary", "$(projectdir)/dependencies/ovmf-binaries/edk2-nightly/bin/RELEASEX64_OVMF.fd")
        else
            raise("unknown ovmf architecture")
        end
    end)