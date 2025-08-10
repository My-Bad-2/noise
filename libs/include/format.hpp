#ifndef LIBS_FORMAT_HPP
#define LIBS_FORMAT_HPP 1

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <algorithm>
#include <optional>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <utility>

namespace std {
// Otherwise, the Linker imports `std::terminate()` from `libstdc++.a`
inline void terminate() noexcept {
  while (true);
}
}  // namespace std

namespace format {
template <typename T>
concept Sink = requires(T t, const char *str, char ch) {
  t.append(str);
  t.append(ch);
};

namespace details {
enum class format_conversion {
  null,
  character,
  binary,
  octal,
  decimal,
  hex,
};

struct format_spec {
  format_spec() : conversion(format_conversion::null) {
  }

  format_spec with_conv(format_conversion conv) {
    auto copy = *this;
    copy.conversion = conv;
    return copy;
  }

  format_conversion conversion;
  int min_width = 0;
  int arg_pos = -1;
  std::optional<int> precision;
  bool dollar_arg_pos = false;
  bool left_justify = false;
  bool always_sign = false;
  bool plus_becomes_space;
  bool alt_conversion = false;
  bool fill_zeros = false;
  bool group_thousands = false;
  bool use_capitals = false;
};

struct locale_options {
  locale_options() : decimal_point("."), thousands_sep(""), grouping("\255") {
  }

  locale_options(const char *prec, const char *sep, const char *grp)
      : decimal_point(prec), thousands_sep(sep), grouping(grp) {
    thousands_sep_size = strlen(thousands_sep);
  }

