#define KERNEL_CODE 0x8          // Selector for kernel code segment (index 1)
#define KERNEL_DATA 0x10         // Selector for kernel data segment (index 2)

#define TSS_OFFSET 0x28          // Offset to TSS descriptor within the GDT

.section .text
.global load_gdt
.type load_gdt, @function
load_gdt:
  // Load GDTR with pointer (limit, base) to our GDT (argument in %rdi)
  lgdtq (%rdi)

  // Reload data segment registers with kernel data selector.
  // Note: in long mode, DS/ES/SS are mostly ignored but must be valid.
  movw $KERNEL_DATA, %ax
  movl %eax, %ds
  movl %eax, %es
  movl %eax, %fs
  movl %eax, %gs
  movl %eax, %ss

  // Perform a far return to reload CS with kernel code selector:
  // push target CS, push target RIP, then lretq
  lea 1f(%rip), %rax
  pushq $KERNEL_CODE
  pushq %rax
  lretq
1:
  retq

.global load_tss
.type load_tss, @function
load_tss:
  // Load the Task Register with the TSS selector.
  // TSS selector points to the 64-bit available TSS descriptor (type 0x9).
  movw $TSS_OFFSET, %ax
  ltr %ax
  retq
