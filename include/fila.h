#ifndef FILA_H
#define FILA_H

#include "comum.h"

// Nó da fila encadeada
typedef struct No {
    Cliente *cliente;
    struct No *prox;
} No;

typedef struct {
    No *inicio;
    No *fim;
    int tamanho;
} Fila;

Fila* inicializar_fila();
void enfileirar(Fila *f, Cliente *c);
Cliente* desenfileirar(Fila *f);
bool fila_vazia(Fila *f);
void destruir_fila(Fila *f);

#endif