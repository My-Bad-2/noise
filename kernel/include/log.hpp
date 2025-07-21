#ifndef KERNEL_LOG_HPP
#define KERNEL_LOG_HPP

#include <arch/arch.hpp>
#include <logger.hpp>

#include <source_location>

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
struct log_sink {
  void operator()(char ch) {
    arch::write(ch);
  }

  void operator()(const char* str) {
    arch::write(str);
  }
};

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
void write(log_level level, std::source_location loc, std::string_view str,
           Args&&... args) {
  log_sink sink = {};
  logger::logger logger = {sink};

  sink(level_color(level));
  sink(level_label(level));

  auto fmt = format::fmt("[{}] [{}] ", loc.file_name(), loc.function_name());
  logger.write(fmt);

  auto fmt1 = format::fmt(str, std::forward<Args>(args)...);
  logger.write(fmt1);

  logger.flush();

  sink("\033[0m");
  sink('\n');

  if (level == PANIC) {
    arch::halt(false);
  }
}
}  // namespace log

#endif  // KERNEL_LOG_HPP