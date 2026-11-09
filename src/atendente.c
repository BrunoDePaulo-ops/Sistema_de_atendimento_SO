#include "../include/atendente.h"
#include "../include/scheduler.h"
#include <signal.h>


static Fila *fila_global = NULL;
static int N_total = 0;
static int X_paciencia = 0;
static pid_t analista_pid = 0;

static int total_atendidos = 0;
static int satisfeitos = 0;
static long long tempo_inicio = 0;
static long long tempo_fim = 0;

void configurar_atendente(Fila *f_compartilhada, int n_clientes, int x_paciencia, pid_t pid_analista) {
    fila_global = f_compartilhada;
    N_total = n_clientes;
    X_paciencia = x_paciencia;
    analista_pid = pid_analista;
    tempo_inicio = obter_tempo_ms();
}

void funcao_atendente(void) {
    while (true) {
        // Se a fila estiver vazia, cede a vez para a Recepção gerar mais
        if (fila_vazia(fila_global)) {
            yield_thread();
            continue;
        }

        // Processa TODOS os clientes que estiverem na fila neste momento
        while (!fila_vazia(fila_global)) {
            Cliente *c = desenfileirar(fila_global);
            if (c != NULL) {
                long long tempo_atual = obter_tempo_ms();
                long long tempo_espera = tempo_atual - c->tempo_chegada;

                // Verifica se o tempo de espera respeitou a paciência máxima
                bool foi_satisfeito = (tempo_espera <= c->paciencia_max);
                if (foi_satisfeito) {
                    satisfeitos++;
                }
                total_atendidos++;

                // Escreve o ID/PID do cliente no arquivo compartilhado
                FILE *f = fopen(ARQUIVO_PIDS, "a");
                if (f) {
                    fprintf(f, "Cliente ID: %d | Prio: %d | Espera: %lld ms | Paciência: %lld ms | %s\n", 
                    c->id, c->prioridade, tempo_espera, c->paciencia_max, 
                    foi_satisfeito ? "SATISFEITO" : "INSATISFEITO");
                    fclose(f);
                }

                // Acorda o processo Analista (envia sinal SIGUSR1)
                if (analista_pid > 0) {
                    kill(analista_pid, SIGUSR1);
                }

                free(c);

                long long inicio_servico = obter_tempo_ms();
                while (obter_tempo_ms() - inicio_servico < 110) {
                    yield_thread(); // Permite que a Recepção continue injetando clientes na fila!
                }
            }
            // Se N for finito e já atendeu todos, encerra o loop da thread
            if (N_total > 0 && total_atendidos >= N_total) {
                tempo_fim = obter_tempo_ms();
            
                // Garante que o analista limpe o resto do arquivo antes de fechar
                if (analista_pid > 0) {
                    kill(analista_pid, SIGUSR1);
                    usleep(50000); // pequeníssimo delay para o analista processar
                }

                double taxa_satisfacao = 0.0;
                if (total_atendidos > 0) {
                    taxa_satisfacao = (double) satisfeitos / total_atendidos;
                }
                long long tempo_total = tempo_fim - tempo_inicio;

                printf("\n=== Relatorio Final de Atendimento ===\n");
                printf("Taxa de Satisfacao: %.2f%%\n", taxa_satisfacao * 100.0);
                printf("Tempo Total de Execucao: %lld ms\n", tempo_total);
                printf("======================================\n");

                // Encerra o processo analista e o programa principal de vez
                kill(analista_pid, SIGKILL);
                exit(0);
            }

        }

    }
}

void obter_estatisticas(double *taxa_satisfacao, long long *tempo_total) {
    if (total_atendidos > 0) {
        *taxa_satisfacao = (double) satisfeitos / total_atendidos;
    } else {
        *taxa_satisfacao = 0.0;
    }
    *tempo_total = tempo_fim - tempo_inicio;
}

