#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define Msize 61
#define Nsize 67


/*void transpose_lazy_32_32(int M, int N, int A[Nsize][Msize], int B[Msize][Nsize]);*/

void transpose_4_4_non_even(int M, int N, int A[Msize][Nsize], int B[Nsize][Msize]);
// Assume this is implemented elsewhere
void transpose(int M, int N, int A[Msize][Nsize], int B[Nsize][Msize]) {
    transpose_4_4_non_even(M, N, A, B);
}

// Initialize matrix A with unique values
void initialize_matrix(int A[Msize][Nsize]) {
    for (int i = 0; i < Msize; i++) {
        for (int j = 0; j < Nsize; j++) {
            A[i][j] = (i * Msize + j) % 10;
        }
    }
}

// Initialize matrix A with unique values
void initialize_matrix_zero(int A[Nsize][Msize]) {
    for (int i = 0; i < Nsize; i++) {
        for (int j = 0; j < Msize; j++) {
            A[i][j] = 0;
        }
    }
}

// Print a matrix of given dimensions
void print_matrix_A(const char* name, int rows, int cols, int matrix[Msize][Nsize]) {
    printf("%s:\n", name);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
void print_matrix_B(const char* name, int rows, int cols, int matrix[Nsize][Msize]) {
    printf("%s:\n", name);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Check if B is the transpose of A
int is_transpose(int M, int N, int A[Msize][Nsize], int B[Nsize][Msize]);

// Test function with printing
int main() {
    int A[Msize][Nsize];
    int B[Nsize][Msize];

    initialize_matrix(A);
    initialize_matrix_zero(B);

    // Print matrix before transpose
    print_matrix_A("Original Matrix A", Msize, Nsize, A);

    transpose(Msize, Nsize, A, B);

    // Print matrix after transpose
    print_matrix_B("Transposed Matrix B", Nsize, Msize, B);

    // Verify correctness
    if (is_transpose(Msize, Nsize, A, B)) {
        printf("Transpose test passed.\n");
    } else {
        printf("Transpose test failed.\n");
    }
    return 0;
}

