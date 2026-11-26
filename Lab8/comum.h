#ifndef COMUM_H
#define COMUM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include <openacc.h>

int compare_int(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;

    return (x > y) - (x < y);
}

int verificar_ordenacao(int *data, int n) {
    for (int i = 0; i < n - 1; i++) {
        if (data[i] > data[i + 1])
            return 0;
    }

    return 1;
}

void gerar_dados(int *data, int n) {
    for (int i = 0; i < n; i++) {
        data[i] = rand() % n;
    }
}

void apresentar_resultado(const char *versao,
                           double tempo_serial,
                           double tempo_paralelo,
                           int p,
                           int correto) {
    double speedup = tempo_serial / tempo_paralelo;
    double eficiencia = speedup / p;

    printf("\n--- %s ---\n", versao);
    printf("Verificacao: %s\n", correto ? "Passou" : "Falhou");
    printf("Tempo: %.6f segundos\n", tempo_paralelo);
    printf("Speedup: %.4f\n", speedup);
    printf("Eficiencia: %.2f%%\n", eficiencia * 100);
}

#pragma acc routine seq
void ordenar_bloco_acc(int *data, int n) {
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            int temp = data[i];
            int j = i;

            while (j >= gap && data[j - gap] > temp) {
                data[j] = data[j - gap];
                j -= gap;
            }

            data[j] = temp;
        }
    }
}

#endif