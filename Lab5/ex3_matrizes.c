/*
 * Gian Lucca Campanha Ribeiro
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

// MULTIPLICACAO SEQUENCIAL
void multiplicacao_serial(double *A, double *BT,
                          double *C, int n) {

    for (int i = 0; i < n; i++) {

        for (int j = 0; j < n; j++) {

            double soma = 0.0;

            for (int k = 0; k < n; k++) {

                soma += A[(size_t)i * n + k] *
                        BT[(size_t)j * n + k];
            }

            C[(size_t)i * n + j] = soma;
        }
    }
}

// MULTIPLICACAO PARALELA
void multiplicacao_paralela(double *A, double *BT,
                            double *C, int n,
                            int threads) {

    #pragma omp parallel for num_threads(threads) schedule(static)

    for (int i = 0; i < n; i++) {

        for (int j = 0; j < n; j++) {

            double soma = 0.0;

            for (int k = 0; k < n; k++) {

                soma += A[(size_t)i * n + k] *
                        BT[(size_t)j * n + k];
            }

            C[(size_t)i * n + j] = soma;
        }
    }
}

// PROGRAMA PRINCIPAL
int main(int argc, char *argv[]) {

    if (argc != 4) {

        printf("Uso: %s <serial|parallel> <threads> <ordem>\n",
               argv[0]);

        return 1;
    }

    int threads = atoi(argv[2]);
    int n = atoi(argv[3]);

    if (threads < 1 || n < 1 || n > 8192) {
        printf("Parametros invalidos.\n");
        return 1;
    }

    size_t elementos = (size_t)n * n;

    double *A = malloc(elementos * sizeof(double));
    double *BT = malloc(elementos * sizeof(double));
    double *C = malloc(elementos * sizeof(double));

    if (A == NULL || BT == NULL || C == NULL) {

        printf("Erro ao alocar memoria.\n");

        free(A);
        free(BT);
        free(C);

        return 1;
    }

    // Inicializacao das matrizes
    for (int i = 0; i < n; i++) {

        for (int j = 0; j < n; j++) {

            A[(size_t)i * n + j] =
                (double)((i + j) % 13) / 13.0;

            // Armazena a matriz B transposta
            BT[(size_t)j * n + i] =
                (double)((i * 3 + j) % 17) / 17.0;
        }
    }

    double tempo_total = 0.0;
    double checksum = 0.0;

    printf("\n=== EXERCICIO 3 - MULTIPLICACAO DE MATRIZES ===\n");
    printf("Versao: %s\n", argv[1]);
    printf("Threads: %d\n", threads);
    printf("Ordem: %d x %d\n\n", n, n);

    // Tres execucoes
    for (int execucao = 1; execucao <= 3; execucao++) {

        double inicio = omp_get_wtime();

        if (strcmp(argv[1], "serial") == 0) {

            multiplicacao_serial(A, BT, C, n);

        } else if (strcmp(argv[1], "parallel") == 0) {

            multiplicacao_paralela(A, BT, C, n, threads);

        } else {

            printf("Versao invalida.\n");

            free(A);
            free(BT);
            free(C);

            return 1;
        }

        double tempo = omp_get_wtime() - inicio;

        tempo_total += tempo;

        printf("Execucao %d: %.9f segundos\n",
               execucao, tempo);
    }

    // Verificacao dos resultados
    for (size_t i = 0; i < elementos; i++) {
        checksum += C[i];
    }

    printf("\nChecksum: %.10f\n", checksum);

    printf("Tempo medio: %.9f segundos\n",
           tempo_total / 3.0);

    free(A);
    free(BT);
    free(C);

    return 0;
}