#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <omp.h>

#ifndef N
#define N (1024LL * 1024 * 100)
#endif

int main(void) {
    int *data = malloc((size_t)N * sizeof(int));

    if (data == NULL) {
        perror("Erro ao alocar memoria");
        return 1;
    }

    printf("Inicializando dados...\n");

    #pragma omp parallel for
    for (long long i = 0; i < N; i++) {
        data[i] = (i % 1000) - 500;
    }

    int min_val = INT_MAX;
    int max_val = INT_MIN;
    long long sum = 0;
    long long sum_sq = 0;

    double inicio = omp_get_wtime();

    #pragma omp parallel sections reduction(min:min_val) \
                             reduction(max:max_val) \
                             reduction(+:sum,sum_sq)
    {
        #pragma omp section
        {
            for (long long i = 0; i < N; i++) {
                if (data[i] < min_val)
                    min_val = data[i];
            }
        }

        #pragma omp section
        {
            for (long long i = 0; i < N; i++) {
                if (data[i] > max_val)
                    max_val = data[i];
            }
        }

        #pragma omp section
        {
            for (long long i = 0; i < N; i++) {
                sum += data[i];
                sum_sq += (long long)data[i] * data[i];
            }
        }
    }

    double tempo = omp_get_wtime() - inicio;

    double media = (double)sum / N;
    double variancia = (double)sum_sq / N - media * media;
    double desvio = sqrt(fmax(0.0, variancia));

    printf("\n--- Versao Sections ---\n");
    printf("Minimo: %d\n", min_val);
    printf("Maximo: %d\n", max_val);
    printf("Soma: %lld\n", sum);
    printf("Media: %.2f\n", media);
    printf("Desvio padrao: %.2f\n", desvio);
    printf("Tempo: %.6f segundos\n", tempo);

    free(data);

    return 0;
}