  const char *decimal_point;
  const char *thousands_sep;
  const char *grouping;
  size_t thousands_sep_size;
};

template <Sink S, typename T>
  requires(std::is_integral_v<T>)
void print_digits(S &sink, T num, bool neg, int radix, int width, int precision,
                  char padding, bool left_justify, bool group_thousands,
                  bool always_sign, bool plus_becomes_space, bool use_capitals,
                  locale_options options) {
  const char *digits = use_capitals ? "0123456789ABCDEF" : "0123456789abcedf";
  char buffer[64];

  int num_digits = 0;
  int num_chars = 0;
  int group_idx = 0;
  int repeats = 0;
  size_t extra = 0;

  auto step_grouping = [&]() {
    if (!group_thousands) {
      return;
    }

    if (++num_chars == options.grouping[group_idx]) {
      if (options.grouping[group_idx + 1] > 0) {
        group_idx++;
      } else {
        repeats++;
      }

      num_chars = 0;
      extra += options.thousands_sep_size;
    }
  };

  auto emit_grouping = [&]() {
    if (!group_thousands) {
      return;
    }

    if (--num_chars == 0) {
      sink.append(options.thousands_sep);

      if ((repeats == 0) || (--repeats == 0)) {
        group_idx--;
      }

      num_chars = options.grouping[group_idx];
    }
  };

  do {
    buffer[num_digits++] = digits[num % radix];
    num /= radix;
    step_grouping();
  } while (num);

  if (num_digits < precision) {
    for (int i = 0; i < precision - num_digits; i++) {
      step_grouping();
    }
  }

  if (num_chars == 0) {
    num_chars = options.grouping[group_idx];
  }

  int final_width = std::max(num_digits, precision) + extra;

  if (!left_justify && final_width < width) {
    for (int i = 0; i < (width - final_width); i++) {
      sink.append(padding);
    }
  }

  if (neg) {
    sink.append('-');
  } else if (always_sign) {
    sink.append('+');
  } else if (plus_becomes_space) {
    sink.append(' ');
  }

  if (num_digits < precision) {
    for (int i = 0; i < (precision - num_digits); i++) {
      sink.append('0');
      emit_grouping();
    }
  }

  for (int i = num_digits - 1; i >= 0; i--) {
    sink.append(buffer[i]);
    emit_grouping();
  }

  if (left_justify && final_width < width) {
    for (int i = final_width; i < width; i++) {
      sink.append(padding);
    }
  }
}

template <Sink S, typename T>
  requires(std::is_integral_v<T>)
void print_int(S &sink, T num, int radix, int width = 0, int precision = 1,
               char padding = ' ', bool left_justify = false,
               bool group_thousands = false, bool always_sign = false,
               bool plus_becomes_space = false, bool use_capitals = false,
               locale_options options = {}) {
  if (num < 0) {
    auto abs = ~static_cast<typename std::make_unsigned_t<T>>(num) + 1;
    print_digits(sink, abs, true, radix, width, precision, padding,
                 left_justify, group_thousands, always_sign, plus_becomes_space,
                 use_capitals, options);
  } else {
    print_digits(sink, num, false, radix, width, precision, padding,
                 left_justify, group_thousands, always_sign, plus_becomes_space,
                 use_capitals, options);
  }
}

template <Sink S, typename T>
  requires(std::is_integral_v<T>)
void format_integer(S &sink, T num, format_spec spec) {
  int radix = 10;

  if (spec.conversion == format_conversion::hex) {
    radix = 16;
  } else if (spec.conversion == format_conversion::octal) {
    radix = 8;
  } else if (spec.conversion == format_conversion::binary) {
    radix = 2;
  }

  print_int(sink, num, radix, spec.min_width,
            spec.precision ? *spec.precision : 1, spec.fill_zeros ? '0' : ' ',
            spec.left_justify, spec.group_thousands, spec.always_sign,
            spec.plus_becomes_space, spec.use_capitals);
}
}  // namespace details

using format_spec = details::format_spec;

template <Sink S>
void format_obj(S &sink, unsigned int num, format_spec spec) {
  details::format_integer(sink, num, spec);
}

template <Sink S>
void format_obj(S &sink, unsigned long num, format_spec spec) {
  details::format_integer(sink, num, spec);
}

template <Sink S>
void format_obj(S &sink, unsigned char num, format_spec spec) {
  details::format_integer(sink, num, spec);
}

template <Sink S>
void format_obj(S &sink, unsigned short num, format_spec spec) {
  details::format_integer(sink, num, spec);
}

template <Sink S>
void format_obj(S &sink, int num, format_spec spec) {
  details::format_integer(sink, num, spec);
}

template <Sink S>
void format_obj(S &sink, long num, format_spec spec) {
  details::format_integer(sink, num, spec);
}

template <Sink S>
void format_obj(S &sink, short num, format_spec spec) {
  details::format_integer(sink, num, spec);
}

template <Sink S>
void format_obj(S &sink, char ch, format_spec) {
  sink.append(ch);
}

template <Sink S>
void format_obj(S &sink, std::string_view &str, format_spec) {
  for (const auto ch : str) {
    sink.append(ch);
  }
}

template <Sink S>
void format_obj(S &sink, const char *str, format_spec) {
  while (*str) {
    sink.append(*str++);
  }
}

template <Sink S>
void format_obj(S &sink, const void *addr, format_spec spec) {
  sink.append("0x");
  details::format_integer(sink, reinterpret_cast<uintptr_t>(addr),
                          spec.with_conv(details::format_conversion::hex));
}

template <Sink S>
void format_obj(S &sink, std::nullptr_t, format_spec spec) {
  format_obj(sink, static_cast<const void *>(nullptr), spec);
}

template <Sink S, std::ranges::input_range R>
  requires requires {
    typename std::char_traits<std::ranges::range_value_t<R>>;
  }
void format_obj(S &sink, const R &range, format_spec spec) {
  std::ranges::for_each(range, [&](const auto &elem) {
    format_obj(sink, static_cast<char>(elem), spec);
  });
}

template <Sink S, typename T>
void format(S &sink, const T &obj) {
  format_obj(sink, obj, format_spec{});
}

template <Sink S, typename T>
void format(S &sink, const T &obj, format_spec spec) {
  format_obj(sink, obj, spec);
}

struct escape_fmt {
 public:
  escape_fmt(const void *buffer, size_t size) : m_buffer(buffer), m_size(size) {
  }

  template <Sink S>
  friend void format_obj(S &sink, escape_fmt &self, format_spec spec) {
    auto ptr = reinterpret_cast<const unsigned char *>(self.m_buffer);

    for (size_t i = 0; i < self.m_size; i++) {
      unsigned char ch = ptr[i];

      if (isalpha(ch)) {
        sink.append(ch);
      } else if (isdigit(ch)) {
        sink.append(ch);
      } else if (ch == ' ') {
        sink.append(' ');
      } else if (strchr("!#$%&()*+,-./:;<=>?@[]^_`{|}~", ch)) {
        sink.append(ch);
      } else if (ch == '\\') {
        sink.append("\\\\");
      } else if (ch == '\"') {
        sink.append("\\\"");
      } else if (ch == '\'') {
        sink.append("\\\'");
      } else if (ch == '\n') {
        sink.append("\\n");
      } else if (ch == '\t') {
        sink.append("\\t");
      } else {
        sink.append("\\x{");
        format(sink, static_cast<unsigned int>(ch),
               spec.with_conv(details::format_conversion::hex));
        sink.append('}');
      }
    }
  }

