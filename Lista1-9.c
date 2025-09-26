#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h> 

#define DURACAO_TESTE 60 //segundos

pthread_barrier_t barreira;
int rodadas = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int terminar_teste = 0; 

long get_time_sec() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec;
}

typedef struct {
    int id;
    int K;
} corredor_args;

void* corredor(void* arg) {
    corredor_args* c = (corredor_args*)arg;

    while (1) {
        usleep((rand() % 400 + 100) * 1000); // 100–500 ms

        //verifica se o tempo acabou antes de tentar a barreira
        pthread_mutex_lock(&lock);
        if (terminar_teste) {
            pthread_mutex_unlock(&lock);
            break; //sai do loop
        }
        pthread_mutex_unlock(&lock);
        
        //chega na barreira
        int r = pthread_barrier_wait(&barreira);

        //contagem da rodada e verificação de término
        if (r == PTHREAD_BARRIER_SERIAL_THREAD) {
            //thread serializada verifica a flag e conta
            pthread_mutex_lock(&lock);
            if (terminar_teste) {
                pthread_mutex_unlock(&lock);
                break; //sai do loop se o tempo acabou
            }
            rodadas++;
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    int K;

    printf("--- Simulação de Revezamento ---\n");
    printf("Digite o tamanho da equipe (K): ");
    
    if (scanf("%d", &K) != 1) {
        printf("Erro na leitura. Tamanho da equipe deve ser um número inteiro.\n");
        return 1;
    }
    
    if (K <= 0) {
        printf("K inválido. Use um número maior que zero.\n");
        return 1;
    }

    srand(time(NULL)); 
    pthread_barrier_init(&barreira, NULL, K);

    pthread_t corredores[K];
    corredor_args args[K];

    //cria as threads
    for (int i = 0; i < K; i++) {
        args[i].id = i;
        args[i].K = K;
        pthread_create(&corredores[i], NULL, corredor, &args[i]);
    }

    long inicio = get_time_sec();
    long fim = inicio + DURACAO_TESTE;

    printf("Iniciando simulação de %d segundos com %d corredores...\n", DURACAO_TESTE, K);

    //thread principal atua como o cronômetro
    while (get_time_sec() < fim) {
        sleep(1);
    }
    
    pthread_mutex_lock(&lock);
    terminar_teste = 1;
    pthread_mutex_unlock(&lock);
    
    //aguarda o término limpo das threads
    for (int i = 0; i < K; i++) {
        pthread_join(corredores[i], NULL);
    }

    pthread_barrier_destroy(&barreira);
    pthread_mutex_destroy(&lock);

    printf("\n=== Resultado Final ===\n");
    printf("Teste de %d segundos concluído.\n", DURACAO_TESTE);
    printf("Equipe com %d corredores\n", K);
    printf("Rodadas concluídas: %d\n", rodadas);
    printf("Rodadas por minuto: %.2f\n", rodadas * 60.0 / DURACAO_TESTE);

    return 0;
}