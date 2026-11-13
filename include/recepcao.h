#ifndef RECEPCAO_H
#define RECEPCAO_H

#include "comum.h"
#include "fila.h"

/*
 * Thread 2 — "Recepção" (diagrama do enunciado):
 *   - cria os semáforos "/sem_atend" e "/sem_block";
 *   - cria N processos cliente (ou infinitos, quando N = 0);
 *   - cada cliente nasce com prioridade sorteada, 50% para cada uma das duas;
 *   - insere cada cliente na fila de atendimento;
 *   - no modo infinito, pausa a criação quando a fila chega a LIMITE_FILA clientes.
 *
 * Os semáforos são criados em configurar_recepcao(), que o main chama antes do fork
 * do Analista — assim o Analista já encontra "/sem_block" pronto para usar.
 */
void   configurar_recepcao(Fila *f_compartilhada, int n_clientes, int x_paciencia);
void   funcao_recepcao(void);
sem_t* recepcao_sem_block(void);        /* semáforo que protege o arquivo LNG */
void   recepcao_fechar_semaforos(void); /* fecha e remove os semáforos nomeados */

#endif
