# 🚀 Simulação de Atendimento Multiprocesso e Multithread em C

Bem-vindo ao repositório do **Sistema de Simulação de Atendimento ao Cliente**! 

Este projeto é uma aplicação desenvolvida em C para sistemas POSIX (Linux), projetada para demonstrar conceitos avançados de **Sistemas Operacionais**, como concorrência cooperativa via `ucontext.h`, comunicação entre processos (IPC) com sinais (`SIGUSR1`), gerência de arquivos compartilhados thread-safe e análise estatística baseada na Teoria das Filas.

---

## 📌 Sumário
1. [Visão Geral da Arquitetura](#-visão-geral-da-arquitetura)
2. [Conceitos Fundamentais](#-conceitos-fundamentais)
3. [Regras de Negócio e Tolerância](#-regras-de-negócio-e-tolerância)
4. [Estrutura do Projeto](#-estrutura-do-projeto)
5. [Como Compilar e Executar](#-como-compilar-e-executar)
6. [Cenários de Teste e Validação](#-cenários-de-teste-e-validação)
7. [Aprendizados e Teoria das Filas](#-aprendizados-e-teoria-das-filas)

---

## 🏗 Visão Geral da Arquitetura

O sistema opera com uma arquitetura híbrida que combina **multiprocessamento (processos SO)** e **multithreading em espaço de usuário (User-level Threads)**:

```
┌─────────────────────────────────────────────────────────────┐
│               Processo Principal de Atendimento             │
│                                                             │
│  ┌────────────────────┐            ┌─────────────────────┐  │
│  │ Thread: Recepção   │            │ Thread: Atendente   │  │
│  │ (Gera clientes)    │            │ (Consome a fila)    │  │
│  └─────────┬──────────┘            └──────────┬──────────┘  │
│            │   Fila Global de Clientes        │             │
│            └───────────────► 📥 ──────────────┘             │
└──────────────────────────────┬──────────────────────────────┘
                               │
                Grava log      │  Envia Sinais
             no arquivo audit  │  SIGUSR1
                               ▼
            ┌────────────────────────────────────┐
            │         Processo Analista          │
            │  (Processa logs & exibe no stdout) │
            └────────────────────────────────────┘
```

---

## 💡 Conceitos Fundamentais

* **Threads Cooperativas (`ucontext.h`):** Em vez de utilizar threads do kernel (`pthread`), o sistema faz a troca de contexto explicitamente usando a API POSIX `ucontext.h` (funções `getcontext`, `makecontext`, `swapcontext`). Isso exige trocas de turno manuais através da função `yield_thread()` para que o tempo de CPU seja compartilhado de forma justa sem travar o processo principal.
* **Comunicação Interprocessos (IPC via `SIGUSR1`):** A cada atendimento finalizado, o processo de atendimento envia um sinal `SIGUSR1` para o processo **Analista**, notificando que há novos registros para processamento.
* **Audit Trail Thread-Safe:** A troca de dados detalhados entre o atendente e o analista ocorre por meio de um arquivo de log compartilhado (`ARQUIVO_PIDS`). O Analista lê continuamente o arquivo usando controle preciso de *file pointer* (`fgets`) para garantir consistência.

---

## 📐 Regras de Negócio e Tolerância

Cada cliente gerado na recepção possui atributos únicos e um tempo máximo de paciência:

* **Prioridade 1 (Alta Prioridade):** Tolerância máxima reduzida pela metade ($\text{Paciência} = \frac{X}{2}$).
* **Prioridade 2 (Baixa Prioridade):** Tolerância máxima integral ($\text{Paciência} = X$).

### Cálculo de Satisfação
$$\text{Tempo de Espera} = \text{Tempo de Atendimento} - \text{Tempo de Chegada}$$
* Se $\text{Tempo de Espera} \le \text{Paciência Máxima} \rightarrow \mathbf{SATISFEITO}$
* Se $\text{Tempo de Espera} > \text{Paciência Máxima} \rightarrow \mathbf{INSATISFEITO}$

No final da execução, o relatório exibe a **Taxa de Satisfação Geral** ($\frac{\text{Total Satisfeitos}}{\text{Total Atendidos}} \times 100$).

---

## 📁 Estrutura do Projeto

```text
.
├── Makefile                # Automação de compilação
├── README.md               # Documentação do projeto
├── include/                # Cabeçalhos (.h)
│   ├── atendente.h
│   ├── cliente.h
│   ├── recepcao.h
│   └── scheduler.h
└── src/                    # Código fonte (.c)
    ├── analista.c          # Processo secundário (leitor de auditoria)
    ├── atendente.c         # Thread consumidora de clientes
    ├── cliente.c           # Lógica de criação de structs de clientes
    ├── main.c              # Ponto de entrada e tratamento de argumentos
    ├── recepcao.c          # Thread produtora de clientes
    └── scheduler.c         # Alternância de contexto (ucontext)
```

---

## 🛠 Como Compilar e Executar

### Pré-requisitos
* Sistema operacional POSIX (Linux, WSL ou macOS).
* Compilador `gcc` e utilitário `make`.

### Compilando o Projeto
Para limpar builds antigos e compilar o código fonte:
```bash
make clean && make
```

### Executando a Simulação
O programa aceita dois parâmetros principais via linha de comando:
```bash
./sistema_atendimento <N> <X>
```
* **`N`**: Número total de clientes a serem gerados no lote (ex: `50`).
* **`X`**: Tempo limite de paciência base em milissegundos (ex: `800`).

Exemplo:
```bash
./sistema_atendimento 50 800
```

---

## 📊 Cenários de Teste e Validação

Você pode validar a estabilidade e a sensibilidade matemática do sistema ajustando os parâmetros $N$ e $X$:

### 1. Teste de Carga Pesada / Colapso de Fila ($\text{Satisfação} < 10\%$)
Ao usar uma paciência baixa em relação à taxa de acúmulo da fila, os clientes estouram a paciência rapidamente:
```bash
./sistema_atendimento 50 200
# Resultado esperado: Satisfação em torno de 4% a 10%
```

### 2. Teste de Equilíbrio / Carga Moderada ($\text{Satisfação} \approx 60\%$)
Ajustando o limite $X$ para um valor intermediário:
```bash
./sistema_atendimento 50 400
# Resultado esperado: Satisfação em torno de 60%
```

### 3. Teste de Alto Desempenho ($\text{Satisfação} > 80\%$)
Aumentando a paciência base $X$, a fila consegue absorver a demanda com poucas insatisfações:
```bash
./sistema_atendimento 50 800
# Resultado esperado: Satisfação em torno de 85% a 90%
```

---

## 🎓 Aprendizados e Teoria das Filas

Durante a construção e validação deste projeto, observamos conceitos clássicos de **Teoria das Filas (Modelo M/M/1)**:

1. **Taxa de Chegada ($\lambda$) vs. Taxa de Serviço ($\mu$):** Se a taxa com que os clientes chegam for maior do que a velocidade com que o atendente processa a fila, o tempo de espera acumulado cresce de forma linear/exponencial, degradando a satisfação do sistema.
2. **Delays Cooperativos vs. Blocking Syscalls:** Em sistemas baseados em threads em espaço de usuário (`ucontext.h`), chamadas bloqueantes clássicas como `usleep()` paralisam o processo inteiro do SO. A solução adotada foi o uso de **loops de espera cooperativa com `yield_thread()`**, garantindo o escalonamento alternado de CPU em tempo real.

---

✨ *Desenvolvido para estudo de Concorrência e Sistemas Operacionais.*