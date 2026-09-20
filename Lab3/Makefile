CC = gcc
CFLAGS = -O2 -Wall -pthread -D_POSIX_C_SOURCE=199309L

TARGETS = contacorrente_seq contacorrente_f1 contacorrente_f2 primos soma_vetores busca_padrao

all: $(TARGETS)

contacorrente_seq: contacorrente_seq.c
	$(CC) $(CFLAGS) contacorrente_seq.c -o contacorrente_seq

contacorrente_f1: contacorrente_f1.c
	$(CC) $(CFLAGS) contacorrente_f1.c -o contacorrente_f1

contacorrente_f2: contacorrente_f2.c
	$(CC) $(CFLAGS) contacorrente_f2.c -o contacorrente_f2

primos: primos.c
	$(CC) $(CFLAGS) primos.c -o primos

soma_vetores: soma_vetores.c
	$(CC) $(CFLAGS) soma_vetores.c -o soma_vetores

busca_padrao: busca_padrao.c
	$(CC) $(CFLAGS) busca_padrao.c -o busca_padrao

clean:
	rm -f $(TARGETS)