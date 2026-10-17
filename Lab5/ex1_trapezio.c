/*
 * Gian Lucca Campanha Ribeiro
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Funcao matematica utilizada: f(x) = x^2
double f(double x) {
    return x * x;
}

// VERSAO 1 - SEQUENCIAL
double trapezio_serial(double a, double b, long long n) {

    double h = (b - a) / n;
    double resultado = (f(a) + f(b)) / 2.0;

    for (long long i = 1; i < n; i++) {
        double x = a + i * h;
        resultado += f(x);
    }

    return resultado * h;
}

// VERSAO 2 - PARALELA COM CRITICAL
double trapezio_critical(double a, double b,
                         long long n, int threads) {

    double h = (b - a) / n;
    double resultado = 0.0;

    #pragma omp parallel num_threads(threads)
    {
        int id = omp_get_thread_num();
        int total_threads = omp_get_num_threads();

        long long inicio = (n * id) / total_threads;
        long long fim = (n * (id + 1)) / total_threads;

        double soma_local = 0.0;

        for (long long i = inicio; i < fim; i++) {

            double x1 = a + i * h;
            double x2 = x1 + h;

            soma_local += (f(x1) + f(x2)) * h / 2.0;
        }

        #pragma omp critical
        {
            resultado += soma_local;
        }
    }

    return resultado;
}

// VERSAO 3 - PARALELA COM REDUCTION
double trapezio_reduction(double a, double b,
                          long long n, int threads) {

    double h = (b - a) / n;
    double resultado = 0.0;

    #pragma omp parallel num_threads(threads) reduction(+:resultado)
    {
        int id = omp_get_thread_num();
        int total_threads = omp_get_num_threads();

        long long inicio = (n * id) / total_threads;
        long long fim = (n * (id + 1)) / total_threads;

        for (long long i = inicio; i < fim; i++) {

            double x1 = a + i * h;

            resultado += (f(x1) + f(x1 + h)) * h / 2.0;
        }
    }

    return resultado;
}

// VERSAO 4 - PARALLEL FOR + REDUCTION
double trapezio_parallel_for(double a, double b,
                             long long n, int threads) {

    double h = (b - a) / n;
    double resultado = (f(a) + f(b)) / 2.0;

    #pragma omp parallel for num_threads(threads) \
    reduction(+:resultado) schedule(static)

    for (long long i = 1; i < n; i++) {

        double x = a + i * h;

        resultado += f(x);
    }

    return resultado * h;
}

// PROGRAMA PRINCIPAL
int main(int argc, char *argv[]) {

    if (argc != 5) {
        printf("Uso: %s <threads> <a> <b> <trapezios>\n",
               argv[0]);
        return 1;
    }

    int threads = atoi(argv[1]);
    double a = atof(argv[2]);
    double b = atof(argv[3]);
    long long n = atoll(argv[4]);

    if (threads < 1 || n < 1 || b <= a) {
        printf("Parametros invalidos.\n");
        return 1;
    }

    double inicio, tempo;

    printf("\n=== EXERCICIO 1 - REGRA DO TRAPEZIO ===\n");
    printf("Funcao: f(x) = x^2\n");
    printf("Intervalo: %.2f ate %.2f\n", a, b);
    printf("Trapezios: %lld\n", n);
    printf("Threads: %d\n\n", threads);

    inicio = omp_get_wtime();

    double serial = trapezio_serial(a, b, n);

    tempo = omp_get_wtime() - inicio;

    printf("Versao Sequencial\n");
    printf("Resultado: %.12f\n", serial);
    printf("Tempo: %.6f segundos\n\n", tempo);

    inicio = omp_get_wtime();

    double critical = trapezio_critical(a, b, n, threads);

    tempo = omp_get_wtime() - inicio;

    printf("Versao Critical\n");
    printf("Resultado: %.12f\n", critical);
    printf("Tempo: %.6f segundos\n\n", tempo);

    inicio = omp_get_wtime();

    double reduction = trapezio_reduction(a, b, n, threads);

    tempo = omp_get_wtime() - inicio;

    printf("Versao Reduction\n");
    printf("Resultado: %.12f\n", reduction);
    printf("Tempo: %.6f segundos\n\n", tempo);

    inicio = omp_get_wtime();

    double parallel_for = trapezio_parallel_for(a, b, n, threads);

    tempo = omp_get_wtime() - inicio;

    printf("Versao Parallel For + Reduction\n");
    printf("Resultado: %.12f\n", parallel_for);
    printf("Tempo: %.6f segundos\n", tempo);

    return 0;
}