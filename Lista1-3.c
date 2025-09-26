#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#define M 10        //n contas
#define T 5         //n threads
#define ITER 100000 //n transferências por thread
#define SALDO_INICIAL 1000

int contas[M];
pthread_mutex_t conta_mutex[M];

int usar_travas = 1; //1 = execucao correta, 0 = incorreta sem mutex

void transferir(int src, int dst, int valor) {
    if (usar_travas) {
        //evitar deadlock
        if (src < dst) {
            pthread_mutex_lock(&conta_mutex[src]);
            pthread_mutex_lock(&conta_mutex[dst]);
        } else {
            pthread_mutex_lock(&conta_mutex[dst]);
            pthread_mutex_lock(&conta_mutex[src]);
        }
    }

    if (contas[src] >= valor) {
        contas[src] -= valor;
        contas[dst] += valor;
    }

    if (usar_travas) {
        pthread_mutex_unlock(&conta_mutex[src]);
        pthread_mutex_unlock(&conta_mutex[dst]);
    }
}

void* thread_func(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < ITER; i++) {
        int src = rand() % M;
        int dst = rand() % M;
        if (src == dst) continue;
        int valor = (rand() % 100) + 1;

        transferir(src, dst, valor);
    }
    return NULL;
}

long soma_global() {
    long soma = 0;
    for (int i = 0; i < M; i++) soma += contas[i];
    return soma;
}

int main() {
    srand(time(NULL));

    //ini contas e mutex
    for (int i = 0; i < M; i++) {
        contas[i] = SALDO_INICIAL;
        pthread_mutex_init(&conta_mutex[i], NULL);
    }
    long soma_inicial = soma_global();

    pthread_t threads[T];
    int ids[T];
    for (int i = 0; i < T; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, thread_func, &ids[i]);
    }
    for (int i = 0; i < T; i++) {
        pthread_join(threads[i], NULL);
    }

    long soma_final = soma_global();

    printf("Execução %s travas\n", usar_travas ? "com" : "sem");
    printf("Soma inicial: %ld\n", soma_inicial);
    printf("Soma final:   %ld\n", soma_final);

    //verifica invariância
    assert(soma_inicial == soma_final && "Erro: soma global alterada por condição de corrida!");

    return 0;
}