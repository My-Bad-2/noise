#include "arch/x86_64/registers.h"

.section .text
.extern exception_handler
.extern nmi_handler
.type interrupt_common_stub, @function
interrupt_common_stub:
  cld

  pushq %rax
  pushq %rbx
  pushq %rcx
  pushq %rdx
  pushq %rsi
  pushq %rdi
  pushq %rbp
  pushq %r8
  pushq %r9
  pushq %r10
  pushq %r11
  pushq %r12
  pushq %r13
  pushq %r14
  pushq %r15

  // Zero general-purpose registers to constrain speculative execution
  xorl %eax, %eax
  xorl %ebx, %ebx
  xorl %ecx, %ecx
  xorl %edx, %edx
  xorl %esi, %esi
  xorl %edi, %edi
  xorq %r8, %r8
  xorq %r9, %r9
  xorq %r10, %r10
  xorq %r11, %r11
  xorq %r12, %r12
  xorq %r13, %r13
  xorq %r14, %r14
  xorq %r15, %r15

  movq %rsp, %rdi

  // Non-Maskable Interrupt
  cmpq $2, IFRAME_OFFSET_VECTOR(%rsp)
  je .nmi

  // User-space exception?
  testb $3, IFRAME_OFFSET_CS(%rsp)
  jz 1f

  // No stack frame for user fault
  xorl %ebp, %ebp
  swapgs
1:
  call exception_handler

  // User-space exception?
  testb $3, IFRAME_OFFSET_CS(%rsp)
  jz .common_return
  swapgs
.common_return:
  popq %r15
  popq %r14
  popq %r13
  popq %r12
  popq %r11
  popq %r10
  popq %r9
  popq %r8
  popq %rbp
  popq %rdi
  popq %rsi
  popq %rdx
  popq %rcx
  popq %rbx
  popq %rax

  // Drop vector number and error code
  add $16, %rsp
  iretq
.nmi:
  mov $MSR_GS_BASE, %ecx
  rdmsr
  shl $32, %rdx
  or %rdx, %rax
  mov %rax, %rbx

  // Compute the %rsp-relative offset of the stack base
  .stack_base_from_rsp = INTERRUPT_STACK_SIZE - IFRAME_SIZE
  .percpu_from_rsp = .stack_base_from_rsp + PERCPU_INTERRUPT_STACKS_NMI_OFFSET

  movq %rsp, %rax
  sub $.percpu_from_rsp, %rax

  mov $MSR_GS_BASE, %ecx
  movq %rax, %rdx
  shr $32, %rdx
  wrmsr

  call nmi_handler
  movq %rbx, %rax

  mov $MSR_GS_BASE, %ecx
  movq %rax, %rdx
  shr $32, %rdx
  wrmsr

  jmp .common_return

.section .text
.global load_idt
.type load_idt, @function
load_idt:
  lidtq (%rdi)
  ret

.macro isr number
  .type isr_\number, @function
  isr_\number:
.if !(\number == 8 || (\number >= 10 && \number <= 14) || \number == 17 || \number == 21 || \number == 29 || \number == 30)
    push $0
.endif
    push $\number
    jmp interrupt_common_stub
.endm

.altmacro
.macro isr_insert number
    .section .text
    isr \number

    .section .data
    .quad isr_\number
.endm

.section .data
isr_table:
.set i, 0
.rept 256
    isr_insert %i
    .set i, i + 1
.endr
.global isr_table
