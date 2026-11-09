#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "comum.h"

// Estados possíveis de uma thread
typedef enum {
    PRONTA,
    EXECUTANDO,
    TERMINADA
} ThreadState;

// Bloco de Controle de Thread (TCB)
typedef struct {
    int id;
    ucontext_t contexto;
    ThreadState estado;
    void *pilha;
} TCB;

void inicializar_escalonador();
void criar_thread(void (*func)(void), int id_thread);
void escalonar();
void yield_thread(); // Para ceder o processamento para outra thread

#endif