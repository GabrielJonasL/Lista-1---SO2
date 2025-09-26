#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 20   //itens
#define Q1 5   //fila 1
#define Q2 5   //fila 2

//fila circular
typedef struct {
    int *dados;
    int tamanho;
    int in, out, count;
    pthread_mutex_t mutex;
    pthread_cond_t cheio, vazio;
} fila_t;

void fila_init(fila_t *f, int tamanho) {
    f->dados = malloc(tamanho * sizeof(int));
    f->tamanho = tamanho;
    f->in = f->out = f->count = 0;
    pthread_mutex_init(&f->mutex, NULL);
    pthread_cond_init(&f->cheio, NULL);
    pthread_cond_init(&f->vazio, NULL);
}

void fila_push(fila_t *f, int item) {
    pthread_mutex_lock(&f->mutex);
    while (f->count == f->tamanho) {
        pthread_cond_wait(&f->cheio, &f->mutex);
    }
    f->dados[f->in] = item;
    f->in = (f->in + 1) % f->tamanho;
    f->count++;
    pthread_cond_signal(&f->vazio);
    pthread_mutex_unlock(&f->mutex);
}

int fila_pop(fila_t *f) {
    pthread_mutex_lock(&f->mutex);
    while (f->count == 0) {
        pthread_cond_wait(&f->vazio, &f->mutex);
    }
    int item = f->dados[f->out];
    f->out = (f->out + 1) % f->tamanho;
    f->count--;
    pthread_cond_signal(&f->cheio);
    pthread_mutex_unlock(&f->mutex);
    return item;
}

//filas globais
fila_t fila1, fila2;

//thread captura
void* captura(void* arg) {
    for (int i = 0; i < N; i++) {
        usleep(100000);
        printf("[Captura] Gerou item %d\n", i);
        fila_push(&fila1, i);
    }
    //poison pill
    fila_push(&fila1, -1);
    return NULL;
}

//thread processamento
void* processamento(void* arg) {
    while (1) {
        int item = fila_pop(&fila1);
        if (item == -1) { //poison pill
            fila_push(&fila2, -1);
            break;
        }
        usleep(150000); //simula processamento
        int resultado = item * 2;
        printf("   [Processamento] Processou %d -> %d\n", item, resultado);
        fila_push(&fila2, resultado);
    }
    return NULL;
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