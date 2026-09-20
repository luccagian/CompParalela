#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    long thread_id;
    long num_threads;
    long K;
    long total_local;
} ThreadData;

bool eh_primo(long n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (long i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

void* worker_primos(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    data->total_local = 0;

    // Distribuição cíclica para balanceamento de carga
    for (long i = 2 + data->thread_id; i <= data->K; i += data->num_threads) {
        if (eh_primo(i)) {
            data->total_local++;
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Uso: %s <K> <num_threads>\n", argv[0]);
        return 1;
    }

    long K = atol(argv[1]);
    int num_threads = atoi(argv[2]);

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].num_threads = num_threads;
        thread_data[i].K = K;
        pthread_create(&threads[i], NULL, worker_primos, &thread_data[i]);
    }

    long total_primos = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total_primos += thread_data[i].total_local;
    }

    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("[Primos] K: %ld | Threads: %d | Total Primos: %ld | Tempo: %.4f s\n",
           K, num_threads, total_primos, tempo);

    return 0;
}