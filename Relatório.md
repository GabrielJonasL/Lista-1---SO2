# Lista-1---SO2
Repositório contendo questões da primeira lista da matéria de Sistemas Operacionasi II

1>

Como Compilar/Rodar: Para compilar este programa, é necessário vincular a biblioteca de threads POSIX. No Linux, use o seguinte comando no terminal: gcc -o Lista1-1 Lista1-1.c -pthread
Após a compilação, execute o programa com:./Lista1-1

Decisões de Sincronização:

  Mutex: Usado para garantir a exclusão mútua. Sempre que uma thread precisa acessar ou modificar uma variável compartilhada(distancias[]; vencedor; corrida_terminada), ela primeiro adquire o lock do mutex. Isso impede que outras threads acessem os mesmos dados simultaneamente, prevenindo condições de corrida. O pthread_mutex_lock e o pthread_mutex_unlock delimitam a seção crítica do código.

  Variável de Condição: Usada para a sincronização de largada. As threads dos cavalos são criadas, mas elas não começam a correr imediatamente. Elas ficam em um estado de espera até que a thread principal, após a contagem regressiva, envie um sinal de broadcast. Esse sinal acorda todas as threads de uma vez, garantindo que a corrida inicie de forma simultânea para todos os participantes.

Evidências de Execução: A execução do programa é acompanhada por printfs que agem como um sistema de logging.

  Log de Posição: A cada passo, cada thread imprime a sua própria posição. Isso demonstra que as threads estão executando de forma concorrente, e a ordem dos prints é imprevisível.

  Anúncio do Vencedor: O printf especial(O Cavalo X cruzou a linha de chegada!) marca o exato momento em que a primeira thread atingiu ou ultrapassou a distância final. Como essa ação está dentro de uma seção crítica protegida pelo mutex, ela é atômica e garante que o vencedor seja registrado sem risco de sobreposição por outras threads. 

Análise dos Resultados: A lógica de sincronização implementada no código produz um resultado determinístico para o vencedor.

  Vencedor Único: O uso da flag vencedor = -1 (inicialmente) e a condição if (vencedor == -1) dentro da seção crítica garantem que apenas a primeira thread a alcançar essa seção defina o vencedor. As outras threads que chegarem logo depois encontrarão a flag já alterada e não poderão mudar o resultado.

  Finalização da Corrida: A flag corrida_terminada é essencial para o encerramento das threads. Assim que um vencedor é declarado, as threads remanescentes detectam a mudança da flag e terminam sua execução, evitando um laço infinito e liberando recursos.

  Aposta: A lógica final de verificação da aposta é simples e direta. Se a aposta do usuário for igual ao ID do cavalo vencedor, a aposta é considerada correta.

2>

Como Compilar/Rodar: Este programa utiliza as bibliotecas de threads POSIX (pthread) e semáforos POSIX. No Linux, use o seguinte comando: gcc -o Lista1-2 Lista1-2.c -pthread e então ./Lista1-2

Decisões de Sincronização: A solução produtor-consumidor requer a coordenação de acesso ao buffer e a sinalização de sua capacidade/ocupação. O código usa a combinação ideal de Mutexes e Semáforos para atingir esse objetivo.

Evidências de Execução:

  Logs Concorrentes: O log mostra que as mensagens de 'produtor x' e 'consumidor y' se intercalam de forma não determinística, provando a execução concorrente das threads.

  Latência: A coluna 'Espera: X µs' é a evidência crucial. Ela mostra o tempo que o item levou para ser consumido, calculado com precisão através dos timestamps de produção e consumo.

Análise dos Resultados: O programa processa corretamente o número esperado de itens. A garantia da exclusão mútua e o controle de capacidade previnem que o buffer sofra underflow ou overflow.

3>

Como Compilar/Rodar:

Decisões de Sincronização:

Evidências de Execução:

Análise dos Resultados:

4>

Como Compilar/Rodar: O código utiliza a biblioteca POSIX Threads, que requer uma flag específica durante a compilação para ser vinculada corretamente ao executável.

Passos para Compilação e Execução:

Salve o código-fonte em um arquivo, por exemplo, Lista1-4.c.

Navegue até o diretório onde o arquivo foi salvo.

Use o compilador GCC com a flag -pthread para incluir as bibliotecas de threads.

Então gcc -o Lista1-4 Lista1-4.c -pthread Executar: Inicie o programa a partir do terminal. ./Lista1-4

Decisões de Sincronização: O desafio de sincronização neste código é gerenciar o acesso a duas filas compartilhadas por múltiplas threads, evitando condições de corrida e o consumo ineficiente de CPU.

  Mutex: É usado para garantir exclusão mútua. Sempre que uma thread precisa modificar a estrutura interna da fila, ela deve primeiro adquirir o mutex. Isso impede que duas threads alterem a fila simultaneamente, o que corromperia seu estado. A seção crítica protegida pelo mutex é mantida a menor possível para maximizar a concorrência.

