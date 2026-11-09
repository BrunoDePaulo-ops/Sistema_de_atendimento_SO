#ifndef RECEPCAO_H
#define RECEPCAO_H

#include "comum.h"
#include "fila.h"

void configurar_recepcao(Fila *f_compartilhada, int n_clientes, int x_paciencia);
void funcao_recepcao(void);

#endif