#include "../include/comum.h"
#include "../include/cliente.h"
#include "../include/fila.h"
#include "../include/scheduler.h"
#include "../include/recepcao.h"
#include "../include/atendente.h"
#include "../include/analista.h"
#include <signal.h>

int main(int argc, char *argv[]) {
    srand(42);
    if (argc < 3) {
        printf("Uso correto: %s <N_clientes> <X_paciencia_ms>\n", argv[0]);
        printf("Exemplo (finito): %s 50 1000\n", argv[0]);
        printf("Exemplo (infinito): %s 0 1000\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int X = atoi(argv[2]);

    srand(time(NULL));

    // Inicializa / limpa o arquivo de PIDs
    FILE *f = fopen(ARQUIVO_PIDS, "w");
    if (f) fclose(f);

    // Cria o processo Analista via fork()
    pid_t pid_analista = fork();
    if (pid_analista < 0) {
        perror("Erro ao criar processo analista com fork");
        return 1;
    }

    if (pid_analista == 0) {
        // Código executado pelo Processo Filho (Analista)
        iniciar_processo_analista();
        exit(0);
    }

    // Código executado pelo Processo Pai (Atendimento)
    Fila *fila = inicializar_fila();
    configurar_recepcao(fila, N, X);
    configurar_atendente(fila, N, X, pid_analista);

    // Inicializa o escalonador próprio por troca de contexto
    inicializar_escalonador();
    criar_thread(funcao_recepcao, 1);
    criar_thread(funcao_atendente, 2);

    printf("=== Processo de Atendimento Iniciado (PID: %d) ===\n", getpid());
    printf("Parametros: N = %d, X = %d ms\n", N, X);

    if (N > 0) {
        // Dispara o escalonador para rodar até esgotar os N clientes
        escalonar();
    } else {
        printf("Modo infinito ativado (N = 0).\n");
        printf("Pressione a tecla 's' e Enter para encerrar a simulacao...\n");
        
        // Dispara o escalonador em background ou gerencia a parada com a tecla 's'
        // Dica: Para o modo infinito, você pode rodar o escalonador e monitorar a entrada de teclado
        escalonar(); 
        
        char op = getchar();
        if (op == 's' || op == 'S') {
            printf("\nTecla 's' pressionada. Encerrando atendimento...\n");
            // Se restou algo no arquivo, acorda o analista para esvaziar tudo conforme o enunciado
            kill(pid_analista, SIGUSR1);
            usleep(100000); // pequeno delay para o analista processar
        }
    }

    // Obtém estatísticas finais do atendente
    double taxa_satisfacao = 0.0;
    long long tempo_total = 0;
    obter_estatisticas(&taxa_satisfacao, &tempo_total);

    printf("\n=== Relatorio Final de Atendimento ===\n");
    printf("Taxa de Satisfacao: %.2f%%\n", taxa_satisfacao * 100.0);
    printf("Tempo Total de Execucao: %lld ms\n", tempo_total);
    printf("======================================\n");

    // Limpa o processo analista e recursos
    kill(pid_analista, SIGKILL);
    destruir_fila(fila);

    return 0;
}