Variáveis de Condição: São o mecanismo chave para a eficiência. Sem elas, uma thread que encontrasse uma fila cheia ou vazia teria que ficar em um loop, desperdiçando ciclos de CPU. As variáveis de condição permitem que a thread durma até que a condição que a bloqueia mude. pthread_cond_wait atomicamente libera o mutex, permitindo que outra thread entre e altere o estado da fila. Quando a condição muda, a outra thread é acordada com um sinal.

Evidências de Execução: $ ./pipeline_processamento
[Captura] Gerou item 0
[Captura] Gerou item 1
    [Processamento] Processou 0 -> 0
[Captura] Gerou item 2
        [Gravação] Gravou item 0
    [Processamento] Processou 1 -> 2
[Captura] Gerou item 3
        [Gravação] Gravou item 2
    [Processamento] Processou 2 -> 4

Análise dos Resultados: O log de saída é a principal evidência do sucesso da arquitetura. O intercalar das mensagens confirma que as três threads estavam ativas ao mesmo tempo, cada uma em seu estágio. Isso aumenta a vazão do sistema, pois o tempo total de execução não é a soma dos tempos de cada estágio, mas sim o tempo do estágio mais lento, mais a latência inicial para preencher o pipeline. O programa processa todos os 20 itens sem perda de dados, duplicação ou travamento. Isso prova que o mecanismo de sincronização funcionou corretamente, gerenciando o acesso às filas de forma segura, mesmo sob alta contenção. A ausência de travamentos quando as filas ficam cheias ou vazias demonstra a eficácia do pthread_cond_wait e pthread_cond_signal.

5>

Como Compilar/Rodar: Use o compilador GCC e a flag -pthread: gcc Lista1-5.c -o Lista1-5 -pthread
Execução: ./Lista1-5
O programa pedirá que o usuário digite números na linha de comando. Após digitar todos os números desejados, o usuário deve enviar o sinal de EOF para finalizar a entrada e iniciar o desligamento do sistema.

Decisões de Sincronização: Mutex: Produtor: Usado em fila_push para proteger a estrutura da fila. Consumidor: Usado em fila_pop para proteger a estrutura da fila.

Evidências de Execução: [Thread 2] 100 -> NAO PRIMO
[Thread 0] 13 -> PRIMO
[Thread 3] 7 -> PRIMO
[Thread 1] 25 -> NAO PRIMO
[Thread 2] Encerrando.
[Thread 0] Encerrando.

Análise dos Resultados: Balanceamento de Carga: As mensagens de [Thread X] mostram que as tarefas de verificação de primalidade são distribuídas entre as N_THREADS=4 workers. Isso é a essência da thread pool, onde múltiplas CPUs podem trabalhar em paralelo para reduzir o tempo total de processamento.

6>

Como Compilar/Rodar: Use o compilador GCC e a flag -pthread: gcc Lista1-6.c -o Lista1-6 -pthread Para testar, crie um arquivo (dados.txt) com números inteiros (separados por espaço ou linha), por exemplo, milhares de números aleatórios entre 0 e 1000. Execute o programa passando o nome do arquivo de dados: ./Lista1-6 dados.txt

Decisões de Sincronização: O trabalho em fatias disjuntas garante que não há variáveis compartilhadas sendo modificadas simultaneamente, prevenindo data races. O pthread_join garante que a thread principal só inicie a leitura dos resultados locais (soma_local, hist_local) depois que a worker correspondente tiver terminado de escrevê-los. Isso assegura a correção dos resultados sem a complexidade de locks.

Evidências de Execução: Exemplo de Saída:

Lidos 1000000 inteiros do arquivo.
Soma total = 500500000
Histograma (valores não-nulos):
...
499: 1001
500: 998

Análise dos Resultados: Evidência de Paralelismo: Se o tempo de execução com P=4 for significativamente menor do que o tempo de execução com P=1, o paralelismo está funcionando e a carga de trabalho está sendo distribuída de forma eficaz entre os núcleos da CPU.

Eficiência: Como o código evita locks durante a fase de processamento, ele minimiza a sobrecarga de sincronização, maximizando a eficiência da computação paralela. O gargalo principal será a leitura do arquivo e o tempo de agregação, que é rápido.

7>

Como Compilar/Rodar: Use o compilador GCC e a flag -pthread. A flag -pthread é crucial, pois ela vincula tanto as funções de threads quanto as de semáforo no Linux: gcc Lista1-7.c -o Lista1-7 -pthread e execute com ./Lista1-7 O programa executará as threads até que todas as 100 refeições sejam concluídas e, em seguida, exibirá as estatísticas.

Decisões de Sincronização: Recursos Compartilhados onde ada Mutex representa um garfo. Os filósofos competem por eles usando pthread_mutex_lock. A prevenção de deadlock implementa a solução de restrição de filósofos. Ao usar sem_wait, limita o número de filósofos que podem tentar pegar os garfos a 4, garantindo que sempre haverá recursos suficientes para que pelo menos um filósofo consiga pegar ambos os garfos e comer.

