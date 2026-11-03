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

    if (particles == NULL) {
        perror("Erro ao alocar memoria");
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
    }

    double inicio = omp_get_wtime();

    #pragma omp parallel for
    for (long long i = 0; i < NUM_PARTICLES; i++) {
        particles[i].x += particles[i].vx * DT;
        particles[i].y += particles[i].vy * DT;
        particles[i].z += particles[i].vz * DT;
    }

    #pragma omp parallel for
    for (long long i = 0; i < NUM_PARTICLES; i++) {
        particles[i].vy += GRAVITY * DT;
    }

    double tempo = omp_get_wtime() - inicio;

    printf("\n--- Versao Inicial ---\n");
    printf("Particulas: %lld\n", (long long)NUM_PARTICLES);
    printf("Posicao 0: %.4f %.4f %.4f\n",
           particles[0].x, particles[0].y, particles[0].z);
    printf("Velocidade 0: %.4f %.4f %.4f\n",
           particles[0].vx, particles[0].vy, particles[0].vz);
    printf("Tempo: %.6f segundos\n", tempo);

    free(particles);

    return 0;
}