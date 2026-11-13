#ifndef ANALISTA_H
#define ANALISTA_H

#include "comum.h"

/*
 * Processo 2 — "Analista" (diagrama do enunciado):
 *   - gera um arquivo com seu PID e dorme;
 *   - quando acordado (SIGUSR1): bloqueia o LNG, lê e imprime no máximo os 10
 *     primeiros valores, apaga do arquivo os que imprimiu, desbloqueia e dorme de novo;
 *   - ao receber SIGTERM, imprime tudo o que ainda restar no arquivo e termina.
 *
 * Roda no processo filho criado por fork() no main.
 */
void iniciar_processo_analista(void);

#endif
