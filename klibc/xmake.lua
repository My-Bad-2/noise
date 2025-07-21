target("klibc-headers")
    set_default(false)
    set_kind("headeronly")

    add_includedirs("$(projectdir)/klibc/include", {public = true})

target("noise-klibc")
    set_default(false)
    set_kind("static")

    add_files("src/*.cpp")
    add_files("src/*/*.cpp")
    
    add_deps("klibc-headers")

    on_run(function (target)
    end)