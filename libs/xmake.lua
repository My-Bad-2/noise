target("libs-headers")
    set_default(false)
    set_kind("headeronly")

    add_includedirs("$(projectdir)/libs/include", {public = true})
