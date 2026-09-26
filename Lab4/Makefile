CC = gcc
CFLAGS_O0 = -O0 -Wall -pthread -D_POSIX_C_SOURCE=199309L
CFLAGS_O3 = -O3 -Wall -pthread -D_POSIX_C_SOURCE=199309L

all: varredura matmul_o0 matmul_o3 matmul_pthreads

varredura: varredura_linha.c varredura_coluna.c
	$(CC) $(CFLAGS_O3) varredura_linha.c -o varredura_linha
	$(CC) $(CFLAGS_O3) varredura_coluna.c -o varredura_coluna

matmul_o0: matmul_padrao.c matmul_bloco.c
	$(CC) $(CFLAGS_O0) matmul_padrao.c -o matmul_padrao_O0
	$(CC) $(CFLAGS_O0) matmul_bloco.c -o matmul_bloco_O0

matmul_o3: matmul_padrao.c matmul_bloco.c
	$(CC) $(CFLAGS_O3) matmul_padrao.c -o matmul_padrao_O3
	$(CC) $(CFLAGS_O3) matmul_bloco.c -o matmul_bloco_O3

matmul_pthreads: matmul_pthreads.c
	$(CC) $(CFLAGS_O3) matmul_pthreads.c -o matmul_pthreads

clean:
	rm -f varredura_linha varredura_coluna matmul_padrao_O0 matmul_bloco_O0 matmul_padrao_O3 matmul_bloco_O3 matmul_pthreads