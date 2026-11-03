#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <omp.h>

long long count_lines_in_file(const char *filepath) {
    FILE *fp = fopen(filepath, "r");

    if (fp == NULL)
        return 0;

    long long count = 0;
    int ch;

    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n')
            count++;
    }

    fclose(fp);

    return count;
}

int is_source_file(const char *filename) {
    size_t len = strlen(filename);

    if (len < 2)
        return 0;

    return strcmp(filename + len - 2, ".c") == 0 ||
           strcmp(filename + len - 2, ".h") == 0;
}

void traverse_directory(const char *dirpath,
                      long long *total_lines) {
    struct dirent **namelist;

    int n = scandir(dirpath, &namelist, NULL, alphasort);

    if (n < 0) {
        fprintf(stderr, "Erro ao acessar %s: %s\n",
                dirpath, strerror(errno));
        return;
    }

    for (int i = 0; i < n; i++) {
        const char *name = namelist[i]->d_name;

        if (strcmp(name, ".") == 0 ||
            strcmp(name, "..") == 0) {
            free(namelist[i]);
            continue;
        }

        size_t size = strlen(dirpath) + strlen(name) + 2;
        char *fullpath = malloc(size);

        if (fullpath == NULL) {
            free(namelist[i]);
            continue;
        }

        snprintf(fullpath, size, "%s/%s", dirpath, name);

        struct stat path_stat;

        if (lstat(fullpath, &path_stat) == 0) {
            if (S_ISDIR(path_stat.st_mode)) {

                #pragma omp task firstprivate(fullpath, total_lines)
                {
                    traverse_directory(fullpath, total_lines);
                    free(fullpath);
                }

                fullpath = NULL;
            }
            else if (S_ISREG(path_stat.st_mode) &&
                     is_source_file(name)) {

                #pragma omp task firstprivate(fullpath, total_lines)
                {
                    long long linhas = count_lines_in_file(fullpath);

                    #pragma omp atomic update
                    *total_lines += linhas;

                    free(fullpath);
                }

                fullpath = NULL;
            }
        }

        free(fullpath);
        free(namelist[i]);
    }

    free(namelist);
}

int main(int argc, char *argv[]) {
    const char *start_dir = ".";

    if (argc > 1)
        start_dir = argv[1];

    long long total_lines = 0;

    printf("Versao OpenMP Tasks\n");
    printf("Diretorio: %s\n", start_dir);

    double inicio = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                traverse_directory(start_dir, &total_lines);
            }
        }
    }

    double tempo = omp_get_wtime() - inicio;

    printf("\nTotal de linhas: %lld\n", total_lines);
    printf("Tempo: %.6f segundos\n", tempo);

    return 0;
}