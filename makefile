CC = gcc
CFLAGS = -g -Wall
OBJ_CLIENT = raw_socket.o protocol.o  client.o local_lib.o
OBJ_SERVER = raw_socket.o protocol.o  server.o server_lib.o local_lib.o
DEPS = raw_socket.h protocol.h local_lib.h

all: client server

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

# Cria o executavel client
client: $(OBJ_CLIENT)
	$(CC) -o $@ $^ $(CFLAGS)

# Cria o executavel server
server: $(OBJ_SERVER)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	@rm -f *.o

purge: clean
	@rm server client

# EOF ----------------------------------------------------------