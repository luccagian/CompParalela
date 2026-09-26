#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN(a,b) (((a)<(b))?(a):(b))

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <N> <BLOCK_SIZE>\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);
    int B = atoi(argv[2]);

    double *A = (double *)malloc(N * N * sizeof(double));
    double *B_mat = (double *)malloc(N * N * sizeof(double));
    double *C = (double *)calloc(N * N, sizeof(double));

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = (double)(i + j);
            B_mat[i * N + j] = (double)(i * j);
        }
    }

    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // Multiplicação Blocada (6 loops com reordenação interna i-k-j)
    for (int ii = 0; ii < N; ii += B) {
        for (int jj = 0; jj < N; jj += B) {
            for (int kk = 0; kk < N; kk += B) {
                for (int i = ii; i < MIN(ii + B, N); i++) {
                    for (int k = kk; k < MIN(kk + B, N); k++) {
                        double rA = A[i * N + k];
                        for (int j = jj; j < MIN(jj + B, N); j++) {
                            C[i * N + j] += rA * B_mat[k * N + j];
                        }
                    }
                }
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
    double gflops = (2.0 * N * N * N) / (tempo * 1e9);

    printf("[Matmul Blocado] N: %d | Bloco B: %d | Tempo: %.4f s | GFLOPS: %.2f | C[0]: %.1f\n", N, B, tempo, gflops, C[0]);

    free(A); free(B_mat); free(C);
    return 0;
}