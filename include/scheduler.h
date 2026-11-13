#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "comum.h"

/*
 * Escalonador cooperativo de threads de usuário, feito com ucontext (getcontext,
 * makecontext, swapcontext). É a exigência central do enunciado: "as threads devem
 * ser criadas usando troca de contexto, isto é, você deve implementar o escalonador".
 * Nenhuma biblioteca de paralelismo é usada.
 *
 * Desenho: round-robin cooperativo passando sempre pelo contexto do escalonador.
 *   escalonar()     roda no contexto do main, escolhe a próxima thread PRONTA e
 *                   entra nela com swapcontext;
 *   yield_thread()  devolve o processador ao escalonador;
 *   quando a função da thread retorna, um trampolim marca o TCB como TERMINADA e
 *                   volta ao escalonador — por isso escalonar() retorna ao main
 *                   quando todas as threads acabam (é o que permite o relatório
 *                   final e a limpeza acontecerem de verdade).
 */

typedef enum {
    PRONTA,
    EXECUTANDO,
    TERMINADA
} ThreadState;

/* Bloco de Controle de Thread */
typedef struct {
    int id;
    ucontext_t contexto;
    ThreadState estado;
    void *pilha;
    void (*func)(void);
} TCB;

void inicializar_escalonador(void);
void criar_thread(void (*func)(void), int id_thread);
void escalonar(void);        /* roda até todas as threads terminarem */
void yield_thread(void);     /* cede o processador para a próxima thread */

#endif
