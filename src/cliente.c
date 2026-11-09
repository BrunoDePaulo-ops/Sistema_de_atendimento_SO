#include "../include/cliente.h"

Cliente* criar_cliente(int id_seq, int X) {
    Cliente *c = (Cliente*) malloc(sizeof(Cliente));
    if (!c) {
        perror("Erro ao alocar memória para cliente");
        exit(1);
    }

    c->id = id_seq;

    // Sorteia a prioridade: 50% de chance para alta ou baixa
    // rand() % 2 gera 0 ou 1
    if (rand() % 2 == 0) {
        c->prioridade = PRIORIDADE_ALTA;
        c->paciencia_max = X / 2;
    } else {
        c->prioridade = PRIORIDADE_BAIXA;
        c->paciencia_max = X;
    }

    // Opcional: registrar o tempo de chegada atual (pode usar clock() ou gettimeofday)
    c->tempo_chegada = 0; // Ajustaremos o relógio quando integrarmos com a simulação

    return c;
}