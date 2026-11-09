#include "../include/analista.h"
#include <signal.h>

static volatile sig_atomic_t sinal_recebido = 0;

void manipulador_sinal(int sig) {
    if (sig == SIGUSR1) {
        sinal_recebido = 1;
    }
}

void processar_arquivo_pids() {
    FILE *f = fopen(ARQUIVO_PIDS, "r");
    if (!f) return;

    char linhas[1000][256];
    int count = 0;

    // Lê linha por linha até o final do arquivo
    while (fgets(linhas[count], sizeof(linhas[count]), f) != NULL && count < 1000) {
        // Remove a quebra de linha se existir, para formatação limpa
        linhas[count][strcspn(linhas[count], "\n")] = '\0';
        if (strlen(linhas[count]) > 0) {
            count++;
        }
    }
    fclose(f);

    if (count == 0) return;

    // Imprime no máximo os 10 primeiros
    int qtd_imprimir = count < 10 ? count : 10;
    
    printf("\n[Analista] --- Lote de PIDs Atendidos ---\n");
    for (int i = 0; i < qtd_imprimir; i++) {
        printf("%s\n", linhas[i]);
    }
    printf("---------------------------------------\n");

    // Reescreve o arquivo mantendo apenas as linhas que ainda não foram impressas
    f = fopen(ARQUIVO_PIDS, "w");
    if (f) {
        for (int i = qtd_imprimir; i < count; i++) {
            fprintf(f, "%s\n", linhas[i]);
        }
        fclose(f);
    }
}

void iniciar_processo_analista(void) {
    // Configura o tratador do sinal SIGUSR1
    struct sigaction sa;
    sa.sa_handler = manipulador_sinal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    while (1) {
        // Fica dormindo até receber um sinal do S.O.
        pause();

        if (sinal_recebido) {
            processar_arquivo_pids();
            sinal_recebido = 0;
        }
    }
}