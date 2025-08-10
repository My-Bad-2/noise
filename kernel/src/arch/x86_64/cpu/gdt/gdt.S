#define KERNEL_CODE 0x8
#define KERNEL_DATA 0x10

#define TSS_OFFSET 0x28

.section .text
.global load_gdt
.type load_gdt, @function
load_gdt:
  lgdtq (%rdi)

  movw $KERNEL_DATA, %ax
  movl %eax, %ds
  movl %eax, %es
  movl %eax, %fs
  movl %eax, %gs
  movl %eax, %ss

  lea 1f(%rip), %rax

  pushq $KERNEL_CODE
  pushq %rax

  lretq
1:
  retq

.global load_tss
.type load_tss, @function
load_tss:
  movw $TSS_OFFSET, %ax
  ltr %ax
  retq
