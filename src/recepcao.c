#include "../include/recepcao.h"
#include "../include/cliente.h"
#include "../include/scheduler.h"

static Fila *fila_global = NULL;
static int N_total = 0;
static int X_paciencia = 0;
static int clientes_gerados = 0;

void configurar_recepcao(Fila *f_compartilhada, int n_clientes, int x_paciencia) {
    fila_global = f_compartilhada;
    N_total = n_clientes;
    X_paciencia = x_paciencia;
    clientes_gerados = 0;
}

void funcao_recepcao(void) {
    int id_seq = 1;

    while (true) {
        if (N_total > 0 && clientes_gerados >= N_total) {
            yield_thread();
            continue;
        }

        Cliente *c = criar_cliente(id_seq++, X_paciencia);
        c->tempo_chegada = obter_tempo_ms();

        enfileirar(fila_global, c);
        clientes_gerados++;

        // Delay cooperativo: espera 100 ms sem bloquear o processo
        long long inicio_espera = obter_tempo_ms();
        while (obter_tempo_ms() - inicio_espera < 100) {
            yield_thread();
        }
    }
}