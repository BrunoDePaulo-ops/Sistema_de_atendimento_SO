#include "../include/scheduler.h"

#define TAM_PILHA (1024 * 64)   /* 64 KB de pilha por thread de usuário */
#define MAX_THREADS 4

static TCB threads[MAX_THREADS];
static int thread_atual = 0;
static int total_threads = 0;

/* Contexto do escalonador. Toda troca passa por aqui, o que mantém o controle
 * centralizado e permite voltar ao main quando não há mais thread viva. */
static ucontext_t ctx_escalonador;

void inicializar_escalonador(void)
{
    total_threads = 0;
    thread_atual = 0;
}

/* Trampolim: executa o corpo da thread e, quando ele retorna, marca a thread como
 * terminada e devolve o processador ao escalonador. Sem isso, o retorno da função
 * cairia no uc_link (NULL) e encerraria o processo inteiro. */
static void trampolim(void)
{
    TCB *t = &threads[thread_atual];
    t->func();
    t->estado = TERMINADA;
    swapcontext(&(t->contexto), &ctx_escalonador);
}

void criar_thread(void (*func)(void), int id_thread)
{
    if (total_threads >= MAX_THREADS) {
        fprintf(stderr, "Escalonador: limite de %d threads atingido.\n", MAX_THREADS);
        return;
    }

    TCB *t = &threads[total_threads];
    t->id = id_thread;
    t->estado = PRONTA;
    t->func = func;

    t->pilha = malloc(TAM_PILHA);
    if (!t->pilha) {
        perror("Erro ao alocar pilha da thread");
        exit(1);
    }

    getcontext(&(t->contexto));
    t->contexto.uc_link = &ctx_escalonador;   /* rede de segurança */
    t->contexto.uc_stack.ss_sp = t->pilha;
    t->contexto.uc_stack.ss_size = TAM_PILHA;
    t->contexto.uc_stack.ss_flags = 0;

    makecontext(&(t->contexto), trampolim, 0);

    total_threads++;
}

void yield_thread(void)
{
    TCB *t = &threads[thread_atual];
    if (t->estado == EXECUTANDO)
        t->estado = PRONTA;
    swapcontext(&(t->contexto), &ctx_escalonador);
}

void escalonar(void)
{
    int proxima_busca = 0;

    while (1) {
        /* Round-robin: procura a próxima thread que ainda não terminou. */
        int escolhida = -1;
        for (int k = 0; k < total_threads; k++) {
            int idx = (proxima_busca + k) % total_threads;
            if (threads[idx].estado != TERMINADA) {
                escolhida = idx;
                break;
            }
        }
        if (escolhida < 0)
            break;                  /* nenhuma thread viva: volta para o main */

        thread_atual = escolhida;
        threads[escolhida].estado = EXECUTANDO;
        swapcontext(&ctx_escalonador, &(threads[escolhida].contexto));
        proxima_busca = (escolhida + 1) % total_threads;
    }

    /* Libera as pilhas das threads encerradas. */
    for (int i = 0; i < total_threads; i++) {
        free(threads[i].pilha);
        threads[i].pilha = NULL;
    }
}
