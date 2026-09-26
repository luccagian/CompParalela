#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

double *A, *B, *C;
int N, num_threads;

typedef struct {
    int thread_id;
} ThreadData;

void *worker_matmul(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int linhas_por_thread = N / num_threads;
    int inicio = data->thread_id * linhas_por_thread;
    int fim = (data->thread_id == num_threads - 1) ? N : inicio + linhas_por_thread;

    // Particionamento por bloco de linhas
    for (int i = inicio; i < fim; i++) {
        for (int j = 0; j < N; j++) {
            double sum = 0.0;
            for (int k = 0; k < N; k++) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <N> <num_threads>\n", argv[0]);
        return 1;
    }
    N = atoi(argv[1]);
    num_threads = atoi(argv[2]);

    A = (double *)malloc(N * N * sizeof(double));
    B = (double *)malloc(N * N * sizeof(double));
    C = (double *)calloc(N * N, sizeof(double));

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = (double)(i + j);
            B[i * N + j] = (double)(i * j);
        }
    }

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        pthread_create(&threads[i], NULL, worker_matmul, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
    double gflops = (2.0 * N * N * N) / (tempo * 1e9);

    printf("[Pthreads Matmul] N: %d | Threads: %d | Tempo: %.4f s | GFLOPS: %.2f | C[0]: %.1f\n", N, num_threads, tempo, gflops, C[0]);

    free(A); free(B); free(C);
    return 0;
}