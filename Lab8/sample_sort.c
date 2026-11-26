#include "comum.h"

void selecionar_splitters(int *samples,
                          int *splitters,
                          int p) {
    int total_samples = p * (p - 1);

    qsort(samples,
          total_samples,
          sizeof(int),
          compare_int);

    for (int i = 0; i < p - 1; i++) {
        splitters[i] = samples[(i + 1) * (p - 1)];
    }
}

#pragma acc routine seq
int encontrar_bucket(int valor, int *splitters, int p) {
    int bucket = 0;

    while (bucket < p - 1 &&
           valor > splitters[bucket]) {
        bucket++;
    }

    return bucket;
}

void calcular_deslocamentos(int *counts,
                            int *offsets,
                            int *bucket_start,
                            int p) {
    int posicao = 0;

    for (int j = 0; j < p; j++) {
        bucket_start[j] = posicao;

        for (int i = 0; i < p; i++) {
            offsets[i * p + j] = posicao;
            posicao += counts[i * p + j];
        }
    }

    bucket_start[p] = posicao;
}

void sample_sort_omp(int *data, int n, int p) {
    if (p == 1) {
        qsort(data, n, sizeof(int), compare_int);
        return;
    }

    int n_local = n / p;
    int total_samples = p * (p - 1);

    int *samples = malloc(total_samples * sizeof(int));
    int *splitters = malloc((p - 1) * sizeof(int));
    int *counts = calloc(p * p, sizeof(int));
    int *offsets = malloc(p * p * sizeof(int));
    int *bucket_start = malloc((p + 1) * sizeof(int));
    int *temp = malloc(n * sizeof(int));

    if (!samples || !splitters || !counts ||
        !offsets || !bucket_start || !temp) {
        perror("Erro ao alocar memoria");
        exit(1);
    }

    #pragma omp parallel
    {
        // Ordenacao local e selecao de amostras
        #pragma omp for
        for (int i = 0; i < p; i++) {
            int *local = data + i * n_local;

            qsort(local,
                  n_local,
                  sizeof(int),
                  compare_int);

            for (int j = 0; j < p - 1; j++) {
                int pos = (j + 1) * n_local / p;

                samples[i * (p - 1) + j] = local[pos];
            }
        }

        // Gather, selecao e broadcast dos splitters
        #pragma omp single
        {
            selecionar_splitters(samples, splitters, p);
        }

        // Particionamento local
        #pragma omp for
        for (int i = 0; i < p; i++) {
            for (int j = 0; j < n_local; j++) {
                int valor = data[i * n_local + j];

                int bucket =
                    encontrar_bucket(valor, splitters, p);

                counts[i * p + bucket]++;
            }
        }

        #pragma omp single
        {
            calcular_deslocamentos(
                counts,
                offsets,
                bucket_start,
                p
            );
        }

        // Redistribuicao all-to-all
        #pragma omp for
        for (int i = 0; i < p; i++) {
            for (int j = 0; j < n_local; j++) {
                int valor = data[i * n_local + j];

                int bucket =
                    encontrar_bucket(valor, splitters, p);

                int pos = offsets[i * p + bucket]++;

                temp[pos] = valor;
            }
        }

        // Ordenacao final dos buckets
        #pragma omp for
        for (int i = 0; i < p; i++) {
            int tamanho =
                bucket_start[i + 1] - bucket_start[i];

            qsort(temp + bucket_start[i],
                  tamanho,
                  sizeof(int),
                  compare_int);
        }

        #pragma omp for
        for (int i = 0; i < n; i++) {
            data[i] = temp[i];
        }
    }

    free(samples);
    free(splitters);
    free(counts);
    free(offsets);
    free(bucket_start);
    free(temp);
}

void sample_sort_acc(int *data, int n, int p) {
    if (p == 1) {
        #pragma acc data copy(data[0:n])
        {
            #pragma acc serial
            {
                ordenar_bloco_acc(data, n);
            }
        }

        return;
    }

    int n_local = n / p;
    int total_samples = p * (p - 1);
    int total_counts = p * p;

    int *samples = malloc(total_samples * sizeof(int));
    int *splitters = malloc((p - 1) * sizeof(int));
    int *counts = calloc(total_counts, sizeof(int));
    int *offsets = malloc(total_counts * sizeof(int));
    int *bucket_start = malloc((p + 1) * sizeof(int));
    int *temp = malloc(n * sizeof(int));

    if (!samples || !splitters || !counts ||
        !offsets || !bucket_start || !temp) {
        perror("Erro ao alocar memoria");
        exit(1);
    }

    #pragma acc data copy(data[0:n]) \
        copyin(counts[0:total_counts]) \
        create(samples[0:total_samples], \
               splitters[0:p-1], \
               offsets[0:total_counts], \
               bucket_start[0:p+1], \
               temp[0:n])
    {
        // Ordenacao local na GPU
        #pragma acc parallel loop gang
        for (int i = 0; i < p; i++) {
            ordenar_bloco_acc(
                data + i * n_local,
                n_local
            );
        }

        // Selecao de amostras na GPU
        #pragma acc parallel loop gang
        for (int i = 0; i < p; i++) {
            #pragma acc loop seq
            for (int j = 0; j < p - 1; j++) {
                int pos = (j + 1) * n_local / p;

                samples[i * (p - 1) + j] =
                    data[i * n_local + pos];
            }
        }

        // Gather das amostras para a CPU
        #pragma acc update self(samples[0:total_samples])

        selecionar_splitters(samples, splitters, p);

        // Broadcast dos splitters para a GPU
        #pragma acc update device(splitters[0:p-1])

        // Particionamento local na GPU
        #pragma acc parallel loop gang
        for (int i = 0; i < p; i++) {
            #pragma acc loop seq
            for (int j = 0; j < n_local; j++) {
                int valor = data[i * n_local + j];

                int bucket =
                    encontrar_bucket(valor, splitters, p);

                counts[i * p + bucket]++;
            }
        }

        #pragma acc update self(counts[0:total_counts])

        calcular_deslocamentos(
            counts,
            offsets,
            bucket_start,
            p
        );

        #pragma acc update device(offsets[0:total_counts], \
                                  bucket_start[0:p+1])

        // Redistribuicao all-to-all na GPU
        #pragma acc parallel loop gang
        for (int i = 0; i < p; i++) {
            #pragma acc loop seq
            for (int j = 0; j < n_local; j++) {
                int valor = data[i * n_local + j];

                int bucket =
                    encontrar_bucket(valor, splitters, p);

                int pos = offsets[i * p + bucket]++;

                temp[pos] = valor;
            }
        }

        // Ordenacao final na GPU
        #pragma acc parallel loop gang
        for (int i = 0; i < p; i++) {
            int tamanho =
                bucket_start[i + 1] - bucket_start[i];

            ordenar_bloco_acc(
                temp + bucket_start[i],
                tamanho
            );
        }

        #pragma acc parallel loop
        for (int i = 0; i < n; i++) {
            data[i] = temp[i];
        }
    }

    free(samples);
    free(splitters);
    free(counts);
    free(offsets);
    free(bucket_start);
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

    printf("Sample Sort\n");
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

    sample_sort_omp(omp_data, n, p);

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

        sample_sort_acc(acc_data, n, p);

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