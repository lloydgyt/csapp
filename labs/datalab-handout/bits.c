#include <assert.h>
/*
 * CS:APP Data Lab
 *
 * <Please put your name and userid here>
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */

#endif
// 1
/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) { return ~((~x) & (~y)) & (~(x & y)); }
/*
 * tmin - return minimum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) { return 1 << 31; }
// 2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
    // get tmax and then xor?
    // int tmax = ;
    // return !(x ^ tmax);
    int first_part = !!(x + 1);
    int second_part = !((x + 1) ^ (~x));
    return first_part & second_part;
}
/*
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
    int low_mask = 0x55;
    int mid_mask = (low_mask << 8) | low_mask;
    int mask = (mid_mask << 16) | mid_mask;
    return !(~(x | mask));
}
/*
 * negate - return -x
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) { return ~(x) + 1; }
// 3
/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0'
 * to '9') Example: isAsciiDigit(0x35) = 1. isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
    int first_part = (x & (~0xf)) ^ 0x30;         // should be 0
    int second_part = ((x & (0xf)) + 6) & (~0xf); // should also be 0
    return !(first_part | second_part);
}
/*
 * conditional - same as x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
    // convert x to 0xffffffff or 0
    int mask = ~((~(!x)) + 1);
    return (y & mask) | (z & (~mask));
}
/*
 * isLessOrEqual - if x <= y  then return 1, else return 0
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
    int nx = ~(x) + 1;   // TODO caution TMin
    int result = y + nx; // result = y - x
    return !(result & (1 << 31));
}
// 4
/*
 * logicalNeg - implement the ! operator, using all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int logicalNeg(int x) {
    int lx;
    int lx1;
    int lx2;
    int lx3;
    int lx4;
    lx = x >> 16;
    lx = lx | x; // should aggregate to higher half

    lx1 = lx >> 8;
    lx1 = lx1 | lx;

    lx2 = lx1 >> 4;
    lx2 = lx2 | lx1;

    lx3 = lx2 >> 2;
    lx3 = lx3 | lx2;

    lx4 = lx3 >> 1;
    lx4 = lx4 | lx3;
    return (lx4 & 0x1) ^ 0x1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
    int sign_bit = x & (1 << 31);
    int zero_bits = 0;
    x = x ^ (sign_bit >> 31);
    int flag_16, flag_8, flag_4, flag_2, flag_1, flag_0;
    // now x start with a bunch of 0s

    int mask_16 = (1 << 31) >> 15;
    // TODO delete later
    int flag_16 = !!!(x & mask_16);
    x = x << (flag_16 << 4);
    zero_bits += (flag_16 << 4);

    int mask_8 = (1 << 31) >> 7;
    // TODO delete later
    int flag_8 = !!!(x & mask_8);
    x = x << (flag_8 << 3);
    zero_bits += (flag_8 << 3);

    int mask_4 = (1 << 31) >> 3;
    // TODO delete later
    int flag_4 = !!!(x & mask_4);
    x = x << (flag_4 << 2);
    zero_bits += (flag_4 << 2);

    int mask_2 = (1 << 31) >> 1;
    // TODO delete later
    int flag_2 = !!!(x & mask_2);
    x = x << (flag_2 << 1);
    zero_bits += (flag_2 << 1);

    int mask_1 = (1 << 31) >> 0;
    // TODO delete later
    int flag_1 = !!!(x & mask_1);
    x = x << (flag_1 << 0);
    zero_bits += (flag_1 << 0);

    int flag_0 = !!!(x & mask_1);
    zero_bits += flag_0;
    return (32 - zero_bits + 1);
}
// float
/*
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
    // for denorm
    int exp_field = uf & (0xff << 23);
    int frac_field = uf & (~(0xff << 23)); // TODO also include sign-bit
    int head_bit_frac = frac_field & (0x1 << 22);
    int increment_num = 1 << 23;
    int sign_bit = uf & (0x1 << 31);
    if (!(exp_field ^ (0xff << 23))) { // denorm big
        return uf;
    } else if (!exp_field) { // denorm small
        if (head_bit_frac) {
            // close to norm
            uf = uf << 1; // TODO interesting!
            return sign_bit | uf;
        } else {
            // really small
            uf = uf & ((0x1 << 31) >> 8); // set frac to zero
            return uf | (frac_field << 1);
        }
    } else {
        // for normalized, simply increment EXP field
        // TODO if denorm big, remember to set frac to 0
        return uf + increment_num;
    }
}
/*
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
    int sign_bit = uf & (0x1 << 31);
    int exp_field = uf & (0xff << 23);
    int frac_field = (uf & (~(0xff << 23))) & (~(0x1 << 31));
    int Exp;
    int result;
    if (!(exp_field ^ (0xff << 23))) { // denorm big
        return 0x80000000u;
    } else if (!exp_field) { // denorm small
        return 0;
    } else {
        Exp = (exp_field >> 23) - 127;
        if (Exp > 31) { // too big
            return 0x80000000u;
        } else if (Exp < 0) { // too small
            return 0;
        } else {
            frac_field = frac_field >>
                         (23 - Exp); // TODO should I rely on shift wrap around?
            // reset upper bits of frac
            result = (0x1 << Exp) | frac_field; // 0x1 is the implict "1"
            // handle sign-bit
            sign_bit = sign_bit >> (32 - (Exp + 1) - 1);
            return result | sign_bit;
        }
    }
}
/*
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 *
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatPower2(int x) {
    int result;
    if (x > 127) { // too big
        result = 0xff << 23;
    } else if (x < -126) { // denorm
        if (x < -149) {    // too small
            result = 0;
        } else {
            result = 0x1 << (x + 149);
        }
    } else { // norm
        result = (x + 127) << 23;
    }
    return result;
}
