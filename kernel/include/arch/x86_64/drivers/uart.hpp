#ifndef ARCH_DRIVERS_UART_HPP
#define ARCH_DRIVERS_UART_HPP 1

#include "drivers/manager.hpp"

#include <stdint.h>

namespace drivers {

constexpr uint16_t PORT_A = 0x3f8;
constexpr uint16_t PORT_B = 0x2f8;
constexpr uint16_t PORT_C = 0x3e8;
constexpr uint16_t PORT_D = 0x2e8;

class UartDriver final : public IDriver {
 public:
  UartDriver() : IDriver(), m_port(PORT_A) {
  }

  UartDriver(const uint16_t port) : IDriver(), m_port(port) {
  }

  bool initialize() override;
  void shutdown() override;

  void putchar(uint8_t ch) const;

  constexpr void set_port(const uint16_t port) {
    this->m_port = port;
  }

 private:
  void write(uint16_t reg, uint8_t val) const;
  uint8_t read(uint16_t reg) const;

  uint16_t m_port;
};
}  // namespace drivers

#endif  // ARCH_DRIVERS_UART_HPP