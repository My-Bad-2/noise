#ifndef KERNEL_LOG_HPP
#define KERNEL_LOG_HPP

#include <arch/arch.hpp>

#include <printf_config.h>

#include <source_location>
#include <utility>

#include <printf/printf.h>

#define debug(...) \
  log::write(log::DEBUG, std::source_location::current(), __VA_ARGS__)

#define info(...) \
  log::write(log::INFO, std::source_location::current(), __VA_ARGS__)

#define warning(...) \
  log::write(log::WARNING, std::source_location::current(), __VA_ARGS__)

#define error(...) \
  log::write(log::ERROR, std::source_location::current(), __VA_ARGS__)

#define panic(...) \
  log::write(log::PANIC, std::source_location::current(), __VA_ARGS__)

namespace log {
enum log_level {
  DEBUG,
  INFO,
  WARNING,
  ERROR,
  PANIC,
};

constexpr const char* level_color(log_level level) {
  switch (level) {
    case DEBUG:
      return "\033[90m";
    case INFO:
      return "\033[96m";
    case WARNING:
      return "\033[93m";
    case ERROR:
      return "\033[91m";
    case PANIC:
      return "\033[1;91m";
  }
}

constexpr const char* level_label(log_level level) {
  switch (level) {
    case DEBUG:
      return "[DEBUG  ]";
    case INFO:
      return "[INFO   ]";
    case WARNING:
      return "[WARNING]";
    case ERROR:
      return "[ERROR  ]";
    case PANIC:
      return "[PANIC  ]";
  }
}

template <typename... Args>
void write(log_level level, std::source_location loc, const char* str,
           Args&&... args) {
  printf(level_color(level));
  printf(level_label(level));

  printf("[%s] [%s] ", loc.file_name(), loc.function_name());
  printf(str, std::forward<Args>(args)...);

  arch::write("\033[0m");
  arch::write('\n');

  if (level == PANIC) {
    arch::halt(false);
  }
}
}  // namespace log

#endif  // KERNEL_LOG_HPP
