#ifndef MONITOR_H
#define MONITOR_H

#include "comum.h"

/*
 * Estado de encerramento e monitoramento do teclado.
 *
 * No modo infinito (N = 0) o enunciado manda o usuário poder encerrar o atendimento
 * a qualquer momento apertando "s". Como o escalonador é cooperativo, ler o teclado
 * de forma bloqueante travaria o programa inteiro: a thread monitor lê a entrada em
 * modo não bloqueante e cede o processador a cada volta.
 */

void monitor_configurar(void);      /* prepara o terminal e o stdin não bloqueante */
void monitor_restaurar(void);       /* devolve o terminal ao estado original       */
void funcao_monitor_teclado(void);  /* corpo da thread monitor                     */

void pedir_encerramento(void);          /* qualquer módulo pode pedir o encerramento */
void pedir_encerramento_por_erro(void); /* encerra e faz o programa sair com erro    */
bool encerramento_pedido(void);
bool houve_erro(void);

#endif
