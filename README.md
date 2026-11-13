# Simulação de Atendimento ao Cliente — Trabalho 1 (Sistemas Operacionais, CK0234)

Simulação do atendimento a clientes com **threads de usuário feitas na mão** (troca de contexto com
`ucontext`), **processos** separados para o Cliente e para o Analista, **sinais** (`SIGSTOP`, `SIGCONT`,
`SIGUSR1`, `SIGTERM`) e **semáforos nomeados POSIX** para coordenar o acesso aos arquivos compartilhados.

Nenhuma biblioteca de paralelismo é usada: o escalonador das threads é implementado em
`src/scheduler.c`, como o enunciado exige.

---

## Arquitetura

```
┌──────────────────────────────────────────────────────────┐
│  Processo 1 — ATENDIMENTO  (escalonador cooperativo)     │
│                                                          │
│  Thread 1 — Atendente          Thread 2 — Recepção       │
│  - lê o 1º da fila             - cria /sem_atend e       │
│  - acorda o cliente (SIGCONT)    /sem_block              │
│  - espera o atendimento        - cria os processos       │
│  - fecha /sem_block              cliente (fork + exec)   │
│  - grava o PID no LNG          - sorteia a prioridade    │
│  - abre /sem_block               (50% / 50%)             │
│  - calcula a satisfação        - lê demanda.txt          │
│  - tira o cliente da fila      - enfileira               │
│  - acorda o Analista           - pausa com 100 na fila   │
│          │                             │                 │
│          └────── Fila de clientes ◄────┘                 │
│                                                          │
│  Thread 3 — Monitor (só quando N = 0): lê a tecla "s"    │
└───────────┬──────────────────────────────┬───────────────┘
            │ grava (protegido             │ SIGCONT / espera o fim
            │ por /sem_block)              ▼
      ┌─────▼────────┐            ┌─────────────────────┐
      │  lng.txt     │            │ Processo 3 — CLIENTE│
      │  (Lista de   │            │ (./cliente, código  │
      │   Números    │            │  dado pelo professor)│
      │   Gerados)   │            │ escreve demanda.txt,│
      └─────┬────────┘            │ dorme (SIGSTOP),    │
            │ lê e apaga          │ fecha /sem_atend,   │
            │ (protegido)         │ usleep, abre, sai   │
      ┌─────▼────────────┐        └─────────────────────┘
      │ Processo 2 —     │ ◄── SIGUSR1 (acorda)
      │ ANALISTA         │ ◄── SIGTERM (encerra esvaziando)
      │ imprime até 10   │
      │ PIDs por vez     │
      └──────────────────┘
```

---

## Como compilar e executar

Requisitos: Linux (ou WSL), `gcc` e `make`.

```bash
make clean && make          # gera ./sistema_atendimento e ./cliente
./sistema_atendimento <N> <X>
```

* **N** — número de clientes a gerar. **0 = infinito**.
* **X** — paciência base, em milissegundos. Prioridade alta espera `X/2`; prioridade baixa espera `X`.

```bash
./sistema_atendimento 50 1000   # 50 clientes, paciência base de 1 s
./sistema_atendimento 0 1000    # infinito: aperte "s" para encerrar
make run                        # atalho para 50 1000
make run-infinito               # atalho para 0 1000
```

Os dois binários precisam estar na mesma pasta: o Atendimento executa `./cliente`.

### Arquivos criados em tempo de execução
| Arquivo | Para que serve |
|---|---|
| `demanda.txt` | o cliente escreve o tempo de que precisa; a Recepção lê e apaga |
| `lng.txt` | Lista de Números Gerados: PIDs atendidos, protegida por `/sem_block` |
| `analista.pid` | PID do processo Analista, gravado por ele ao nascer |

---

## Decisões de implementação

**Escalonador (`src/scheduler.c`).** Round-robin cooperativo com `getcontext`/`makecontext`/`swapcontext`.
Toda troca passa pelo contexto do escalonador: `yield_thread()` volta para ele e ele escolhe a próxima
thread PRONTA. Cada thread roda dentro de um trampolim que, ao fim do corpo, marca o TCB como
`TERMINADA` e devolve o controle — por isso `escalonar()` **retorna ao `main`** quando todas acabam, que é
o que permite esvaziar o arquivo LNG, imprimir o relatório e liberar tudo.

**Espera sem bloquear.** Num escalonador cooperativo, qualquer chamada bloqueante paralisa o processo
inteiro. Por isso toda espera é `WNOHANG`/`sem_trywait` + `yield_thread()`: esperar o cliente parar
(`waitpid(..., WUNTRACED|WNOHANG)`), esperar o fim do atendimento e esperar o semáforo do LNG.

