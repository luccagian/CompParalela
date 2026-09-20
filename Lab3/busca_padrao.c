#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

char *buffer;
long total_bytes;
int num_threads;
char padrao[] = "AB";
int len_padrao;

typedef struct {
    int thread_id;
    long contagem_local;
} ThreadData;

void* worker_busca(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    long bloco = total_bytes / num_threads;
    long inicio = data->thread_id * bloco;
    
    // Inclui a extensão de fronteira para ler o padrão se for cortado na divisão do bloco
    long fim = (data->thread_id == num_threads - 1) ? total_bytes : (inicio + bloco + len_padrao - 1);
    if (fim > total_bytes) fim = total_bytes;

    data->contagem_local = 0;

    for (long i = inicio; i <= fim - len_padrao; i++) {
        if (strncmp(&buffer[i], padrao, len_padrao) == 0) {
            data->contagem_local++;
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Uso: %s <tamanho_MB> <num_threads>\n", argv[0]);
        return 1;
    }

    long tamanho_mb = atol(argv[1]);
    num_threads = atoi(argv[2]);
    total_bytes = tamanho_mb * 1024 * 1024;
    len_padrao = strlen(padrao);

    buffer = (char*)malloc(total_bytes);
    
    // Preenche com caracteres aleatórios simples ('A'..'Z')
    for (long i = 0; i < total_bytes; i++) {
        buffer[i] = 'A' + (rand() % 26);
    }

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        pthread_create(&threads[i], NULL, worker_busca, &thread_data[i]);
    }

    long total_ocorrencias = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total_ocorrencias += thread_data[i].contagem_local;
    }

    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("[Busca Padrao] MB: %ld | Threads: %d | Ocorrencias ('AB'): %ld | Tempo: %.4f s\n",
           tamanho_mb, num_threads, total_ocorrencias, tempo);

    free(buffer);
    return 0;
}