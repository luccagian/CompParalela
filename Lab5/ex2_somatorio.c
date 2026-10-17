/*
 * Gian Lucca Campanha Ribeiro
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <omp.h>

// SOMATORIO UTILIZANDO CRITICAL
uint64_t somatorio_critical(uint32_t *vetor,
                            size_t n, int threads) {

    uint64_t soma_global = 0;

    #pragma omp parallel num_threads(threads)
    {
        uint64_t soma_local = 0;

        #pragma omp for schedule(static)
        for (size_t i = 0; i < n; i++) {
            soma_local += vetor[i];
        }

        #pragma omp critical
        {
            soma_global += soma_local;
        }
    }

    return soma_global;
}

// SOMATORIO UTILIZANDO REDUCTION
uint64_t somatorio_reduction(uint32_t *vetor,
                             size_t n, int threads) {

    uint64_t soma = 0;

    #pragma omp parallel for num_threads(threads) \
    reduction(+:soma) schedule(static)

    for (size_t i = 0; i < n; i++) {
        soma += vetor[i];
    }

    return soma;
}

// PROGRAMA PRINCIPAL
int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Uso: %s <critical|reduction> <threads> <tamanho>\n",
               argv[0]);
        return 1;
    }

    int threads = atoi(argv[2]);
    size_t n = strtoull(argv[3], NULL, 10);

    if (threads < 1 || n == 0 ||
        n > SIZE_MAX / sizeof(uint32_t)) {
        printf("Parametros invalidos.\n");
        return 1;
    }

    uint32_t *vetor = malloc(n * sizeof(uint32_t));

    if (vetor == NULL) {
        printf("Erro ao alocar memoria.\n");
        return 1;
    }

    // Inicializacao deterministica
    uint64_t soma_esperada = 0;

    for (size_t i = 0; i < n; i++) {
        vetor[i] = (uint32_t)((i * 17ULL + 23ULL) % 1009ULL);
        soma_esperada += vetor[i];
    }

    double tempo_total = 0;
    uint64_t resultado = 0;

    printf("\n=== EXERCICIO 2 - SOMATORIO ===\n");
    printf("Versao: %s\n", argv[1]);
    printf("Threads: %d\n", threads);
    printf("Elementos: %zu\n\n", n);

    // Dez execucoes
    for (int execucao = 1; execucao <= 10; execucao++) {

        double inicio = omp_get_wtime();

        if (strcmp(argv[1], "critical") == 0) {

            resultado = somatorio_critical(
                vetor, n, threads
            );

        } else if (strcmp(argv[1], "reduction") == 0) {

            resultado = somatorio_reduction(
                vetor, n, threads
            );

        } else {
            printf("Versao invalida.\n");
            free(vetor);
            return 1;
        }

        double tempo = omp_get_wtime() - inicio;

        tempo_total += tempo;

        printf("Execucao %2d: %.9f segundos\n",
               execucao, tempo);

        if (resultado != soma_esperada) {
            printf("ERRO: Resultado incorreto!\n");
            free(vetor);
            return 1;
        }
    }

    printf("\nResultado da soma: %" PRIu64 "\n", resultado);

    printf("Tempo medio: %.9f segundos\n",
           tempo_total / 10.0);

    printf("Verificacao: CORRETO\n");

    free(vetor);

    return 0;
}