#ifndef ARCH_DRIVERS_UART_INTERNAL_HPP
#define ARCH_DRIVERS_UART_INTERNAL_HPP 1

#include <stdint.h>

namespace drivers::uart {

// 16550-compatible UART register offsets (from base I/O port)
constexpr uint8_t DATA = 0;                      // RBR (read) / THR (write)
constexpr uint8_t INTERRUPT = 1;                 // IER
constexpr uint8_t INTERRUPT_IDENTIFICATION = 2;  // IIR (read) / FCR (write)
constexpr uint8_t LINE_CONTROL = 3;              // LCR
constexpr uint8_t MODEM_CONTROL = 4;             // MCR
constexpr uint8_t LINE_STATUS = 5;               // LSR
constexpr uint8_t MODEM_STATUS = 6;              // MSR
constexpr uint8_t SCRATCH = 7;                   // SCR

// Backwards-compatibility alias (typo retained for transitional code paths)
constexpr uint8_t INTERRUPT_IDENTIFACTOR = INTERRUPT_IDENTIFICATION;

constexpr uint8_t BAUD_RATE_LOW = DATA;        // DLL when DLAB=1
constexpr uint8_t BAUD_RATE_HIGH = INTERRUPT;  // DLH when DLAB=1
constexpr uint8_t FIFO_CONTROLLER =
    INTERRUPT_IDENTIFICATION;  // FCR when written

// LCR bits
constexpr uint8_t LINE_DS_5 = 0;
constexpr uint8_t LINE_DS_6 = 1;
constexpr uint8_t LINE_DS_7 = 2;
constexpr uint8_t LINE_DS_8 = 3;
constexpr uint8_t LINE_DLAB_STATUS = (1u << 7);

// MCR bits
constexpr uint8_t MODEM_DTR = (1u << 0);
constexpr uint8_t MODEM_RTS = (1u << 1);
constexpr uint8_t MODEM_OUT1 = (1u << 2);
constexpr uint8_t MODEM_OUT2 = (1u << 3);
constexpr uint8_t MODEM_LOOPBACK = (1u << 4);

// IER bits
constexpr uint8_t INTERRUPT_WHEN_DATA_AVAILABLE = (1u << 0);
constexpr uint8_t INTERRUPT_WHEN_TRANSMITTER_EMPTY = (1u << 1);
constexpr uint8_t INTERRUPT_WHEN_BREAK_EMPTY = (1u << 2);
constexpr uint8_t INTERRUPT_WHEN_STATUS_UPDATE = (1u << 3);

// LSR bits
constexpr uint8_t LINE_DATA_READY = (1u << 0);
constexpr uint8_t LINE_OVERRUN_ERROR = (1u << 1);
constexpr uint8_t LINE_PARITY_ERROR = (1u << 2);
constexpr uint8_t LINE_FRAMING_ERROR = (1u << 3);
constexpr uint8_t LINE_BREAK_INDICATOR = (1u << 4);
constexpr uint8_t LINE_TRANSMITTER_BUF_EMPTY = (1u << 5);
constexpr uint8_t LINE_TRANSMITTER_EMPTY = (1u << 6);
constexpr uint8_t LINE_IMPENDING_ERROR = (1u << 7);

// FCR bits and trigger levels
constexpr uint8_t ENABLE_FIFO = (1u << 0);
constexpr uint8_t FIFO_CLEAR_RECEIVE = (1u << 1);
constexpr uint8_t FIFO_CLEAR_TRANSMIT = (1u << 2);
constexpr uint8_t FIFO_ENABLE_64_BYTE = (1u << 5);
constexpr uint8_t FIFO_TRIGGER_LEVEL1 = (0u << 6);
constexpr uint8_t FIFO_TRIGGER_LEVEL2 = (1u << 6);
constexpr uint8_t FIFO_TRIGGER_LEVEL3 = (2u << 6);
constexpr uint8_t FIFO_TRIGGER_LEVEL4 = (3u << 6);

}  // namespace drivers::uart

#endif  // ARCH_DRIVERS_UART_INTERNAL_HPP
