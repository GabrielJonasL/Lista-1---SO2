#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_VAL 1000   //valor max esperado para histograma
#define P 4            //numero de threads (mudar para 1,2,4,8)

//passar dados para threads
typedef struct {
    int* dados;
    long inicio;
    long fim;
    long soma_local;
    int* hist_local;
} args_t;

int* dados;
long N;
long soma_global = 0;
int* hist_global;

//tempo
double tempo() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1e6;
}

//thread(map)
void* worker(void* arg) {
    args_t* a = (args_t*)arg;
    a->soma_local = 0;
    a->hist_local = calloc(MAX_VAL+1, sizeof(int));

    for (long i = a->inicio; i < a->fim; i++) {
        int v = a->dados[i];
        a->soma_local += v;
        if (v >= 0 && v <= MAX_VAL) {
            a->hist_local[v]++;
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: %s arquivo.txt\n", argv[0]);
        return 1;
    }

    //ler arquivo
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror("Erro ao abrir arquivo");
        return 1;
    }
    dados = malloc(10000000 * sizeof(int)); //ate 10M inteiros
    N = 0;
    while (fscanf(f, "%d", &dados[N]) == 1) {
        N++;
    }
    fclose(f);

    printf("Lidos %ld inteiros do arquivo.\n", N);

    //cria threads
    pthread_t threads[P];
    args_t args[P];

    long bloco = N / P;
    double t0 = tempo();

    for (int i = 0; i < P; i++) {
        args[i].dados = dados;
        args[i].inicio = i * bloco;
        args[i].fim = (i == P-1) ? N : (i+1) * bloco;
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }

    //espera threads terminarem
    hist_global = calloc(MAX_VAL+1, sizeof(int));
    for (int i = 0; i < P; i++) {
        pthread_join(threads[i], NULL);

        soma_global += args[i].soma_local;
        for (int v = 0; v <= MAX_VAL; v++) {
            hist_global[v] += args[i].hist_local[v];
        }
        free(args[i].hist_local);
    }

    double t1 = tempo();

    printf("Soma total = %ld\n", soma_global);
    printf("Histograma (valores não-nulos):\n");
    for (int v = 0; v <= MAX_VAL; v++) {
        if (hist_global[v] > 0) {
            printf("%d: %d\n", v, hist_global[v]);
        }
    }

    printf("Tempo com %d threads = %.3f s\n", P, t1 - t0);

    free(dados);
    free(hist_global);
    return 0;
}