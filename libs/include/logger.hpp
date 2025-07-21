#ifndef LIBS_LOGGER_HPP
#define LIBS_LOGGER_HPP 1

#include "format.hpp"

namespace logger {
template <typename Sink>
struct logger {
 public:
  logger(Sink sink = {}) : m_sink(std::move(sink)) {}

  logger(const logger&) = delete;
  logger& operator=(const logger&) = delete;

  ~logger() = default;

  template <typename T>
  void write(T&& obj) {
    format::format(*this, std::forward<T>(obj));
  }

  void append(char ch) {
    constexpr size_t buf_size = sizeof(m_buffer) / sizeof(m_buffer[0]);
    
    if((this->m_offset + 1) == buf_size) {
      this->m_buffer[this->m_offset] = '\0';
      this->flush();
      this->m_offset = 0;
    }

    this->m_buffer[this->m_offset++] = ch;
  }

  void append(const char* str) {
    while (*str) {
      this->append(*str++);
    }
  }

  void flush() {
    this->m_sink(this->m_buffer);
  }

 private:
  Sink m_sink = {};
  char m_buffer[128] = {0};
  size_t m_offset = 0;
};
}  // namespace logger

#endif  // LIBS_LOGGER_HPP