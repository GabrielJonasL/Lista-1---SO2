#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#define NUM_PRODUTORES 3
#define NUM_CONSUMIDORES 3
#define NUM_ITENS 50 //total de itens que cda produtor gera

//armazena o item e seu timestamp
typedef struct {
    int item_id;
    long production_time;
} BufferItem;

BufferItem *buffer;
int N;
int in = 0, out = 0;

//sincronização
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t sem_empty, sem_full;

//estatisticas
long total_wait_time = 0;
long total_items_processed = 0;

//função auxiliar para o tempo total em microsegundos
long get_micros() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (t.tv_sec * 1000000L) + t.tv_usec;
}

void* produtor(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < NUM_ITENS; i++) {
        usleep((rand() % 200 + 100) * 1000); //tempo de produção

        //cria o item e registra o tempo
        BufferItem produced_item;
        produced_item.item_id = rand() % 1000;
        produced_item.production_time = get_micros();

        //espera por espaço vazio
        sem_wait(&sem_empty);
        pthread_mutex_lock(&mutex);

        //insere o item no buffer
        buffer[in] = produced_item;
        in = (in + 1) % N;

        pthread_mutex_unlock(&mutex);
        sem_post(&sem_full); //sinaliza que ha um item cheio

        printf("[Produtor %d] produziu item %d\n", id, produced_item.item_id);
    }
    return NULL;
}

void* consumidor(void* arg) {
    int id = *(int*)arg;
    
    for (int i = 0; i < NUM_ITENS * (NUM_PRODUTORES / NUM_CONSUMIDORES); i++) {
        
        //espera por item cheio
        sem_wait(&sem_full);
        pthread_mutex_lock(&mutex);

        //remove o item do buffer
        BufferItem consumed_item = buffer[out];
        out = (out + 1) % N;

        //atualiza estatísticas
        long consumption_time = get_micros();
        long wait_time = consumption_time - consumed_item.production_time;

        total_wait_time += wait_time;
        total_items_processed++;

        pthread_mutex_unlock(&mutex);
        sem_post(&sem_empty); //sinaliza que ha espaço vazio

        printf("  [Consumidor %d] consumiu item %d (Espera: %ld µs)\n", 
               id, consumed_item.item_id, wait_time);
        usleep((rand() % 200 + 100) * 1000); //tempo de consumo
    }
    return NULL;
}

int main() {
    srand(time(NULL));

    printf("Digite o tamanho do buffer: ");
    scanf("%d", &N);
    buffer = malloc(N * sizeof(BufferItem));

    //inicia os semáforos
    sem_init(&sem_empty, 0, N);
    sem_init(&sem_full, 0, 0);

    long start_micros = get_micros();

    pthread_t prod[NUM_PRODUTORES], cons[NUM_CONSUMIDORES];
    int ids_prod[NUM_PRODUTORES], ids_cons[NUM_CONSUMIDORES];

    //cria as threads
    for (int i = 0; i < NUM_PRODUTORES; i++) {
        ids_prod[i] = i;
        pthread_create(&prod[i], NULL, produtor, &ids_prod[i]);
    }
    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        ids_cons[i] = i;
        pthread_create(&cons[i], NULL, consumidor, &ids_cons[i]);
    }

    //espera as threads de produtores terminarem
    for (int i = 0; i < NUM_PRODUTORES; i++) {
        pthread_join(prod[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        pthread_join(cons[i], NULL);
    }

    long end_micros = get_micros();
    double elapsed_seconds = (double)(end_micros - start_micros) / 1000000.0;

    printf("\n=== Estatísticas Finais ===\n");
    printf("Itens processados (Total Esperado): %d\n", NUM_ITENS * NUM_PRODUTORES);
    printf("Itens processados (Contado): %ld\n", total_items_processed);
    printf("Tempo total: %.3f s\n", elapsed_seconds);
    printf("Throughput: %.2f itens/s\n", total_items_processed / elapsed_seconds);
    
    //tempo medio de espera
    if (total_items_processed > 0) {
        printf("Tempo medio de espera (Latencia de Fila): %.2f µs/item\n",
               (double)total_wait_time / total_items_processed);
    } else {
         printf("Nenhum item processado para calcular o tempo medio de espera.\n");
    }

    free(buffer);
    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
    pthread_mutex_destroy(&mutex);

    return 0;
}
