#ifndef ATENDENTE_H
#define ATENDENTE_H

#include "comum.h"
#include "fila.h"

/*
 * Thread 1 — "Atendente" (diagrama do enunciado). Para cada cliente:
 *   1. lê na fila o próximo cliente;
 *   2. acorda o cliente (SIGCONT — ele estava parado em raise(SIGSTOP));
 *   3. espera o atendimento terminar (o cliente fecha e reabre "/sem_atend");
 *   4. fecha "/sem_block", escreve o PID do cliente no arquivo LNG, abre "/sem_block";
 *   5. calcula a satisfação do cliente;
 *   6. tira o cliente da fila;
 *   7. acorda o Analista (SIGUSR1).
 */
void configurar_atendente(Fila *f_compartilhada, int n_clientes, int x_paciencia, pid_t pid_analista);
void funcao_atendente(void);

/* Resultados para o relatório final do processo Atendimento. */
void obter_estatisticas(double *taxa_satisfacao, long long *tempo_total,
                        int *atendidos, int *satisfeitos);

#endif
