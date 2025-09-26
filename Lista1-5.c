#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct tarefa {
    int valor;//numero a processar
    struct tarefa* prox;
} tarefa_t;

typedef struct {
    tarefa_t* frente;
    tarefa_t* tras;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int fechar;//sem mais tarefas
} fila_t;

void fila_init(fila_t* f) {
    f->frente = f->tras = NULL;
    pthread_mutex_init(&f->mutex, NULL);
    pthread_cond_init(&f->cond, NULL);
    f->fechar = 0;
}

void fila_push(fila_t* f, int valor) {
    tarefa_t* nova = malloc(sizeof(tarefa_t));
    nova->valor = valor;
    nova->prox = NULL;

    pthread_mutex_lock(&f->mutex);
    if (f->tras) {
        f->tras->prox = nova;
    } else {
        f->frente = nova;
    }
    f->tras = nova;
    pthread_cond_signal(&f->cond);
    pthread_mutex_unlock(&f->mutex);
}

int fila_pop(fila_t* f, int* valor) {
    pthread_mutex_lock(&f->mutex);
    while (!f->frente && !f->fechar) {
        pthread_cond_wait(&f->cond, &f->mutex);
    }
    if (f->fechar && !f->frente) {
        pthread_mutex_unlock(&f->mutex);
        return 0; //nenhuma tarefa disponível
    }
    tarefa_t* t = f->frente;
    *valor = t->valor;
    f->frente = t->prox;
    if (!f->frente) f->tras = NULL;
    free(t);
    pthread_mutex_unlock(&f->mutex);
    return 1;
}

void fila_fechar(fila_t* f) {
    pthread_mutex_lock(&f->mutex);
    f->fechar = 1;
    pthread_cond_broadcast(&f->cond);
    pthread_mutex_unlock(&f->mutex);
}

//CPU-bound
int eh_primo(int n) {
    if (n < 2) return 0;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

//thread pool
#define N_THREADS 4
fila_t fila;

void* worker(void* arg) {
    int id = *(int*)arg;
    int valor;
    while (fila_pop(&fila, &valor)) {
        //testar primalidade
        int primo = eh_primo(valor);
        printf("[Thread %d] %d -> %s\n", id, valor, primo ? "PRIMO" : "NAO PRIMO");
        //simula carga CPU-bound
        usleep(100000);
    }
    printf("[Thread %d] Encerrando.\n", id);
    return NULL;
}

int main() {
    pthread_t threads[N_THREADS];
    int ids[N_THREADS];

    fila_init(&fila);
    
    //pool fixo
    for (int i = 0; i < N_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }
    
    printf("Digite uma lista de numeros inteiros para processar.\n");
    printf("O processamento de cada numero sera delegado a uma das %d threads.\n", N_THREADS);
    printf("Pressione CTRL+D (ou CTRL+Z no Windows) e ENTER para finalizar a entrada de dados.\n\n");
    
    //entrada até EOF
    int num;
    while (scanf("%d", &num) == 1) {
        fila_push(&fila, num);
    }

    fila_fechar(&fila);

    //aguarda threads
    for (int i = 0; i < N_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n=== Todas as tarefas foram processadas sem perda ===\n");
    return 0;
}

//thread gravação
void* gravacao(void* arg) {
    while (1) {
        int item = fila_pop(&fila2);
        if (item == -1) break;
        usleep(120000);
        printf("      [Gravação] Gravou item %d\n", item);
    }
    return NULL;
}

int main() {
    pthread_t tcap, tproc, tgrav;

    fila_init(&fila1, Q1);
    fila_init(&fila2, Q2);

    pthread_create(&tcap, NULL, captura, NULL);
    pthread_create(&tproc, NULL, processamento, NULL);
    pthread_create(&tgrav, NULL, gravacao, NULL);

    pthread_join(tcap, NULL);
    pthread_join(tproc, NULL);
    pthread_join(tgrav, NULL);

    printf("\n=== Fim do pipeline ===\n");

    free(fila1.dados);
    free(fila2.dados);
    return 0;
}