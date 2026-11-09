#ifndef COMUM_H
#define COMUM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <time.h>
#include <ucontext.h>
#include <sys/time.h>

#define ARQUIVO_PIDS "pids_atendidos.txt"

// Tipos de prioridade
#define PRIORIDADE_ALTA 1
#define PRIORIDADE_BAIXA 2

typedef struct {
    int id;                 // PID ou identificador único do cliente
    int prioridade;         // 1 = Alta (paciência X/2), 2 = Baixa (paciência X)
    long long tempo_chegada;// Momento em que entrou na fila (em ms ou ticks)
    long long paciencia_max;// Tempo máximo de paciência (X ou X/2)
} Cliente;

static inline long long obter_tempo_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000LL) + (tv.tv_usec / 1000LL);
}

#endif