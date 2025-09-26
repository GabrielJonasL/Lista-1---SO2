#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#define NUM_PRODUTORES 3
#define NUM_CONSUMIDORES 3
#define NUM_ITENS 50 //total de itens que cada produtor gera

//backpressure watermarks
#define HIGH_WATERMARK_FRAC 0.8
#define LOW_WATERMARK_FRAC  0.4

//amostragem para log
#define SAMPLE_INTERVAL_MS 50

//armazena o item e seu timestamp
typedef struct {
    int item_id;
    long production_time;
} BufferItem;

BufferItem *buffer;
int N;
int in = 0, out = 0;
int count = 0; //numero de itens atualmente no buffer

//sincronização
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t sem_empty, sem_full;
pthread_cond_t cond_bp = PTHREAD_COND_INITIALIZER; //backpressure cond

//estatisticas
long total_wait_time = 0;
long total_items_processed = 0;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

//controle de término
int producers_done = 0;
int consumers_done = 0;

//função auxiliar para o tempo total em microssegundos
long get_micros() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (t.tv_sec * 1000000L) + t.tv_usec;
}

//registra ocupação do buffer em CSV
void* monitor_thread(void* arg) {
    FILE *f = fopen("occupancy.csv", "w");
    if (!f) {
        perror("Não foi possível abrir occupancy.csv");
        return NULL;
    }
    fprintf(f, "time_us,count\n");
    long start = get_micros();

    while (1) {
        long now = get_micros() - start;
        pthread_mutex_lock(&mutex);
        int local_count = count;
        int done = (consumers_done == NUM_CONSUMIDORES && producers_done);
        pthread_mutex_unlock(&mutex);

        fprintf(f, "%ld,%d\n", now, local_count);
        fflush(f);

        if (done) break;
        usleep(SAMPLE_INTERVAL_MS * 1000);
    }

    fclose(f);
    return NULL;
}

//produtor com bursts e backpressure
void* produtor(void* arg) {
    int id = *(int*)arg;
    //parametros de burst/idle
    int max_burst = 8; //maximo itens em burst
    int min_burst = 3;
    int idle_min_ms = 200;
    int idle_max_ms = 800;

    for (int i = 0; i < NUM_ITENS; ) {
        int burst = (rand() % (max_burst - min_burst + 1)) + min_burst;
        for (int b = 0; b < burst && i < NUM_ITENS; b++, i++) {
            pthread_mutex_lock(&mutex);//antes de produzir, verificar backpressure watermarks
            int high = (int)(HIGH_WATERMARK_FRAC * N);
            int low  = (int)(LOW_WATERMARK_FRAC  * N);
            while (count >= high) {
                //espera passiva ate que o buffer caia abaixo do LOW_WATERMARK
                pthread_cond_wait(&cond_bp, &mutex);
            }
            pthread_mutex_unlock(&mutex);

            //criação do item
            BufferItem produced_item;
            produced_item.item_id = rand() % 1000000;
            produced_item.production_time = get_micros();

            //semáforo espera espaço vazio
            sem_wait(&sem_empty);
            pthread_mutex_lock(&mutex);

            //insere item no buffer
            buffer[in] = produced_item;
            in = (in + 1) % N;
            count++;

            //sinaliza consumo possível
            pthread_mutex_unlock(&mutex);
            sem_post(&sem_full);

            printf("[Produtor %d] produziu item %d (count=%d)\n", id, produced_item.item_id, count);
            usleep((rand() % 20 + 1) * 1000); // 1..20 ms
        }

        //idle after burst
        int idle_ms = (rand() % (idle_max_ms - idle_min_ms + 1)) + idle_min_ms;
        usleep(idle_ms * 1000);
    }

    return NULL;
}

