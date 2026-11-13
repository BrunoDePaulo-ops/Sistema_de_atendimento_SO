#include "../include/atendente.h"
#include "../include/recepcao.h"
#include "../include/scheduler.h"
#include "../include/monitor.h"

static Fila *fila_global = NULL;
static int N_total = 0;
static pid_t analista_pid = 0;

static int total_atendidos = 0;
static int total_satisfeitos = 0;
static long long tempo_fim = 0;

void configurar_atendente(Fila *f_compartilhada, int n_clientes, int x_paciencia, pid_t pid_analista)
{
    (void) x_paciencia;          /* a paciência já vem calculada dentro de cada cliente */
    fila_global = f_compartilhada;
    N_total = n_clientes;
    analista_pid = pid_analista;
    total_atendidos = 0;
    total_satisfeitos = 0;
    tempo_fim = 0;
}

/*
 * Espera o atendimento acabar. O cliente, depois do SIGCONT, fecha "/sem_atend",
 * dorme o tempo que ele mesmo calculou e abre o semáforo antes de terminar — ou seja,
 * o fim do processo cliente é exatamente o instante em que "/sem_atend" volta a abrir.
 * Esperar pelo fim do processo é equivalente e não tem corrida: se o Atendente também
 * disputasse o semáforo, poderia tomar a vez do próprio cliente.
 *
 * WNOHANG + yield_thread porque o escalonador é cooperativo: uma espera bloqueante
 * aqui pararia também a thread Recepção.
 */
static void esperar_fim_do_atendimento(pid_t pid)
{
    int status;
    while (1) {
        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r == pid && (WIFEXITED(status) || WIFSIGNALED(status)))
            return;
        if (r < 0)
            return;              /* já foi coletado ou não existe mais */
        yield_thread();
    }
}

/*
 * Escreve o PID do cliente atendido no arquivo LNG, com o arquivo bloqueado pelo
 * semáforo "/sem_block" — é o que impede o Analista de ler e reescrever o arquivo
 * no meio da escrita (perdendo registros).
 * sem_trywait + yield pelo mesmo motivo do waitpid acima.
 */
static void registrar_no_lng(pid_t pid)
{
    sem_t *sb = recepcao_sem_block();

    if (sb != SEM_FAILED) {
        while (sem_trywait(sb) != 0) {
            if (errno != EAGAIN) {
                perror("sem_trywait(/sem_block)");
                return;
            }
            yield_thread();
        }
    }

    FILE *f = fopen(ARQUIVO_LNG, "a");
    if (f) {
        fprintf(f, "%d\n", (int) pid);
        fclose(f);
    } else {
        perror("Erro ao abrir o arquivo LNG");
    }

    if (sb != SEM_FAILED)
        sem_post(sb);
}

void funcao_atendente(void)
{
    while (!encerramento_pedido()) {

        /* Modo finito: termina quando todos os N clientes foram atendidos. */
        if (N_total > 0 && total_atendidos >= N_total)
            break;

        Cliente *c = fila_frente(fila_global);      /* 1. lê na fila o próximo cliente */
        if (!c) {
            yield_thread();                         /* fila vazia: cede para a Recepção */
            continue;
        }

        kill(c->pid, SIGCONT);                      /* 2. acorda o cliente              */
        esperar_fim_do_atendimento(c->pid);         /* 3. espera o atendimento acabar   */
        registrar_no_lng(c->pid);                   /* 4. grava o PID no LNG protegido  */

        /* 5. satisfação: atendimento completado dentro da paciência (nota 6). */
        long long agora = tempo_desde_inicio();
        if ((agora - c->tempo_chegada) <= c->paciencia_max)
            total_satisfeitos++;
        total_atendidos++;

        desenfileirar(fila_global);                 /* 6. tira o cliente da fila        */
        free(c);

        if (analista_pid > 0)
            kill(analista_pid, SIGUSR1);            /* 7. acorda o Analista             */

        yield_thread();
    }

    tempo_fim = tempo_desde_inicio();

    /* Avisa as outras threads (Recepção e monitor de teclado) que o atendimento acabou. */
    pedir_encerramento();
}

void obter_estatisticas(double *taxa_satisfacao, long long *tempo_total,
                        int *atendidos, int *satisfeitos)
{
    if (tempo_fim == 0)
        tempo_fim = tempo_desde_inicio();

    *taxa_satisfacao = (total_atendidos > 0)
                     ? (double) total_satisfeitos / total_atendidos
                     : 0.0;
    *tempo_total = tempo_fim;
    *atendidos = total_atendidos;
    *satisfeitos = total_satisfeitos;
}
