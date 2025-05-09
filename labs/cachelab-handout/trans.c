/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include "cachelab.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

void print_matrix(const char *name, int rows, int cols, int matrix[][32]);
int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose_lazy_32_32(int M, int N, int A[N][M], int B[M][N]);
void transpose_lazy_64(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    // 4 by 4 blocking and change access pattern
    if (M == 32 && N == 32) {
        transpose_lazy_32_32(M, N, A, B);
    }

    if (M == 64 && N == 64) {
        transpose_lazy_64(M, N, A, B);
    }
}

char transpose_4_4_local_desc[] =
    "use 4 by 4 and local variable - read 4 elem in a line at a time";
void transpose_4_4_local(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, ii, jj, a, b, c, d;

    for (i = 0; i < N; i += 4) {
        for (j = 0; j < M; j += 4) {
            for (ii = i; ii < i + 4; ii++) {
                for (jj = j; jj < j + 4; jj++) {
                    switch (jj - j) {
                    case 0:
                        a = A[ii][jj];
                        break;
                    case 1:
                        b = A[ii][jj];
                        break;
                    case 2:
                        c = A[ii][jj];
                        break;
                    case 3:
                        d = A[ii][jj];
                        break;
                    }
                }
                for (jj = j; jj < j + 4; jj++) {
                    switch (jj - j) {
                    case 0:
                        B[jj][ii] = a;
                        break;
                    case 1:
                        B[jj][ii] = b;
                        break;
                    case 2:
                        B[jj][ii] = c;
                        break;
                    case 3:
                        B[jj][ii] = d;
                        break;
                    }
                }
            }
        }
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */
char trans_local_desc[] =
    "using local variable to read elems in a 2 by 2 block";
void trans_local(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, ii, jj, cnt, a, b, c, d, stride = 2;

    for (i = 0; i < N; i += stride) {
        for (j = 0; j < M; j += stride) {
            cnt = 0;
            for (ii = i; ii < i + stride; ii++) {
                for (jj = j; jj < j + stride; jj++) {
                    switch (cnt) {
                    case 0:
                        a = A[ii][jj];
                        break;
                    case 1:
                        b = A[ii][jj];
                        break;
                    case 2:
                        c = A[ii][jj];
                        break;
                    case 3:
                        d = A[ii][jj];
                        break;
                    }
                    cnt++;
                }
            }
            cnt = 0;
            for (ii = j; ii < j + stride; ii++) {
                for (jj = i; jj < i + stride; jj++) {
                    switch (cnt) {
                    case 0:
                        B[ii][jj] = a;
                        break;
                    case 1:
                        B[ii][jj] = c;
                        break;
                    case 2:
                        B[ii][jj] = b;
                        break;
                    case 3:
                        B[ii][jj] = d;
                        break;
                    }
                    cnt++;
                }
            }
        }
    }
}

char trans_ap_desc[] = "different access pattern using triangle";
void trans_ap_tri(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;

    assert(M == N);
    for (i = N - 1; i >= 0; i--) {
        for (j = 0; j <= i; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    for (i = N - 1; i >= 0; i--) {
        for (j = M - 1; j > i; j--) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

char transpose_8_8_desc[] = "using 8 by 8 blocking";
void transpose_8_8(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, ii, jj;

    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            for (ii = i; ii < i + 8; ii++) {
                for (jj = j; jj < j + 8; jj++) {
                    B[jj][ii] = A[ii][jj];
                }
            }
        }
    }
}
char transpose_4_4_desc[] = "using 4 by 4 blocking";
void transpose_4_4(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, ii, jj;

    for (i = 0; i < N; i += 4) {
        for (j = 0; j < M; j += 4) {
            for (ii = i; ii < i + 4; ii++) {
                for (jj = j; jj < j + 4; jj++) {
                    B[jj][ii] = A[ii][jj];
                }
            }
        }
    }
}

char transpose_lazy_64_desc[] = "using lazy technique to get 1300 misses";
void transpose_lazy_64(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, ii, jj, a, b, c, d, counter;

    // inside 8 by 8 block
    for (i = 0; i < N - 7; i += 8) {
        for (j = 0; j < M - 7; j += 8) {
            // inside 4 by 4 block
            for (ii = 0; ii < 4; ii++) {
                for (jj = 0; jj < 4; jj++) {
                    B[j + 0 + jj][i + 0 + ii] = A[i + 0 + ii][j + 0 + jj];
                    B[j + 0 + jj][i + 4 + ii] = A[i + 0 + ii][j + 4 + jj];
                }
            }

            for (counter = 0; counter < 4; counter++) {
                // 1. store in local vars
                a = B[j + 0 + counter][i + 4 + 0];
                b = B[j + 0 + counter][i + 4 + 1];
                c = B[j + 0 + counter][i + 4 + 2];
                d = B[j + 0 + counter][i + 4 + 3];
                // 2. move A to B
                B[j + 0 + counter][i + 4 + 0] = A[i + 4 + 0][j + 0 + counter];
                B[j + 0 + counter][i + 4 + 1] = A[i + 4 + 1][j + 0 + counter];
                B[j + 0 + counter][i + 4 + 2] = A[i + 4 + 2][j + 0 + counter];
                B[j + 0 + counter][i + 4 + 3] = A[i + 4 + 3][j + 0 + counter];
                // 3. move local vars to new B
                B[j + 4 + counter][i + 0 + 0] = a;
                B[j + 4 + counter][i + 0 + 1] = b;
                B[j + 4 + counter][i + 0 + 2] = c;
                B[j + 4 + counter][i + 0 + 3] = d;
            }
            for (ii = 0; ii < 4; ii++) {
                for (jj = 0; jj < 4; jj++) {
                    B[j + 4 + jj][i + 4 + ii] = A[i + 4 + ii][j + 4 + jj];
                }
            }
        }
    }
}

char transpose_lazy_32_32_desc[] = "using lazy technique to get 256 misses";
void transpose_lazy_32_32(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, ii, jj, stride;
    stride = 8; // 8 by 8 blocking
    for (i = 8; i < N; i += stride) {
        // process main diagonal first, without (0, 0) blocks
        j = i;
        // use (1, 0) block as temp-block
        for (ii = 0; ii < stride; ii++) { // use local indexing
            for (jj = 0; jj < stride; jj++) {
                B[stride + ii][jj] = A[i + ii][j + jj];
            }
        }
        for (ii = 0; ii < stride; ii++) {
            for (jj = 0; jj < stride; jj++) {
                B[i + jj][j + ii] = B[stride + ii][jj];
            }
        }
    }
    // move (0,1) block to (1, 0) block
    for (ii = 0; ii < stride; ii++) {
        for (jj = 0; jj < stride; jj++) {
            B[stride + jj][ii] = A[ii][stride + jj];
        }
    }
    // move (0, 0) to (0, 1)
    for (ii = 0; ii < stride; ii++) {
        for (jj = 0; jj < stride; jj++) {
            B[ii][stride + jj] = A[ii][jj];
        }
    }
    // move (0, 1) to (0, 0)
    for (ii = 0; ii < stride; ii++) {
        for (jj = 0; jj < stride; jj++) {
            B[ii][jj] = B[jj][stride + ii];
        }
    }
    for (j = 0; j < M; j += stride) {
        for (i = 0; i < N; i += stride) { // this is global indexing
            if (i == j || (i == 1 && j == 0))
                continue;
            for (ii = i; ii < i + stride; ii++) {
                for (jj = j; jj < j + stride; jj++) {
                    B[jj][ii] = A[ii][jj];
                }
            }
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    registerTransFunction(transpose_lazy_32_32, transpose_lazy_32_32_desc);

    /* Register any additional transpose functions */
    // registerTransFunction(trans, trans_desc);

    // registerTransFunction(trans_local, trans_local_desc);

    // registerTransFunction(trans_ap_tri, trans_ap_desc);

    // registerTransFunction(transpose_4_4, transpose_4_4_desc);

    // registerTransFunction(transpose_8_8, transpose_8_8_desc);

    // registerTransFunction(transpose_4_4_local, transpose_4_4_local_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
