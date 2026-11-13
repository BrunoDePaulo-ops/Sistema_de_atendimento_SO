#include "../include/analista.h"

static volatile sig_atomic_t acordado = 0;
static volatile sig_atomic_t terminar = 0;

static sem_t *sem_block = SEM_FAILED;

static void tratar_sigusr1(int sig)
{
    (void) sig;
    acordado = 1;
}

static void tratar_sigterm(int sig)
{
    (void) sig;
    terminar = 1;
    acordado = 1;
}

/* Tamanho atual do arquivo LNG, para saber se ainda há o que imprimir. */
static long tamanho_lng(void)
{
    struct stat st;
    return (stat(ARQUIVO_LNG, &st) == 0) ? (long) st.st_size : 0L;
}

/*
 * Imprime no máximo LOTE_ANALISTA valores e apaga do arquivo exatamente os valores
 * que imprimiu. Todo o ciclo ler/reescrever acontece com o LNG bloqueado por
 * "/sem_block": sem isso, o que o Atendente escrevesse entre a leitura e a reescrita
 * seria apagado sem nunca ter sido impresso.
 *
 * O resto do arquivo é copiado para um temporário e renomeado por cima do original,
 * então não existe limite de quantas linhas podem estar acumuladas.
 */
static void processar_lng(void)
{
    char lote[LOTE_ANALISTA][MAX_LINHA];
    int n = 0;

    if (sem_block != SEM_FAILED)
        sem_wait(sem_block);                 /* bloqueia o LNG */

    FILE *f = fopen(ARQUIVO_LNG, "r");
    if (!f) {
        if (sem_block != SEM_FAILED) sem_post(sem_block);
        return;
    }

    while (n < LOTE_ANALISTA && fgets(lote[n], MAX_LINHA, f) != NULL) {
        lote[n][strcspn(lote[n], "\n")] = '\0';
        if (lote[n][0] != '\0')
            n++;
    }

    if (n == 0) {                            /* nada para imprimir */
        fclose(f);
        if (sem_block != SEM_FAILED) sem_post(sem_block);
        return;
    }

    /* Apaga do arquivo os valores impressos, preservando o resto. */
    static const char *temporario = ARQUIVO_LNG ".tmp";
    FILE *t = fopen(temporario, "w");
    if (t) {
        char buf[4096];
        size_t lidos;
        while ((lidos = fread(buf, 1, sizeof(buf), f)) > 0)
            fwrite(buf, 1, lidos, t);
        fclose(t);
    }
    fclose(f);
    if (t)
        rename(temporario, ARQUIVO_LNG);

    if (sem_block != SEM_FAILED)
        sem_post(sem_block);                 /* desbloqueia o LNG */

    printf("\n[Analista] --- PIDs atendidos (%d) ---\n", n);
    for (int i = 0; i < n; i++)
        printf("%s\n", lote[i]);
    printf("--------------------------------------\n");
    fflush(stdout);
}

void iniciar_processo_analista(void)
{
    /* Saída linha a linha: sem isso, quando a saída do programa é redirecionada para
     * um arquivo ou um pipe, tudo o que o Analista imprime fica no buffer e se perde
     * quando o processo é encerrado. */
    setvbuf(stdout, NULL, _IOLBF, 0);

    /* "Gera arquivo com seu PID e dorme" (diagrama). */
    FILE *f = fopen(ARQUIVO_PID_ANALISTA, "w");
    if (f) {
        fprintf(f, "%d\n", (int) getpid());
        fclose(f);
    }

    sem_block = sem_open(SEM_BLOCK, 0);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sa.sa_handler = tratar_sigusr1;
    sigaction(SIGUSR1, &sa, NULL);
    sa.sa_handler = tratar_sigterm;
    sigaction(SIGTERM, &sa, NULL);

    /*
     * Dorme esperando sinal. SIGUSR1 e SIGTERM ficam bloqueados fora do sigsuspend,
     * o que elimina a corrida clássica do pause(): um sinal que chegasse entre o teste
     * da variável e o pause() seria perdido e o Analista dormiria para sempre.
     */
    sigset_t bloqueados, anterior;
    sigemptyset(&bloqueados);
    sigaddset(&bloqueados, SIGUSR1);
    sigaddset(&bloqueados, SIGTERM);
    sigprocmask(SIG_BLOCK, &bloqueados, &anterior);

    while (!terminar) {
        while (!acordado && !terminar)
            sigsuspend(&anterior);           /* dorme com os sinais liberados */

        acordado = 0;
        sigprocmask(SIG_SETMASK, &anterior, NULL);
        processar_lng();
        sigprocmask(SIG_BLOCK, &bloqueados, NULL);
    }

    sigprocmask(SIG_SETMASK, &anterior, NULL);

    /* Encerramento: esvazia o arquivo, imprimindo tudo o que ainda restou.
     * É o que o enunciado exige quando o usuário aperta "s" no modo infinito. */
    long restante = tamanho_lng();
    while (restante > 0) {
        processar_lng();
        long agora = tamanho_lng();
        if (agora >= restante)
            break;                           /* não está diminuindo: evita laço infinito */
        restante = agora;
    }

    fflush(stdout);

    if (sem_block != SEM_FAILED)
        sem_close(sem_block);
    remove(ARQUIVO_PID_ANALISTA);
}
