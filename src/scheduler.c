#include "../include/scheduler.h"

#define TAM_PILHA (1024 * 64) // 64 KB de pilha para cada thread
#define MAX_THREADS 2

static TCB threads[MAX_THREADS];
static int thread_atual = 0;
static int total_threads = 0;

void inicializar_escalonador() {
    total_threads = 0;
    thread_atual = 0;
}

void criar_thread(void (*func)(void), int id_thread) {
    if (total_threads >= MAX_THREADS) return;

    TCB *t = &threads[total_threads];
    t->id = id_thread;
    t->estado = PRONTA;

    t->pilha = malloc(TAM_PILHA);
    if (!t->pilha) {
        perror("Erro ao alocar pilha da thread");
        exit(1);
    }

    getcontext(&(t->contexto));
    // Se a thread retornar, chama um manipulador de saída em vez de falhar
    t->contexto.uc_link = NULL; 
    t->contexto.uc_stack.ss_sp = t->pilha;
    t->contexto.uc_stack.ss_size = TAM_PILHA;
    t->contexto.uc_stack.ss_flags = 0;

    makecontext(&(t->contexto), func, 0);

    total_threads++;
}

void yield_thread() {
    int proxima = (thread_atual + 1) % total_threads;
    int anterior = thread_atual;
    thread_atual = proxima;

    // Troca o contexto atual pela próxima thread
    swapcontext(&(threads[anterior].contexto), &(threads[proxima].contexto));
}

void escalonar() {
    // Inicia a execução disparando a primeira thread
    if (total_threads > 0) {
        threads[thread_atual].estado = EXECUTANDO;
        setcontext(&(threads[thread_atual].contexto));
    }
}