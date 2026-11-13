#ifndef CLIENTE_H
#define CLIENTE_H

#include "comum.h"

/*
 * Criação do Processo 3 ("Cliente") pela thread Recepção.
 *
 * O binário do cliente é dado pronto pelo professor (src/cliente_proc.c, compilado
 * como ./cliente) e é o mesmo para todas as equipes. Ele escreve em demanda.txt o
 * tempo de que precisa para ser atendido e chama raise(SIGSTOP), ficando dormindo
 * até o Atendente acordá-lo com SIGCONT.
 *
 * criar_cliente() faz fork + exec, espera o filho parar (sem bloquear o escalonador),
 * lê o arquivo Demanda e devolve a estrutura pronta para entrar na fila.
 * Devolve NULL se não conseguiu criar o processo.
 */
Cliente* criar_cliente(int X);

#endif
