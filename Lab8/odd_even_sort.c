#include "comum.h"

#pragma acc routine seq
void compare_split(int *left, int *right,
                   int *temp, int n_local) {
    int i = 0;
    int j = 0;
    int k = 0;

    while (i < n_local && j < n_local) {
        if (left[i] <= right[j]) {
            temp[k++] = left[i++];
        } else {
            temp[k++] = right[j++];
        }
    }

    while (i < n_local) {
        temp[k++] = left[i++];
    }

    while (j < n_local) {
        temp[k++] = right[j++];
    }

    for (i = 0; i < n_local; i++) {
        left[i] = temp[i];
        right[i] = temp[n_local + i];
    }
}

void odd_even_sort_omp(int *data, int n, int p) {
    int n_local = n / p;

    int *temp = malloc(n * sizeof(int));

    if (temp == NULL) {
        perror("Erro ao alocar memoria");
        exit(1);
    }

    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < p; i++) {
            qsort(data + i * n_local,
                  n_local,
                  sizeof(int),
                  compare_int);
        }

        for (int phase = 0; phase < p; phase++) {
            int inicio = phase % 2;

            #pragma omp for
            for (int i = inicio; i < p - 1; i += 2) {
                compare_split(
                    data + i * n_local,
                    data + (i + 1) * n_local,
                    temp + i * n_local,
                    n_local
                );
            }
        }
    }

    free(temp);
}

void odd_even_sort_acc(int *data, int n, int p) {
    int n_local = n / p;

    int *temp = malloc(n * sizeof(int));

    if (temp == NULL) {
        perror("Erro ao alocar memoria");
        exit(1);
    }

    #pragma acc data copy(data[0:n]) create(temp[0:n])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < p; i++) {
            ordenar_bloco_acc(
                data + i * n_local,
                n_local
            );
        }

        for (int phase = 0; phase < p; phase++) {
            int inicio = phase % 2;

            #pragma acc parallel loop gang
            for (int i = inicio; i < p - 1; i += 2) {
                compare_split(
                    data + i * n_local,
                    data + (i + 1) * n_local,
                    temp + i * n_local,
                    n_local
                );
            }
        }
    }

    free(temp);
}

int main(int argc, char *argv[]) {
    int n = 10000;
    int p = 4;

    if (argc > 1)
        n = atoi(argv[1]);

    if (argc > 2)
        p = atoi(argv[2]);

    if (n <= 0 || p <= 0 || n % p != 0) {
        printf("Erro: n deve ser divisivel por p.\n");
        return 1;
    }

    int *original = malloc(n * sizeof(int));
    int *serial = malloc(n * sizeof(int));
    int *omp_data = malloc(n * sizeof(int));
    int *acc_data = malloc(n * sizeof(int));

    if (!original || !serial || !omp_data || !acc_data) {
        perror("Erro ao alocar memoria");

        free(original);
        free(serial);
        free(omp_data);
        free(acc_data);

        return 1;
    }

    srand(42);

    gerar_dados(original, n);

    memcpy(serial, original, n * sizeof(int));
    memcpy(omp_data, original, n * sizeof(int));
    memcpy(acc_data, original, n * sizeof(int));

    printf("Odd-Even Transposition Sort\n");
    printf("Elementos: %d\n", n);
    printf("Blocos: %d\n", p);

    clock_t inicio_serial = clock();

    qsort(serial, n, sizeof(int), compare_int);

    double tempo_serial =
        (double)(clock() - inicio_serial) / CLOCKS_PER_SEC;

    printf("\n--- Versao Serial ---\n");
    printf("Verificacao: %s\n",
           verificar_ordenacao(serial, n) ? "Passou" : "Falhou");
    printf("Tempo: %.6f segundos\n", tempo_serial);

    omp_set_num_threads(p);

    double inicio_omp = omp_get_wtime();

    odd_even_sort_omp(omp_data, n, p);

    double tempo_omp = omp_get_wtime() - inicio_omp;

    apresentar_resultado(
        "OpenMP",
        tempo_serial,
        tempo_omp,
        p,
        memcmp(serial, omp_data, n * sizeof(int)) == 0
    );

    if (acc_get_num_devices(acc_device_nvidia) > 0) {
        acc_init(acc_device_nvidia);

        double inicio_acc = omp_get_wtime();

        odd_even_sort_acc(acc_data, n, p);

        double tempo_acc = omp_get_wtime() - inicio_acc;

        apresentar_resultado(
            "OpenACC",
            tempo_serial,
            tempo_acc,
            p,
            memcmp(serial, acc_data, n * sizeof(int)) == 0
        );
    } else {
        printf("\nGPU NVIDIA nao disponivel.\n");
    }

    free(original);
    free(serial);
    free(omp_data);
    free(acc_data);

    return 0;
}