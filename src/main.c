/*
 * Processo 1 — "Atendimento".
 *
 * Cria o processo Analista (fork), monta a fila e põe o escalonador cooperativo para
 * rodar as threads de usuário Recepção e Atendente (e, no modo infinito, o monitor de
 * teclado). Quando as threads terminam, o escalonador devolve o controle para cá, que
 * é onde o arquivo LNG é esvaziado, o relatório final é impresso e tudo é liberado.
 *
 * Uso:  ./sistema_atendimento <N_clientes> <X_paciencia_ms>
 *       N = 0 significa infinito: o usuário encerra apertando "s".
 */

#include "../include/comum.h"
#include "../include/cliente.h"
#include "../include/fila.h"
#include "../include/scheduler.h"
#include "../include/recepcao.h"
#include "../include/atendente.h"
#include "../include/analista.h"
#include "../include/monitor.h"

long long g_inicio_programa = 0;

/* Clientes que ficaram na fila quando o usuário apertou "s": os processos estão
 * parados em raise(SIGSTOP) e precisam ser encerrados para não virarem órfãos.
 * O enunciado manda ignorá-los no cálculo da taxa de satisfação. */
static void descartar_clientes_na_fila(Fila *fila)
{
    int descartados = 0;
    while (!fila_vazia(fila)) {
        Cliente *c = desenfileirar(fila);
        kill(c->pid, SIGKILL);
        waitpid(c->pid, NULL, 0);
        free(c);
        descartados++;
    }
    if (descartados > 0)
        printf("[Atendimento] %d cliente(s) ainda na fila foram descartados do calculo.\n",
               descartados);
}

static long tamanho_lng(void)
{
    struct stat st;
    return (stat(ARQUIVO_LNG, &st) == 0) ? (long) st.st_size : 0L;
}

/* Acorda o Analista quantas vezes forem necessárias para ele imprimir todos os PIDs
 * que ainda estiverem no arquivo, e só então o encerra (SIGTERM, que ele trata
 * esvaziando o que restar). É a exigência marcada como IMPORTANTE no enunciado. */
static void drenar_e_encerrar_analista(pid_t pid_analista)
{
    for (int tentativas = 0; tentativas < 500 && tamanho_lng() > 0; tentativas++) {
        kill(pid_analista, SIGUSR1);
        usleep(10000);                       /* 10 ms para o Analista processar o lote */
    }
    kill(pid_analista, SIGTERM);
    waitpid(pid_analista, NULL, 0);
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Uso correto: %s <N_clientes> <X_paciencia_ms>\n", argv[0]);
        printf("Exemplo (finito):  %s 50 1000\n", argv[0]);
        printf("Exemplo (infinito): %s 0 1000   (encerra apertando 's')\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int X = atoi(argv[2]);

    if (N < 0 || X <= 0) {
        fprintf(stderr, "Parametros invalidos: N deve ser >= 0 e X deve ser > 0.\n");
        return 1;
    }

    g_inicio_programa = obter_tempo_ms();
    srand((unsigned) time(NULL) ^ (unsigned) getpid());

    Fila *fila = inicializar_fila();

    /* Cria os semáforos e limpa os arquivos ANTES do fork, para o Analista já nascer
     * com "/sem_block" disponível. */
    configurar_recepcao(fila, N, X);

    pid_t pid_analista = fork();
    if (pid_analista < 0) {
        perror("Erro ao criar o processo Analista com fork");
        destruir_fila(fila);
        recepcao_fechar_semaforos();
        return 1;
    }

    if (pid_analista == 0) {
        /* Processo filho: Analista. */
        iniciar_processo_analista();
        exit(0);
    }

    configurar_atendente(fila, N, X, pid_analista);

    inicializar_escalonador();
    criar_thread(funcao_recepcao, 1);
    criar_thread(funcao_atendente, 2);

    printf("=== Processo de Atendimento iniciado (PID: %d) ===\n", getpid());
    printf("Parametros: N = %d, X = %d ms\n", N, X);

    if (N == 0) {
        printf("Modo infinito: aperte 's' para encerrar.\n");
        monitor_configurar();
        criar_thread(funcao_monitor_teclado, 3);
    }
    fflush(stdout);

    /* Roda as threads de usuário. Volta para cá quando todas terminam. */
    escalonar();

    monitor_restaurar();
    descartar_clientes_na_fila(fila);
    drenar_e_encerrar_analista(pid_analista);

    double taxa_satisfacao = 0.0;
    long long tempo_total = 0;
    int atendidos = 0, satisfeitos = 0;
    obter_estatisticas(&taxa_satisfacao, &tempo_total, &atendidos, &satisfeitos);

    printf("\n=== Relatorio Final de Atendimento ===\n");
    printf("Clientes atendidos: %d (satisfeitos: %d)\n", atendidos, satisfeitos);
    printf("Taxa de Satisfacao: %.2f%%\n", taxa_satisfacao * 100.0);
    printf("Tempo Total de Execucao: %lld ms\n", tempo_total);
    printf("======================================\n");
    fflush(stdout);

    destruir_fila(fila);
    recepcao_fechar_semaforos();
    remove(ARQUIVO_DEMANDA);

    return houve_erro() ? 1 : 0;
}
