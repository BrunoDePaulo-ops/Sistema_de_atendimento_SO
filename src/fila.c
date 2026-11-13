#include "../include/fila.h"

Fila* inicializar_fila(void)
{
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

bool fila_vazia(Fila *f)
{
    return (f->inicio == NULL);
}

int fila_tamanho(Fila *f)
{
    return f->tamanho;
}

/* Insere no fim: a fila é FIFO. A prioridade do cliente não muda a ordem de
 * atendimento — no enunciado ela só define a paciência (X/2 para alta, X para baixa). */
void enfileirar(Fila *f, Cliente *c)
{
    No *novo = (No*) malloc(sizeof(No));
    if (!novo) {
        perror("Erro ao alocar nó da fila");
        exit(1);
    }
    novo->cliente = c;
    novo->prox = NULL;

    if (fila_vazia(f))
        f->inicio = novo;
    else
        f->fim->prox = novo;

    f->fim = novo;
    f->tamanho++;
}

/* Consulta o primeiro da fila sem removê-lo. O Atendente atende a partir daqui e
 * só remove depois, como no diagrama. */
Cliente* fila_frente(Fila *f)
{
    return fila_vazia(f) ? NULL : f->inicio->cliente;
}

Cliente* desenfileirar(Fila *f)
{
    if (fila_vazia(f))
        return NULL;

    No *temp = f->inicio;
    Cliente *c = temp->cliente;

    f->inicio = f->inicio->prox;
    if (f->inicio == NULL)
        f->fim = NULL;

    free(temp);
    f->tamanho--;
    return c;
}

void destruir_fila(Fila *f)
{
    if (!f) return;
    while (!fila_vazia(f)) {
        Cliente *c = desenfileirar(f);
        free(c);
    }
    free(f);
}
