mov $0x59b997fa,%rdi
mov $0x59b997fa,%edi
mov %rsp,%rax
add $0x18,%rax
sub $0x18,%rax
lea 0x8(%rsp),%rax
lea 0x8(%rsp),%rdi
lea -0x8(%rsp),%rdi
mov $0x5561dcb0,%rdi # phase3.s
push $0x4018fa
ret