Evidências de Execução: Exemplo de Saída (Usando Semáforo N−1):

=== Estatísticas (Solução 2: Semaforo N-1) ===
Filosofo 0: refeicoes=20, maior espera=0.002s
Filosofo 1: refeicoes=20, maior espera=0.001s

Análise dos Resultados: Os valores de espera são geralmente muito baixos. Isso indica que a solução não gera inanição significativa, pois os filósofos não ficam bloqueados por longos períodos. A latência baixa sugere que a restrição de N−1 não limitou excessivamente a concorrência. Embora 5 filósofos estivessem competindo por 5 garfos, a limitação a 4 concorrentes ativos permitiu um fluxo de trabalho rápido e eficiente.

8>

Como Compilar/Rodar: O código exige a vinculação com a biblioteca de threads POSIX e, por usar a função getopt, deve ser compilado da seguinte forma: gcc Lista1-8.c -o pc_bp -pthread então: ./pc_bp e em seguida: Digite o tamanho do buffer

Decisões de Sincronização: Exclusão Mútua do Buffer, protege o acesso às variáveis de estado do buffer e a lógica de backpressure. O Controle de Vagas/Itens, padrão clássico Produtor-Consumidor. sem_empty bloqueia produtores quando o buffer está cheio. sem_full bloqueia Consumidores quando o buffer está vazio. E o Backpressure, permite que produtores esperem passivamente quando a fila está perigosamente cheia, sendo acordados pelos consumidores quando a fila esvazia o suficiente.

Evidências de Execução: A saída do programa fornece logs detalhados que evidenciam a concorrência e o backpressure como:

[Produtor 0] produziu item 123456 (count=10)
  [Consumidor 2] consumiu item 54321 (Espera: 875 µs) (count=9)

Análise dos Resultados: O uso correto do Mutex com os Semáforos no padrão Produtor-Consumidor e o Buffer Circular garante que as variáveis de estado do buffer nunca sejam corrompidas e que todos os 150 itens sejam processados. E o uso de Poison Pills garante que a thread principal só encerre após todos os itens reais na fila serem processados, permitindo que os consumidores encerrem de forma segura.

9>

Como Compilar/Rodar: O programa foi corrigido para ler o número de corredores K da entrada padrão. A compilação exige a biblioteca de threads POSIX. Use o compilador GCC e a flag -pthread: gcc Lista1-9.c -o revezamento -pthread e excute com: ./Lista1-9

Decisões de Sincronização: A sincronização é baseada no mecanismo de Barreira de Threads para forçar o encontro de todos os participantes antes de iniciar o próximo ciclo de trabalho. Onde todas as threads param em pthread_barrier_wait(). A última thread a chegar libera todas as outras para a próxima rodada. O uso do valor de retorno PTHREAD_BARRIER_SERIAL_THREAD é crucial, pois ele permite que apenas uma thread por ciclo execute a lógica de contagem, garantindo a precisão da métrica global.

Evidências de Execução: A saída final é a principal evidência da funcionalidade do código, provando que o experimento correu pelo tempo definido e que o contador de rodadas é preciso.

Análise dos Resultados: O resultado "Rodadas por minuto" é a métrica chave de throughput do sistema. O Gargalo é a Thread Mais Lenta: O tempo de cada rodada é determinado pelo tempo que a thread mais lenta leva para chegar à barreira. Ao aumentar o valor de K, você aumenta o número de threads que devem "acertar" o intervalo de tempo aleatório. Isso tende a diminuir o throughput porque a probabilidade de ter uma thread mais lenta que 400ms em um grupo de K é maior para K=10 do que para K=2.

10>

Como Compilar/Rodar:Use o compilador GCC e a flag -pthread: gcc Lista1-10.c -o Lista1-10 -pthread

Decisões de Sincronização: A sincronização é usada tanto para causar o deadlock quanto para proteger as métricas de monitoramento. O deadlock ocorre porque as threads violam a regra de ordem de aquisição e como o recurso que cada um espera está preso pelo outro, a execução congela indefinidamente.

Evidências de Execução: A saída do programa fornece a evidência direta de que o deadlock ocorreu e foi detectado. Nenhuma linha de printf de T1 ou T2 é exibida. O log T1 Usando R1 e R2 só é impresso depois que T1 adquire ambos os mutexes. Como as threads travam na tentativa de adquirir o segundo mutex, a função registrar_progresso() nunca é chamada.

Análise dos Resultados: O código atinge seu objetivo principal: confirmando que a aquisição de recursos em ordem inversa entre as threads é uma condição suficiente para causar um deadlock no padrão de Espera Circular. Porém o deadlock ocorre nos primeiros milissegundos, mas a detecção é atrasada artificialmente pelo TIMEOUT de 5 segundos, dando tempo ao watchdog para confirmar a ausência de progresso.
