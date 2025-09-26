#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>

#define N 5              //filósofos
#define REFEICOES 20     //refeições

//qual solução usar: 1 para ordem global / 2 para semáforo
#define SOLUCAO 2 

pthread_mutex_t garfos[N];
sem_t sem;//solução b

//métricas
int refeicoes[N];
double maior_espera[N];
pthread_mutex_t metr_lock = PTHREAD_MUTEX_INITIALIZER;

//tempo
double tempo() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1e6;
}

void pensar() {
    usleep(50000 + rand() % 100000); //50ms a 150ms
}

void comer() {
    usleep(100000 + rand() % 200000); //100ms a 300ms
}

//atualiza as métricas de espera
void atualizar_metricas(int id, double t0_espera, double t1_espera) {
    pthread_mutex_lock(&metr_lock);
    refeicoes[id]++;
    double espera = t1_espera - t0_espera;
    if (espera > maior_espera[id]) {
        maior_espera[id] = espera;
    }
    pthread_mutex_unlock(&metr_lock);
}

//solução 1
void comer_ordem_global(int id) {
    int g1 = id;
    int g2 = (id + 1) % N;

    //garante que o garfo com o menor índice seja pego primeiro
    if (g2 < g1) {
        int tmp = g1; g1 = g2; g2 = tmp;
    }

    double t0 = tempo();
    pthread_mutex_lock(&garfos[g1]); //pega o garfo de menor índice
    pthread_mutex_lock(&garfos[g2]); //pega o garfo de maior índice
    double t1 = tempo();

    atualizar_metricas(id, t0, t1);
    
    comer();

    pthread_mutex_unlock(&garfos[g2]);
    pthread_mutex_unlock(&garfos[g1]);
}

//solução 2
void comer_semaforo(int id) {
    int g1 = id;
    int g2 = (id + 1) % N;

    //limite de N-1 filósofos tentando pegar garfos ao mesmo tempo
    sem_wait(&sem); 

    double t0 = tempo();
    pthread_mutex_lock(&garfos[g1]); //pega o garfo da esquerda
    pthread_mutex_lock(&garfos[g2]); //pega o garfo da direita
    double t1 = tempo();

    atualizar_metricas(id, t0, t1);
    
    comer();

    pthread_mutex_unlock(&garfos[g2]);
    pthread_mutex_unlock(&garfos[g1]);

    //libera a vaga na mesa
    sem_post(&sem);
}

void* filosofo(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < REFEICOES; i++) {
        pensar(); 

        //tenta comer usando a solução selecionada
        #if SOLUCAO == 1
            comer_ordem_global(id);
        #elif SOLUCAO == 2
            comer_semaforo(id);
        #else
            //testar o deadlock: sem solução
            int g1 = id;
            int g2 = (id + 1) % N;
            pthread_mutex_lock(&garfos[g1]);
            pthread_mutex_lock(&garfos[g2]);
            comer();
            pthread_mutex_unlock(&garfos[g2]);
            pthread_mutex_unlock(&garfos[g1]);
        #endif
    }
    return NULL;
}

int main() {
    //inicia o gerador de numeros aleatórios
    srand(time(NULL)); 

    pthread_t th[N];
    int id[N];

    //inicia garfos, estatísticas e IDs
    for (int i = 0; i < N; i++) {
        pthread_mutex_init(&garfos[i], NULL);
        refeicoes[i] = 0;
        maior_espera[i] = 0;
        id[i] = i;
    }

    //inicia o Semáforo (apenas para sluçao 2)
    sem_init(&sem, 0, N - 1); 

    //criação das threads
    for (int i = 0; i < N; i++) {
        pthread_create(&th[i], NULL, filosofo, &id[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(th[i], NULL);
    }

    for (int i = 0; i < N; i++) {
        pthread_mutex_destroy(&garfos[i]);
    }
    sem_destroy(&sem);
    pthread_mutex_destroy(&metr_lock);

    printf("\n=== Estatísticas (Solução %d: %s) ===\n", SOLUCAO, 
           (SOLUCAO == 1 ? "Ordem Global" : (SOLUCAO == 2 ? "Semaforo N-1" : "Sem Solucao")));

    for (int i = 0; i < N; i++) {
        printf("Filosofo %d: refeicoes=%d, maior espera=%.3fs\n",
               i, refeicoes[i], maior_espera[i]);
    }

    return 0;
}