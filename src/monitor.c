#include "../include/monitor.h"
#include "../include/scheduler.h"
#include <termios.h>

static bool encerrar = false;
static bool terminal_alterado = false;
static struct termios termios_original;
static int flags_stdin_originais = -1;

static bool erro = false;

void pedir_encerramento(void)
{
    encerrar = true;
}

/* Encerramento por falha (por exemplo, não dá para criar o processo cliente):
 * além de parar as threads, faz o programa terminar com código de saída diferente
 * de zero, para quem chamou saber que a simulação não rodou. */
void pedir_encerramento_por_erro(void)
{
    erro = true;
    encerrar = true;
}

bool encerramento_pedido(void)
{
    return encerrar;
}

bool houve_erro(void)
{
    return erro;
}

/*
 * Coloca o terminal em modo não canônico (sem esperar Enter) e o stdin em modo não
 * bloqueante. Se a entrada não for um terminal (por exemplo, um pipe), só o modo não
 * bloqueante é aplicado.
 */
void monitor_configurar(void)
{
    flags_stdin_originais = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags_stdin_originais != -1)
        fcntl(STDIN_FILENO, F_SETFL, flags_stdin_originais | O_NONBLOCK);

    if (isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &termios_original) == 0) {
        struct termios bruto = termios_original;
        bruto.c_lflag &= ~(ICANON | ECHO);   /* lê tecla a tecla, sem eco */
        bruto.c_cc[VMIN] = 0;
        bruto.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &bruto) == 0)
            terminal_alterado = true;
    }
}

/* Devolve o terminal e o stdin exatamente como estavam: se o programa saísse deixando
 * o stdin em modo não bloqueante, o shell que o chamou passaria a dar erro de leitura. */
void monitor_restaurar(void)
{
    if (terminal_alterado) {
        tcsetattr(STDIN_FILENO, TCSANOW, &termios_original);
        terminal_alterado = false;
    }
    if (flags_stdin_originais != -1) {
        fcntl(STDIN_FILENO, F_SETFL, flags_stdin_originais);
        flags_stdin_originais = -1;
    }
}

/*
 * Thread monitor: a cada volta lê o que houver no teclado sem bloquear. Qualquer
 * "s" ou "S" encerra o atendimento. Termina sozinha quando o encerramento é pedido
 * por outro caminho (por exemplo, os N clientes acabaram).
 */
void funcao_monitor_teclado(void)
{
    char buf[16];

    while (!encerramento_pedido()) {
        ssize_t lidos = read(STDIN_FILENO, buf, sizeof(buf));
        for (ssize_t i = 0; i < lidos; i++) {
            if (buf[i] == 's' || buf[i] == 'S') {
                printf("\n[Atendimento] Tecla 's' pressionada: encerrando...\n");
                fflush(stdout);
                pedir_encerramento();
                return;
            }
        }
        yield_thread();
    }
}
