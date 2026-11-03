#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#ifndef NUM_PARTICLES
#define NUM_PARTICLES (1024LL * 1024 * 10)
#endif

#define DT 0.01
#define GRAVITY -9.81

typedef struct {
    double x, y, z;
    double vx, vy, vz;
} Particle;

int main(void) {
    Particle *particles =
        malloc((size_t)NUM_PARTICLES * sizeof(Particle));

    double *old_vy =
        malloc((size_t)NUM_PARTICLES * sizeof(double));

    if (particles == NULL || old_vy == NULL) {
        perror("Erro ao alocar memoria");
        free(particles);
        free(old_vy);
        return 1;
    }

    #pragma omp parallel for
    for (long long i = 0; i < NUM_PARTICLES; i++) {
        particles[i].x = (double)(i % 100);
        particles[i].y = (double)(i % 80);
        particles[i].z = (double)(i % 60);

        particles[i].vx = (double)(i % 10) - 5.0;
        particles[i].vy = (double)(i % 8) - 4.0;
        particles[i].vz = (double)(i % 6) - 3.0;

        old_vy[i] = particles[i].vy;
    }

    double inicio = omp_get_wtime();

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (long long i = 0; i < NUM_PARTICLES; i++) {
                particles[i].x += particles[i].vx * DT;
                particles[i].y += old_vy[i] * DT;
                particles[i].z += particles[i].vz * DT;
            }
        }

        #pragma omp section
        {
            for (long long i = 0; i < NUM_PARTICLES; i++) {
                particles[i].vy += GRAVITY * DT;
            }
        }
    }

    double tempo = omp_get_wtime() - inicio;

    printf("\n--- Versao Sections ---\n");
    printf("Particulas: %lld\n", (long long)NUM_PARTICLES);
    printf("Posicao 0: %.4f %.4f %.4f\n",
           particles[0].x, particles[0].y, particles[0].z);
    printf("Velocidade 0: %.4f %.4f %.4f\n",
           particles[0].vx, particles[0].vy, particles[0].vz);
    printf("Tempo: %.6f segundos\n", tempo);

    free(old_vy);
    free(particles);

    return 0;
}