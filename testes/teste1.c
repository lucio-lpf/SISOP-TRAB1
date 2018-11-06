//BASEADO NO TESTE DO COLEGA DE INTITUTO ARTHUR VEDANA
#include "../include/cthread.h"
#include "../include/support.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void *func(void *i) {
	srand(time(NULL));

	int n = *(int *)i;

	int div;
	for (div = 2; div < 10; div++)
		if (n % div == 0) {
			printf("Thread %02d vai ceder\n", n);
			cyield();
		} else {
		    if (n > 9) break;
			int r = rand() % 10 + 1;
			printf("Thread %d vai tentar esperar a thread %d\n", n, r);
			if (cjoin(r) == 0) {
				printf("Thread %d teve de esperar a thread %d\n", n, r);
				break;
			} else
				printf("Thread %d nao teve de esperar a thread %d\n", n, r);
		}
	printf("Thread %02d executando, vai encerrar\n", n);

	return NULL;
}

int main(int argc, char **argv) {

	int ids[10];

	int i;
	for (i = 0; i < 10; i++) {
		ids[i] = ccreate(func, (void *)&ids[i], 1);
		if (ids[i] < 0)
			printf("Erro criando thread %02d, fechando.", i);
		else // imprime i + 1 pois a thread 0 é a main
			printf("Thread %02d criada com sucesso\n", i+1);
	}

	for (i = 0; i < 10; i++) {
		if (cjoin(ids[i]) == 0)
			printf("A main teve de esperar a thread %02d\n", i+1);
		else
			printf("A thread %02d estava encerrada, nao precisou esperar\n", i+1);
	}

	printf("Retornando a main\n");

	return 0;
}
