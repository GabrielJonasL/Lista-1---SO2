#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#define NUM_RECURSOS 2
#define NUM_THREADS 2
#define TIMEOUT 5 //segundos sem progresso

pthread_mutex_t recursos[NUM_RECURSOS];
long ultimo_progresso = 0;
pthread_mutex_t lock_progresso = PTHREAD_MUTEX_INITIALIZER;
int rodando = 1;

long get_time_sec() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec;
}

void registrar_progresso() {
    pthread_mutex_lock(&lock_progresso);
    ultimo_progresso = get_time_sec();
    pthread_mutex_unlock(&lock_progresso);
}

void* trabalhador1(void* arg) {
    while (rodando) {
        // ordem: R1 -> R2
        pthread_mutex_lock(&recursos[0]);
        sleep(1);
        pthread_mutex_lock(&recursos[1]);

        printf("[T1] Usando R1 e R2\n");
        sleep(1);
        registrar_progresso();

        pthread_mutex_unlock(&recursos[1]);
        pthread_mutex_unlock(&recursos[0]);
        sleep(1);
    }
    return NULL;
}

void* trabalhador2(void* arg) {
    while (rodando) {
        //ordem inversa: R2 -> R1
        pthread_mutex_lock(&recursos[1]);
        sleep(1);
        pthread_mutex_lock(&recursos[0]);

        printf("[T2] Usando R2 e R1\n");
        sleep(1);
        registrar_progresso();

        pthread_mutex_unlock(&recursos[0]);
        pthread_mutex_unlock(&recursos[1]);
        sleep(1);
    }
    return NULL;
}

void* watchdog(void* arg) {
    while (rodando) {
        sleep(1);
        pthread_mutex_lock(&lock_progresso);
        long agora = get_time_sec();
        if (agora - ultimo_progresso > TIMEOUT) {
            printf("\n[WATCHDOG] Deadlock detectado! Sem progresso há %d segundos.\n", TIMEOUT);
            printf("Possíveis threads presas: T1 e T2 tentando recursos em ordens diferentes.\n");
            rodando = 0; //encerra a execução
        }
        pthread_mutex_unlock(&lock_progresso);
    }
    return NULL;
}

int main() {
    pthread_t t1, t2, wd;

    for (int i = 0; i < NUM_RECURSOS; i++) {
        pthread_mutex_init(&recursos[i], NULL);
    }

    ultimo_progresso = get_time_sec();

    pthread_create(&t1, NULL, trabalhador1, NULL);
    pthread_create(&t2, NULL, trabalhador2, NULL);
    pthread_create(&wd, NULL, watchdog, NULL);

    pthread_join(wd, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    for (int i = 0; i < NUM_RECURSOS; i++) {
        pthread_mutex_destroy(&recursos[i]);
    }
    return 0;
}