#ifndef COMUM_H
#define COMUM_H

/*
 * comum.h — tipos e constantes compartilhados por todos os módulos.
 *
 * Mapa do sistema (ver diagrama do enunciado):
 *   Processo 1 "Atendimento" ....... este programa (duas threads de usuário)
 *     Thread "Recepção" ............ cria os processos cliente e enfileira
 *     Thread "Atendente" ........... atende a fila e registra no arquivo LNG
 *   Processo 2 "Analista" .......... filho criado por fork(), imprime o LNG
 *   Processo 3 "Cliente" ........... binário ./cliente (código dado pelo professor)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>
#include <ucontext.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>

/* Arquivo "Lista de Números Gerados": o Atendente escreve o PID de cada cliente
 * atendido e o Analista imprime e apaga o que já imprimiu. Protegido por SEM_BLOCK. */
#define ARQUIVO_LNG "lng.txt"

/* Arquivo "Demanda": o processo cliente escreve nele o tempo de que precisa para ser
 * atendido. É único e serve a todos os clientes porque a Recepção cria e lê um cliente
 * por vez (nota de rodapé 7 do enunciado). O nome é fixado pelo cliente.c do professor. */
#define ARQUIVO_DEMANDA "demanda.txt"

/* O Analista "gera um arquivo com seu PID" assim que nasce (diagrama, Processo 2). */
#define ARQUIVO_PID_ANALISTA "analista.pid"

/* Semáforos nomeados POSIX criados pela Recepção (diagrama, Thread 2).
 * SEM_ATEND: o cliente fecha enquanto está sendo atendido e abre ao terminar.
 * SEM_BLOCK: exclusão mútua do arquivo LNG entre o Atendente e o Analista. */
#define SEM_ATEND "/sem_atend"
#define SEM_BLOCK "/sem_block"

/* Nota de rodapé 9: no modo infinito a Recepção pausa a criação quando a fila
 * chega a 100 clientes em espera e volta a criar quando libera espaço. */
#define LIMITE_FILA 100

/* O Analista imprime no máximo 10 valores por vez que acorda. */
#define LOTE_ANALISTA 10

#define MAX_LINHA 64

/* Tipos de prioridade. O enunciado usa a prioridade só para definir a paciência:
 * alta = X/2 ms, baixa = X ms. O sorteio é de 50% para cada. */
#define PRIORIDADE_ALTA 1
#define PRIORIDADE_BAIXA 2

/* Estrutura do cliente na fila (exatamente os quatro campos do diagrama). */
typedef struct {
    pid_t pid;                  /* PID do processo cliente (Processo 3)            */
    int prioridade;             /* 1 = alta (paciência X/2), 2 = baixa (paciência X)*/
    long long tempo_chegada;    /* ms desde o início do programa até a criação      */
    long long paciencia_max;    /* X/2 ou X, em ms                                  */
    int tempo_atendimento;      /* microssegundos lidos do arquivo Demanda          */
} Cliente;

/* Relógio em milissegundos (tempo absoluto). */
static inline long long obter_tempo_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000LL) + (tv.tv_usec / 1000LL);
}

/* Instante em que o programa começou; definido em main.c. */
extern long long g_inicio_programa;

/* Milissegundos passados desde o início do programa. É a base da "hora de chegada"
 * (nota de rodapé 4) e do cálculo de satisfação (nota de rodapé 6). */
static inline long long tempo_desde_inicio(void)
{
    return obter_tempo_ms() - g_inicio_programa;
}

#endif
