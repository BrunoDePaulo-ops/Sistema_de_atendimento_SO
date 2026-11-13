#include "../include/cliente.h"
#include "../include/scheduler.h"

/* Lê o tempo de atendimento que o processo cliente acabou de escrever no arquivo
 * Demanda e apaga o arquivo (nota de rodapé 7: "após lido, pode ser apagado").
 * Em caso de falha devolve 1 microssegundo, o menor valor que o cliente usa. */
static int ler_demanda(void)
{
    int tempo = 1;
    FILE *f = fopen(ARQUIVO_DEMANDA, "r");
    if (f) {
        if (fscanf(f, "%d", &tempo) != 1)
            tempo = 1;
        fclose(f);
        remove(ARQUIVO_DEMANDA);
    }
    return tempo;
}

Cliente* criar_cliente(int X)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("Erro no fork do processo cliente");
        return NULL;
    }

    if (pid == 0) {
        /* Processo filho vira o cliente dado pelo professor. */
        execl("./cliente", "cliente", (char *) NULL);
        perror("Erro ao executar ./cliente");
        _exit(127);
    }

    /* A Recepção espera o cliente escrever a demanda e parar sozinho (SIGSTOP).
     * WNOHANG + yield_thread: a espera não pode bloquear o processo inteiro, senão
     * o Atendente pararia junto (o escalonador é cooperativo). */
    int status;
    while (1) {
        pid_t r = waitpid(pid, &status, WUNTRACED | WNOHANG);
        if (r == pid) {
            if (WIFSTOPPED(status))
                break;                      /* pronto: dormindo, esperando atendimento */
            return NULL;                    /* morreu antes de parar (exec falhou)      */
        }
        if (r < 0) {
            perror("waitpid do cliente");
            return NULL;
        }
        yield_thread();
    }

    Cliente *c = (Cliente *) malloc(sizeof(Cliente));
    if (!c) {
        perror("Erro ao alocar memória para cliente");
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return NULL;
    }

    c->pid = pid;

    /* Prioridade aleatória, 50% para cada uma das duas possíveis. */
    if (rand() % 2 == 0) {
        c->prioridade = PRIORIDADE_ALTA;
        c->paciencia_max = X / 2;           /* alta: metade da paciência */
    } else {
        c->prioridade = PRIORIDADE_BAIXA;
        c->paciencia_max = X;
    }

    c->tempo_atendimento = ler_demanda();
    c->tempo_chegada = tempo_desde_inicio();  /* ms entre o início do programa e a criação */

    return c;
}
