
#!/bin/bash
set -e

echo "Compilando os programas..."

gcc -O3 -fopenmp analise_inicial.c -o analise_inicial -lm
gcc -O3 -fopenmp analise_sections.c -o analise_sections -lm

gcc -O3 -fopenmp simulacao_inicial.c -o simulacao_inicial
gcc -O3 -fopenmp simulacao_sections.c -o simulacao_sections

gcc -O3 -fopenmp contalinhas_inicial.c -o contalinhas_inicial
gcc -O3 -fopenmp contalinhas_tasks.c -o contalinhas_tasks

mkdir -p resultados
echo "Compilacao concluida!"

PROGRAMAS=(
    analise_inicial
    analise_sections
    simulacao_inicial
    simulacao_sections
    contalinhas_inicial
    contalinhas_tasks
)

# Diretorio usado na atividade 3
DIR_TESTE="${1:-.}"

for threads in 1 2 4
do
    export OMP_NUM_THREADS=$threads

    echo ""
    echo "===== TESTES COM $threads THREAD(S) ====="

    for programa in "${PROGRAMAS[@]}"
    do
        arquivo="resultados/${programa}_${threads}threads.txt"
        : > "$arquivo"

        echo "Testando $programa..."

        for execucao in 1 2 3 4 5
        do
            if [[ "$programa" == contalinhas_* ]]; then
                "./$programa" "$DIR_TESTE" >> "$arquivo"
            else
                "./$programa" >> "$arquivo"
            fi
        done

        media=$(awk '/^Tempo: / {
            soma += $2
            n++
        }
        END {
            if (n > 0)
                printf "%.6f", soma/n
        }' "$arquivo")

        echo "Media: $media segundos"
        echo "Media: $media segundos" >> "$arquivo"
    done
done

echo ""
echo "Testes concluidos!"
echo "Resultados disponiveis na pasta resultados/"
