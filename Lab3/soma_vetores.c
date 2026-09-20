#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

double *A, *B, *C;
long N;
int num_threads;

typedef struct {
    int thread_id;
} ThreadData;

void* worker_soma(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    long bloco = N / num_threads;
    long inicio = data->thread_id * bloco;
    long fim = (data->thread_id == num_threads - 1) ? N : inicio + bloco;

    for (long i = inicio; i < fim; i++) {
        C[i] = A[i] + B[i];
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Uso: %s <N> <num_threads>\n", argv[0]);
        return 1;
    }

    N = atol(argv[1]);
    num_threads = atoi(argv[2]);

    A = (double*)malloc(N * sizeof(double));
    B = (double*)malloc(N * sizeof(double));
    C = (double*)malloc(N * sizeof(double));

    for (long i = 0; i < N; i++) {
        A[i] = 1.0;
        B[i] = 2.0;
    }

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        pthread_create(&threads[i], NULL, worker_soma, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("[Soma Vetores] N: %ld | Threads: %d | C[0]: %.1f | Tempo: %.4f s\n",
           N, num_threads, C[0], tempo);

    free(A); free(B); free(C);
    return 0;
}