
tmp.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 fa 97 b9 59 	mov    $0x59b997fa,%rdi
   7:	bf fa 97 b9 59       	mov    $0x59b997fa,%edi
   c:	48 89 e0             	mov    %rsp,%rax
   f:	48 83 c0 18          	add    $0x18,%rax
  13:	48 83 e8 18          	sub    $0x18,%rax
  17:	48 8d 44 24 08       	lea    0x8(%rsp),%rax
  1c:	48 8d 7c 24 08       	lea    0x8(%rsp),%rdi
  21:	48 8d 7c 24 f8       	lea    -0x8(%rsp),%rdi
  26:	48 c7 c7 b0 dc 61 55 	mov    $0x5561dcb0,%rdi
  2d:	68 fa 18 40 00       	push   $0x4018fa
  32:	c3                   	ret
