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
  // Wait until the THR (Transmit Holding Register) is empty, then write byte.
  while (!(this->read(LINE_STATUS) & LINE_TRANSMITTER_BUF_EMPTY)) {
    cpu::pause();
  }

  this->write(DATA, ch);
}

bool UartDriver::initialize() {
  // 16550 init sequence:
  // 1) Disable interrupts
  this->write(INTERRUPT, 0x00);  // IER = 0 (disable UART interrupts)

  // 2) Set baud rate via divisor latch (DLAB)
  this->write(LINE_CONTROL, LINE_DLAB_STATUS);  // LCR: set DLAB=1
  // Baud divisor for 115200 base clock; divisor=3 -> 38400 baud (example)
  this->write(BAUD_RATE_LOW, 3);
  this->write(BAUD_RATE_HIGH, 0);

  // 3) 8 data bits, no parity, 1 stop bit (8N1), clear DLAB
  this->write(LINE_CONTROL, LINE_DS_8);

  // 4) Enable FIFO, clear RX/TX FIFOs, set highest trigger level
  const uint8_t fifo_flags = ENABLE_FIFO | FIFO_CLEAR_RECEIVE |
                             FIFO_CLEAR_TRANSMIT | FIFO_TRIGGER_LEVEL4;
  this->write(FIFO_CONTROLLER, fifo_flags);

  // 5) Modem control: DTR/RTS asserted, OUT2 (often needed to enable IRQs)
  this->write(MODEM_CONTROL, MODEM_RTS | MODEM_DTR | MODEM_OUT2);

  // 6) Loopback self-test: enable loopback, send test byte, verify it reads
  // back
  this->write(MODEM_CONTROL,
              MODEM_RTS | MODEM_DTR | MODEM_OUT2 | MODEM_LOOPBACK);

  // Ensure TX is ready before sending the test byte
  size_t spins = 0;
  constexpr size_t max_spins = 1 << 20;
  while (!(this->read(LINE_STATUS) & LINE_TRANSMITTER_BUF_EMPTY) &&
         spins++ < max_spins) {
    cpu::pause();
  }

  constexpr uint8_t self_test_val = 0xae;
  this->write(DATA, self_test_val);

  // Wait for data to be received back in loopback
  spins = 0;
  while (!(this->read(LINE_STATUS) & LINE_DATA_READY) && spins++ < max_spins) {
    cpu::pause();
  }

  const bool test_ok = (this->read(DATA) == self_test_val);

  // 7) Restore normal operation (disable loopback, keep DTR/RTS/OUT2)
  this->write(MODEM_CONTROL, MODEM_RTS | MODEM_DTR | MODEM_OUT2);

  return test_ok;
}

void UartDriver::shutdown() {
  // Optional: lower DTR/RTS and disable interrupts if desired.
  // this->write(MODEM_CONTROL, 0);
  // this->write(INTERRUPT, 0);
}
}  // namespace drivers
