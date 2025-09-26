#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_CAVALOS 5
#define DISTANCIA 50
#define MAX_PASSO 5

int distancias[NUM_CAVALOS] = {0};
int vencedor = -1;
int corrida_iniciada = 0;
int corrida_terminada = 0; //nova flag para sinalizar fim da corrida

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_largada = PTHREAD_COND_INITIALIZER;

//função de cada cavalo
void* correr(void* arg) {
    int id = *(int*)arg;
    int passo;

    //largada sincronizada
    pthread_mutex_lock(&mutex);
    while (!corrida_iniciada) {
        pthread_cond_wait(&cond_largada, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    //corrida
    while (1) {
        //verifica se a corrida terminou
        pthread_mutex_lock(&mutex);
        if (corrida_terminada) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        //passos aleatórios
        passo = (rand() % MAX_PASSO) + 1;
        usleep(100000 + (rand() % 200000)); //tempo de corrida

        //posição
        pthread_mutex_lock(&mutex);
        distancias[id] += passo;
        printf("Cavalo %d na posicao %d\n", id, distancias[id]);

        //verifica se cruzou a linha de chegada
        if (distancias[id] >= DISTANCIA) {
            if (vencedor == -1) {
                vencedor = id;
                corrida_terminada = 1;
                printf("\n>>> O Cavalo %d cruzou a linha de chegada! <<<\n", id);
            }
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_CAVALOS];
    int ids[NUM_CAVALOS];
    int aposta;
    
    srand(time(NULL));

    //aposta do usuario
    printf("=== CORRIDA DE CAVALOS ===\n");
    printf("Escolha seu cavalo (0 a %d): ", NUM_CAVALOS - 1);
    scanf("%d", &aposta);

    if (aposta < 0 || aposta >= NUM_CAVALOS) {
        printf("Aposta invalida!\n");
        return 1;
    }

    //threads
    for (int i = 0; i < NUM_CAVALOS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, correr, &ids[i]);
    }

    printf("Preparar...\n");
    sleep(1);
    printf("Apontar...\n");
    sleep(1);
    printf("LARGADA!\n\n");
    
    //libera a largada
    pthread_mutex_lock(&mutex);
    corrida_iniciada = 1;
    pthread_cond_broadcast(&cond_largada);
    pthread_mutex_unlock(&mutex);

    for (int i = 0; i < NUM_CAVALOS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n=== RESULTADO FINAL ===\n");
    printf("O vencedor e o Cavalo %d!\n", vencedor);

    if (aposta == vencedor) {
        printf("Parabens! Voce acertou sua aposta!\n");
    } else {
        printf("Que pena! Voce perdeu a aposta.\n");
    }

    //limpeza de recursos
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_largada);

    return 0;
}
