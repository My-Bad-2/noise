.section .text

.extern _init
.extern kmain
.extern _fini

.global _start
.type _start, @function
_start:
  cli

  // Call Global constructors
  call _init

  call kmain

  // Ideally the kernel should never execute this function
  call _fini

  // Halt the system.
  // If kmain (or _fini) returns, something is wrong, or the kernel's job is done.
  // Disable interrupts again (belt-and-suspenders) and halt the processor.
.Lhalt_loop:
  cli
  hlt
  jmp .Lhalt_loop

// Set size for the _start symbol, useful for debugging.
.size _start, . - _start