#ifndef FILA_H
#define FILA_H

#include "comum.h"

/*
 * Fila de clientes: lista simplesmente encadeada com ponteiro para o fim, para que
 * enfileirar e desenfileirar custem O(1). O Atendente primeiro consulta a frente
 * (fila_frente), atende, e só então remove (desenfileirar) — é a ordem do diagrama,
 * em que "tira cliente da fila" vem depois do atendimento.
 */

typedef struct No {
    Cliente *cliente;
    struct No *prox;
} No;

typedef struct {
    No *inicio;
    No *fim;
    int tamanho;
} Fila;

Fila*    inicializar_fila(void);
void     enfileirar(Fila *f, Cliente *c);
Cliente* fila_frente(Fila *f);      /* consulta sem remover; NULL se vazia */
Cliente* desenfileirar(Fila *f);    /* remove e devolve; NULL se vazia     */
bool     fila_vazia(Fila *f);
int      fila_tamanho(Fila *f);
void     destruir_fila(Fila *f);    /* libera nós e clientes restantes     */

#endif
