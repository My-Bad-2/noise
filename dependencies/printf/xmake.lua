option("support_decimal_specifiers")
    set_default(false)
    set_showmenu(true)
    set_description("Support decimal notation floating-point specifiers (%f,%F).")

option("support_exponential_specifiers")
    set_default(false)
    set_showmenu(true)
    set_description("Support exponential floating point specifiers (%e,%E,%g,%G).")

option("support_msvc_style_integer_specifiers")
    set_default(true)
    set_showmenu(true)
    set_description("Support MSVC-style integer specifiers (%I8, %I16, %I32, %I64).")

option("support_writeback_specifier")
    set_default(true)
    set_showmenu(true)
    set_description("Support the length write-back specifier (%n).")

option("support_long_long")
    set_default(true)
    set_showmenu(true)
    set_description("Support long long integral types.")

option("use_double_internally")
    set_default(false)
    set_showmenu(true)
    set_description("Use C `double` type for internal floating-point arithmetic.")

option("check_for_nul_in_format_specifier")
    set_default(true)
    set_showmenu(true)
    set_description("Be defensive when a format specifier is not NUL-terminated.")

option("alias_standard_function_names")
    set_default("SOFT")
    set_showmenu(true)
    set_description("Enable for Hard aliasing and disable for soft alias")
    set_values("HARD", "SOFT")

option("integer_buffer_size")
    set_default("32")
    set_showmenu(true)
    set_description("Integer to string conversion buffer size.")

option("decimal_buffer_size")
    set_default("32")
    set_showmenu(true)
    set_description("Floating-point to decimal conversion buffer size.")

option("default_float_precision")
    set_default("6")
    set_showmenu(true)
    set_description("Default precision when printing floating-point values.")

option("max_integral_digits_for_decimal")
    set_default("9")
    set_showmenu(true)
    set_description("Max integral digits for %f to use non-exponential notation.")

option("log10_taylor_terms")
    set_default("4")
    set_showmenu(true)
    set_description("Number of terms in Taylor series expansion of log_10(x).")

target("printf")
    set_default(false)
    set_kind("static")

    add_files("printf/src/printf/*.c")

    local configs = {}
    local bool_options = {
        "support_decimal_specifiers", "support_exponential_specifiers",
        "support_msvc_style_integer_specifiers", "support_writeback_specifier", "support_long_long",
        "use_double_internally", "check_for_nul_in_format_specifier"
    }

    for _, opt in ipairs(bool_options) do
        configs["PRINTF_" .. opt:upper()] = get_config(opt) and 1 or 0
    end

    local alias_mode = get_config("alias_standard_function_names")
    configs["PRINTF_ALIAS_STANDARD_FUNCTION_NAMES_SOFT"] = (alias_mode == "SOFT") and 1 or 0
    configs["PRINTF_ALIAS_STANDARD_FUNCTION_NAMES_HARD"] = (alias_mode == "HARD") and 1 or 0

    configs["PRINTF_INTEGER_BUFFER_SIZE"] = get_config("integer_buffer_size")
    configs["PRINTF_DECIMAL_BUFFER_SIZE"] = get_config("decimal_buffer_size")
    configs["DEFAULT_FLOAT_PRECISION"] = get_config("default_float_precision")
    configs["MAX_INTEGRAL_DIGITS_FOR_DECIMAL"] = get_config("max_integral_digits_for_decimal")
    configs["LOG10_TAYLOR_TERMS"] = get_config("log10_taylor_terms")

    add_configfiles("printf/printf_config.h.in", {
        filename = "printf/printf_config.h",
        variables  = configs,
        pattern = "@(.-)@"
    })

    add_includedirs("$(builddir)/printf", {public = true})
    add_includedirs("printf/src", {public = true})

    add_defines("PRINTF_INCLUDE_CONFIG_H=1")
