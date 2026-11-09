#include "../include/fila.h"

Fila* inicializar_fila() {
    Fila *f = (Fila*) malloc(sizeof(Fila));
    if (!f) {
        perror("Erro ao alocar fila");
        exit(1);
    }
    f->inicio = NULL;
    f->fim = NULL;
    f->tamanho = 0;
    return f;
}

bool fila_vazia(Fila *f) {
    return (f->inicio == NULL);
}

void enfileirar(Fila *f, Cliente *c) {
    No *novo = (No*) malloc(sizeof(No));
    if (!novo) {
        perror("Erro ao alocar nó da fila");
        exit(1);
    }
    novo->cliente = c;
    novo->prox = NULL;

    // Se quiser dar preferência real para alta prioridade na entrada:
    // (Opcional, mas garante que alta prioridade fure a fila de baixa)
    if (c->prioridade == PRIORIDADE_ALTA && f->inicio != NULL) {
        // Insere logo após o início ou trata prioridade aqui
    }

    if (fila_vazia(f)) {
        f->inicio = novo;
    } else {
        f->fim->prox = novo;
    }
    f->fim = novo;
    f->tamanho++;
}

Cliente* desenfileirar(Fila *f) {
    if (fila_vazia(f)) {
        return NULL;
    }

    No *temp = f->inicio;
    Cliente *c = temp->cliente;

    f->inicio = f->inicio->prox;
    if (f->inicio == NULL) {
        f->fim = NULL;
    }

    free(temp);
    f->tamanho--;
    return c;
}

void destruir_fila(Fila *f) {
    while (!fila_vazia(f)) {
        Cliente *c = desenfileirar(f);
        free(c);
    }
    free(f);
}