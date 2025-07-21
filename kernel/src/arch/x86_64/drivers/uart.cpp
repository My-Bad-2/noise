#include "arch/x86_64/cpu/cpu.hpp"
#include "arch/x86_64/cpu/io.hpp"
#include "arch/x86_64/drivers/uart.hpp"
#include "uart_internal.hpp"

namespace drivers {
using namespace arch::x86_64;
using namespace drivers::uart;

void UartDriver::write(uint16_t reg, uint8_t val) const {
  cpu::out(this->m_port + reg, val);
}

uint8_t UartDriver::read(uint16_t reg) const {
  return cpu::in<uint8_t>(this->m_port + reg);
}

void UartDriver::putchar(uint8_t ch) const {
  while (!(this->read(LINE_STATUS) & LINE_TRANSMITTER_BUF_EMPTY)) {
    cpu::pause();
  }

  this->write(DATA, ch);
}

bool UartDriver::initialize() {
  this->write(INTERRUPT_IDENTIFACTOR, 0);

  this->write(LINE_CONTROL, LINE_DLAB_STATUS);
  this->write(BAUD_RATE_LOW, 3);
  this->write(BAUD_RATE_HIGH, 0);
  this->write(LINE_CONTROL, LINE_DS_8);

  const uint8_t fifo_flags = ENABLE_FIFO | FIFO_CLEAR_RECEIVE |
                             FIFO_CLEAR_TRANSMIT | FIFO_TRIGGER_LEVEL4;

  this->write(FIFO_CONTROLLER, fifo_flags);
  this->write(MODEM_CONTROL,
              MODEM_RTS | MODEM_DTR | MODEM_OUT2 | MODEM_LOOPBACK);
  this->write(MODEM_CONTROL, MODEM_RTS | MODEM_DTR | MODEM_OUT2 | MODEM_OUT1);

  this->write(DATA, 0xae);
  bool test = (this->read(DATA) == 0xae);

  // Backspace to remove the undefined ASCII value `0xae`
  this->write(DATA, 0x8);

  return test;
}

void UartDriver::shutdown() {}
}  // namespace drivers