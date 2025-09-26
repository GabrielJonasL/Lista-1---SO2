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

Como Compilar/Rodar:

Decisões de Sincronização:

Evidências de Execução:

Análise dos Resultados:

5>

Como Compilar/Rodar:

Decisões de Sincronização:

Evidências de Execução:

Análise dos Resultados:

6>

Como Compilar/Rodar:

Decisões de Sincronização:

Evidências de Execução:

Análise dos Resultados:

7>

Como Compilar/Rodar:

Decisões de Sincronização:

Evidências de Execução:

Análise dos Resultados:

8>

Como Compilar/Rodar:

Decisões de Sincronização:

Evidências de Execução:

Análise dos Resultados:

9>

Como Compilar/Rodar:

Decisões de Sincronização:

Evidências de Execução:

Análise dos Resultados:

10>

Como Compilar/Rodar:Use o compilador GCC e a flag -pthread: gcc Lista1-10.c -o Lista1-10 -pthread

Decisões de Sincronização: A sincronização é usada tanto para causar o deadlock quanto para proteger as métricas de monitoramento. O deadlock ocorre porque as threads violam a regra de ordem de aquisição e como o recurso que cada um espera está preso pelo outro, a execução congela indefinidamente.

Evidências de Execução: A saída do programa fornece a evidência direta de que o deadlock ocorreu e foi detectado. Nenhuma linha de printf de T1 ou T2 é exibida. O log T1 Usando R1 e R2 só é impresso depois que T1 adquire ambos os mutexes. Como as threads travam na tentativa de adquirir o segundo mutex, a função registrar_progresso() nunca é chamada.

Análise dos Resultados: O código atinge seu objetivo principal: confirmando que a aquisição de recursos em ordem inversa entre as threads é uma condição suficiente para causar um deadlock no padrão de Espera Circular. Porém o deadlock ocorre nos primeiros milissegundos, mas a detecção é atrasada artificialmente pelo TIMEOUT de 5 segundos, dando tempo ao watchdog para confirmar a ausência de progresso.
