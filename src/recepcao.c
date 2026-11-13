#include "../include/recepcao.h"
#include "../include/cliente.h"
#include "../include/scheduler.h"
#include "../include/monitor.h"

static Fila *fila_global = NULL;
static int N_total = 0;
static int X_paciencia = 0;
static int clientes_gerados = 0;

static sem_t *sem_atend = SEM_FAILED;   /* fechado pelo cliente durante o atendimento */
static sem_t *sem_block = SEM_FAILED;   /* exclusão mútua do arquivo LNG              */

void configurar_recepcao(Fila *f_compartilhada, int n_clientes, int x_paciencia)
{
    fila_global = f_compartilhada;
    N_total = n_clientes;
    X_paciencia = x_paciencia;
    clientes_gerados = 0;

    /* Limpa resíduos de uma execução anterior. */
    remove(ARQUIVO_DEMANDA);
    FILE *lng = fopen(ARQUIVO_LNG, "w");
    if (lng) fclose(lng);

    /* Semáforos nomeados POSIX. sem_unlink antes porque um semáforo nomeado sobrevive
     * ao processo: se a execução anterior morreu no meio, ele poderia estar fechado. */
    sem_unlink(SEM_ATEND);
    sem_unlink(SEM_BLOCK);
    sem_atend = sem_open(SEM_ATEND, O_CREAT | O_EXCL, 0644, 1);
    sem_block = sem_open(SEM_BLOCK, O_CREAT | O_EXCL, 0644, 1);

    if (sem_atend == SEM_FAILED || sem_block == SEM_FAILED) {
        perror("Erro ao criar os semaforos (/sem_atend, /sem_block)");
        exit(1);
    }
}

sem_t* recepcao_sem_block(void)
{
    return sem_block;
}

void recepcao_fechar_semaforos(void)
{
    if (sem_atend != SEM_FAILED) { sem_close(sem_atend); sem_atend = SEM_FAILED; }
    if (sem_block != SEM_FAILED) { sem_close(sem_block); sem_block = SEM_FAILED; }
    sem_unlink(SEM_ATEND);
    sem_unlink(SEM_BLOCK);
}

void funcao_recepcao(void)
{
    int falhas_seguidas = 0;

    while (!encerramento_pedido()) {

        /* Modo finito: para depois de criar os N clientes. */
        if (N_total > 0 && clientes_gerados >= N_total)
            break;

        /* Nota de rodapé 9: pausa a criação enquanto a fila estiver cheia e volta a
         * criar assim que o Atendente liberar espaço. */
        while (fila_tamanho(fila_global) >= LIMITE_FILA && !encerramento_pedido())
            yield_thread();

        if (encerramento_pedido())
            break;

        Cliente *c = criar_cliente(X_paciencia);
        if (!c) {
            if (++falhas_seguidas >= 3) {
                fprintf(stderr, "Recepcao: nao consegui criar o processo cliente. "
                                "O binario ./cliente foi compilado (make)? Encerrando.\n");
                pedir_encerramento_por_erro();
                break;
            }
            yield_thread();
            continue;
        }
        falhas_seguidas = 0;

        enfileirar(fila_global, c);
        clientes_gerados++;

        /* Cede a vez ao Atendente: a fila já tem gente para atender. */
        yield_thread();
    }
}
