CC = gcc
CFLAGS = -Wall -g -Iinclude

SRC = src/main.c src/cliente.c src/fila.c src/scheduler.c src/recepcao.c src/atendente.c src/analista.c
OBJ = $(SRC:.c=.o)
TARGET = sistema_atendimento

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET) pids_atendidos.txt

run: all
	./$(TARGET) 20 1000