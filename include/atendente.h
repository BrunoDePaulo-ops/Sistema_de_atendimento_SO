#ifndef ATENDENTE_H
#define ATENDENTE_H

#include "comum.h"
#include "fila.h"

void configurar_atendente(Fila *f_compartilhada, int n_clientes, int x_paciencia, pid_t pid_analista);
void funcao_atendente(void);
void obter_estatisticas(double *taxa_satisfacao, long long *tempo_total);

#endif