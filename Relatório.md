# Lista-1---SO2
Repositório contendo questões da primeira lista da matéria de Sistemas Operacionasi II

1>

Como Compilar/Rodar: Para compilar este programa, é necessário vincular a biblioteca de threads POSIX. No Linux, use o seguinte comando no terminal: gcc -o Lista 1-1 Lista 1-1.c -pthread
Após a compilação, execute o programa com:./Lista 1-1

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

Como Compilar/Rodar:

Decisões de Sincronização:

Evidências de Execução:

Análise dos Resultados:

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

Como Compilar/Rodar:

Decisões de Sincronização:

Evidências de Execução:

Análise dos Resultados:
