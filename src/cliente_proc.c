/*
 * Processo 3 — "Cliente".
 *
 * ESTE CÓDIGO É DADO PRONTO PELO PROFESSOR (anexo da descrição do trabalho,
 * /home/yuri/Documentos/UFC/aulas/20242/trab/cliente.c). A nota de rodapé 5 do
 * enunciado diz que todas as equipes usam o mesmo processo cliente, então ele é
 * reproduzido aqui sem alterações e compilado no binário ./cliente.
 *
 * O que ele faz:
 *   1. sorteia o tempo de que precisa para ser atendido (15, 5 ou 1 microssegundo);
 *   2. escreve esse tempo no arquivo demanda.txt, que a Recepção lê;
 *   3. para a si mesmo com raise(SIGSTOP) — fica dormindo até receber SIGCONT;
 *   4. ao acordar, fecha o semáforo "/sem_atend", dorme o tempo do atendimento,
 *      abre o semáforo e termina.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>

int main()
{
    int tempo;
    srand( (unsigned)time(NULL) );
    int x = rand()%10;

    if(x == 0)
        tempo = 15;
    else if(x > 0 && x <= 3)
        tempo = 5;
    else
        tempo = 1;

    FILE* demanda = fopen("demanda.txt", "w+");
    fprintf(demanda, "%d", tempo);
    fclose(demanda);

    raise(SIGSTOP);

    sem_t *sem = sem_open("/sem_atend", O_RDWR);

    if(sem != SEM_FAILED) sem_wait(sem);

    usleep(tempo);

    if(sem != SEM_FAILED) sem_post(sem);

    return 0;
}