//consome até receber poison pill
void* consumidor(void* arg) {
    int id = *(int*)arg;

    while (1) {
        sem_wait(&sem_full);
        pthread_mutex_lock(&mutex);

        //remove item do buffer
        BufferItem consumed_item = buffer[out];
        out = (out + 1) % N;
        count--;

        //se abaixo do low watermark, acorda produtores esperando
        int low = (int)(LOW_WATERMARK_FRAC * N);
        if (count <= low) {
            pthread_cond_broadcast(&cond_bp);
        }

        pthread_mutex_unlock(&mutex);
        sem_post(&sem_empty);

        //verifica poison pill
        if (consumed_item.item_id == -1) {
            //encontrou sinal de término
            printf("  [Consumidor %d] recebeu poison pill e encerra.\n", id);
            break;
        }

        long consumption_time = get_micros();
        long wait_time = consumption_time - consumed_item.production_time;

        pthread_mutex_lock(&stats_mutex);
        total_wait_time += wait_time;
        total_items_processed++;
        pthread_mutex_unlock(&stats_mutex);

        printf("  [Consumidor %d] consumiu item %d (Espera: %ld µs) (count=%d)\n",
               id, consumed_item.item_id, wait_time, count);

        //tempo de consumo variável
        usleep((rand() % 200 + 100) * 1000); //100..300 ms
    }

    pthread_mutex_lock(&mutex);
    consumers_done++;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    srand(time(NULL));

    printf("Digite o tamanho do buffer: ");
    if (scanf("%d", &N) != 1 || N <= 0) {
        printf("Tamanho inválido.\n");
        return 1;
    }
    buffer = malloc(N * sizeof(BufferItem));
    if (!buffer) { perror("malloc"); return 1; }

    //inicia os semáforos
    sem_init(&sem_empty, 0, N);
    sem_init(&sem_full, 0, 0);

    long start_micros = get_micros();

    pthread_t prod[NUM_PRODUTORES], cons[NUM_CONSUMIDORES], mon;
    int ids_prod[NUM_PRODUTORES], ids_cons[NUM_CONSUMIDORES];

    //cria monitor
    pthread_create(&mon, NULL, monitor_thread, NULL);

    //cria as threads produtoras
    for (int i = 0; i < NUM_PRODUTORES; i++) {
        ids_prod[i] = i;
        pthread_create(&prod[i], NULL, produtor, &ids_prod[i]);
    }
    //cria as threads consumidoras
    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        ids_cons[i] = i;
        pthread_create(&cons[i], NULL, consumidor, &ids_cons[i]);
    }

    //espera produtores terminarem
    for (int i = 0; i < NUM_PRODUTORES; i++) {
        pthread_join(prod[i], NULL);
    }

    //depois que todos os produtores finalizaram, marca producers_done
    pthread_mutex_lock(&mutex);
    producers_done = 1;
    pthread_mutex_unlock(&mutex);

    //envia poison pills para consumidores
    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        BufferItem pill;
        pill.item_id = -1;
        pill.production_time = get_micros();

        sem_wait(&sem_empty);
        pthread_mutex_lock(&mutex);

        buffer[in] = pill;
        in = (in + 1) % N;
        count++;

        pthread_mutex_unlock(&mutex);
        sem_post(&sem_full);
    }

    //espera consumidores terminarem
    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        pthread_join(cons[i], NULL);
    }

    pthread_join(mon, NULL);

    long end_micros = get_micros();
    double elapsed_seconds = (double)(end_micros - start_micros) / 1000000.0;

    printf("\n=== Estatísticas Finais ===\n");
    printf("Itens produzidos (Total Esperado): %d\n", NUM_ITENS * NUM_PRODUTORES);
    printf("Itens processados (Contado): %ld\n", total_items_processed);
    printf("Tempo total: %.3f s\n", elapsed_seconds);
    printf("Throughput: %.2f itens/s\n", total_items_processed / elapsed_seconds);

    //tempo medio de espera
    if (total_items_processed > 0) {
        printf("Tempo medio de espera (Fila): %.2f µs/item\n",
               (double)total_wait_time / total_items_processed);
    } else {
         printf("Nenhum item processado para calcular o tempo medio de espera.\n");
    }

    free(buffer);
    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&stats_mutex);
    pthread_cond_destroy(&cond_bp);

    printf("Ocupação ao longo do tempo registrada em occupancy.csv\n");
    return 0;
}