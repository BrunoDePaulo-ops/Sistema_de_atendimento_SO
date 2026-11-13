CC      = gcc
CFLAGS  = -Wall -Wextra -g -Iinclude
# -pthread entra só por causa de sem_open/sem_wait: em glibc anterior à 2.34 os
# semáforos POSIX ficam na libpthread. Nenhuma thread de biblioteca é usada — as
# threads do trabalho são de usuário, feitas na mão com ucontext (src/scheduler.c).
LDLIBS  = -pthread

SRC     = src/main.c src/cliente.c src/fila.c src/scheduler.c \
          src/recepcao.c src/atendente.c src/analista.c src/monitor.c
OBJ     = $(SRC:.c=.o)
HDR     = $(wildcard include/*.h)

TARGET  = sistema_atendimento
CLIENTE = cliente

all: $(TARGET) $(CLIENTE)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDLIBS)

# Processo 3: código dado pelo professor, compilado como binário separado.
$(CLIENTE): src/cliente_proc.c
	$(CC) $(CFLAGS) src/cliente_proc.c -o $@ $(LDLIBS)

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET) $(CLIENTE) lng.txt lng.txt.tmp demanda.txt analista.pid

run: all
	./$(TARGET) 50 1000

run-infinito: all
	./$(TARGET) 0 1000

.PHONY: all clean run run-infinito
