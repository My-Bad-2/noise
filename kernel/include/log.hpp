#ifndef KERNEL_LOG_HPP
#define KERNEL_LOG_HPP

#include <arch/arch.hpp>

#include <printf_config.h>

#include <atomic>
#include <utility>

#include <printf/printf.h>

#define debug(...) log::write(log::DEBUG, __VA_ARGS__)

#define info(...) log::write(log::INFO, __VA_ARGS__)

#define warning(...) log::write(log::WARNING, __VA_ARGS__)

#define err(...) log::write(log::ERROR, __VA_ARGS__)

#define panic(...) log::write(log::PANIC, __VA_ARGS__)

namespace log {
enum log_level {
  DEBUG,
  INFO,
  WARNING,
  ERROR,
  PANIC,
};

#ifndef LOG_MIN_LEVEL
#define LOG_MIN_LEVEL log::DEBUG
#endif

// Define LOG_DISABLE_COLOR to strip ANSI color sequences.
#ifdef LOG_DISABLE_COLOR
constexpr const char* level_color(log_level) {
  return "";
}

constexpr const char* color_reset() {
  return "";
}
#else
constexpr const char* color_reset() {
  return "\033[0m";
}

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

  return "\033[0m";
}
#endif

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

  return nullptr;
}

// Global monotonically increasing sequence number for log records.
inline std::atomic<unsigned long long> g_log_seq{0};

template <typename... Args>
void write(log_level level, const char* fmt, Args&&... args) {
  if (level < LOG_MIN_LEVEL) {
    return;
  }

  const unsigned long long seq =
      g_log_seq.fetch_add(1ULL, std::memory_order_relaxed);

  // Prefix now only: color + [SEQ][LEVEL]
  printf("%s[%06llu]%s%s%s ", level_color(level), seq, color_reset(),
         level_color(level), level_label(level));

  printf(fmt, std::forward<Args>(args)...);

  arch::write(color_reset());
  arch::write('\n');

  if (level == PANIC) {
    arch::halt(false);
  }
}
}  // namespace log

#endif  // KERNEL_LOG_HPP