 private:
  const void *m_buffer;
  size_t m_size;
};

template <typename... Args>
struct format_impl {
  std::string_view fmt;
  std::tuple<Args...> args;

  template <Sink S>
    requires(sizeof...(Args) > 0)
  bool format_nth(S &sink, size_t n, format_spec spec) const {
    if (n >= sizeof...(Args)) {
      return false;
    }

    return ![&]<size_t... Idx>(std::index_sequence<Idx...>) -> bool {
      return ((Idx == n ? (format(sink, std::get<Idx>(args), spec), false)
                        : true) &&
              ...);
    }(std::make_index_sequence<sizeof...(Args)>{});
  }

  template <Sink S>
  bool format_nth(S &, size_t, format_spec) const {
    return false;
  }

  bool parse(std::string_view fmt, size_t &pos, format_spec spec) const {
    using namespace details;

    enum class modes { pos, fill, width, conv };

    modes mode = modes::pos;
    bool pos_set = false;
    size_t tmp_pos = 0;

    spec.min_width = 0;

    for (size_t i = 0; i < fmt.size(); i++) {
      char ch = fmt[i];

      switch (mode) {
        case modes::pos:
          if (isdigit(ch)) {
            pos_set = true;
            tmp_pos *= 10;
            tmp_pos += ch - '0';
          } else if (ch == ':') {
            mode = modes::fill;
          } else {
            return false;
          }

          break;
        case modes::fill:
          if (ch == '0') {
            spec.fill_zeros = true;
          }

          mode = modes::width;
          [[fallthrough]];
        case modes::width:
          if (isdigit(ch)) {
            spec.min_width *= 10;
            spec.min_width += ch - '0';
          } else {
            switch (ch) {
              case 'b':
                spec.conversion = format_conversion::binary;
                break;
              case 'c':
                spec.conversion = format_conversion::character;
                break;
              case 'o':
                spec.conversion = format_conversion::octal;
                break;
              case 'i':
              case 'd':
                spec.conversion = format_conversion::decimal;
                break;
              case 'X':
                spec.use_capitals = true;
                [[fallthrough]];
              case 'x':
                spec.conversion = format_conversion::hex;
                break;
              default:
                return false;
            }

            mode = modes::conv;
          }
          break;
        case modes::conv:
          return false;
      }
    }

    if (pos_set) {
      pos = tmp_pos;
    }

    return true;
  }

  template <Sink S>
  friend void format_obj(S &sink, const format_impl self, format_spec spec) {
    enum class modes { str, arg };

    size_t curr_arg = 0;
    size_t arg_start = 0;
    size_t arg_end = 0;

    modes mode = modes::str;

    for (size_t i = 0; i < self.fmt.size(); i++) {
      unsigned char ch = self.fmt[i];
      size_t next = (i + 1) < self.fmt.size() ? self.fmt[i + 1] : 0;

      switch (mode) {
        case modes::str: {
          if (ch == '{' && next != '{') {
            mode = modes::arg;
            arg_start = i;
          } else {
            if (ch == '{') {
              i++;
            }

            sink.append(ch);
          }

          break;
        }

        case modes::arg: {
          if (ch == '}') {
            format_spec spec{};
            size_t pos = curr_arg++;

            mode = modes::str;
            arg_end = i;

            if (!self.parse(
                    self.fmt.substr(arg_start + 1, arg_end - arg_start - 1),
                    pos, spec)) {
              format_obj(sink,
                         self.fmt.substr(arg_start, arg_end - arg_start + 1),
                         spec);
              break;
            }

            if (!self.format_nth(sink, pos, spec)) {
              format_obj(sink,
                         self.fmt.substr(arg_start, arg_end - arg_start + 1),
                         spec);
            }
          }

          break;
        }
      }
    }

    if (mode != modes::str) {
      format_obj(sink, self.fmt.substr(arg_start, self.fmt.size() - arg_start),
                 spec);
    }
  }
};

template <typename... Args>
auto fmt(std::string_view fmt, Args &&...args) {
  return format_impl<Args...>{fmt,
                              std::tuple<Args...>{std::forward<Args>(args)...}};
}
}  // namespace format

#endif  // LIBS_FORMAT_HPP