**Fim do atendimento.** O cliente fecha `/sem_atend`, dorme e abre o semáforo imediatamente antes de
terminar — o fim do processo cliente é, portanto, o mesmo instante em que o semáforo reabre. O Atendente
espera pelo fim do processo em vez de disputar o semáforo: se ele também fizesse `sem_wait`, poderia tomar
a vez do próprio cliente e travar o atendimento.

**Exclusão mútua do LNG.** `/sem_block` é fechado em volta de toda a escrita do Atendente e de todo o ciclo
ler-imprimir-reescrever do Analista. Sem isso, o que fosse escrito entre a leitura e a reescrita seria
apagado sem nunca ter sido impresso.

**Analista sem corrida de sinal.** `SIGUSR1`/`SIGTERM` ficam bloqueados e a espera é feita com
`sigsuspend`, em vez de `pause()`. Com `pause()`, um sinal que chegasse entre o teste da variável e a
chamada seria perdido e o Analista dormiria para sempre. A saída usa `setvbuf(..., _IOLBF, ...)` para que
os PIDs apareçam mesmo quando a saída do programa é redirecionada para um arquivo ou um pipe.

**Estruturas de dados.** `Cliente` guarda exatamente os quatro campos do diagrama (PID, hora de chegada,
prioridade, tempo para atendimento). A fila é uma lista encadeada com ponteiro para o fim (`O(1)` para
enfileirar e desenfileirar). O Atendente consulta a frente, atende e só então remove, como no diagrama.

**Prioridade.** No enunciado ela só define a paciência (alta = `X/2`, baixa = `X`); a ordem de atendimento
continua FIFO.

**Encerramento com "s" (N = 0).** Uma terceira thread lê o teclado em modo não canônico e não bloqueante.
Ao receber "s": as threads param, os clientes que ficaram na fila são encerrados e descartados do cálculo,
o Analista é acordado quantas vezes forem necessárias até o LNG ficar vazio e só então recebe `SIGTERM`.

---

## Resultados medidos

Medições em Linux (container `gcc:13`), executando do disco local:

| Execução | Clientes atendidos | Taxa de satisfação | Tempo total |
|---|---|---|---|
| `./sistema_atendimento 20 1000` | 20 | 100,00% | 9 ms |
| `./sistema_atendimento 50 1000` | 50 | 100,00% | 22 ms |
| `./sistema_atendimento 100 1000` | 100 | 100,00% | 44 ms |
| `./sistema_atendimento 50 1` | 50 | 84,00% | — |
| `./sistema_atendimento 0 1000`, "s" após 3 s | 91 | 100,00% | 3029 ms |

Em todas as execuções o Analista imprimiu **todos** os PIDs atendidos (20/20, 50/50, 100/100, 91/91),
inclusive com a saída redirecionada para arquivo, e o `lng.txt` terminou vazio.
Com paciência muito curta (`X = 1` ms) a taxa cai para 84%, o que mostra que a métrica realmente mede a
espera e não está fixa em 100%.

> O tempo de execução depende do custo de `fork`+`exec` do sistema de arquivos onde os binários estão.
> Rodando a partir de um diretório montado do Windows no Docker, a mesma execução de 50 clientes levou
> 1879 ms em vez de 22 ms. Para medir desempenho, rode em disco Linux nativo.

---

## Estrutura do projeto

```text
.
├── makefile
├── README.md
├── include/
│   ├── analista.h    ├── atendente.h   ├── cliente.h
│   ├── comum.h       ├── fila.h        ├── monitor.h
│   ├── recepcao.h    └── scheduler.h
└── src/
    ├── main.c          # processo Atendimento: fork do Analista, threads, relatório
    ├── scheduler.c     # escalonador cooperativo (ucontext)
    ├── recepcao.c      # thread Recepção: semáforos, criação e enfileiramento
    ├── atendente.c     # thread Atendente: atende, grava no LNG, calcula satisfação
    ├── analista.c      # processo Analista: imprime e apaga os PIDs do LNG
    ├── monitor.c       # thread monitor do teclado ("s") e estado de encerramento
    ├── cliente.c       # criação do processo cliente (fork + exec) pela Recepção
    ├── fila.c          # fila encadeada de clientes
    └── cliente_proc.c  # Processo Cliente — CÓDIGO DADO PELO PROFESSOR, sem alterações
```
