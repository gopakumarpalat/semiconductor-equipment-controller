CC = gcc

CFLAGS = -Wall -Wextra -Iinclude -pthread


# Existing Day 2-4 equipment program
EQUIPMENT_TARGET = build/equipment_controller.exe

EQUIPMENT_SRC = src/main.c src/equipment.c


# Day 6 TCP server
SERVER_TARGET = build/persistent_tcp_server.exe

SERVER_SRC = src/persistent_tcp_server.c src/equipment.c


# Day 6 TCP client
CLIENT_TARGET = build/persistent_tcp_client.exe

CLIENT_SRC = src/persistent_tcp_client.c


# Build everything
all: equipment server client


# Existing equipment program
equipment:
	$(CC) $(CFLAGS) $(EQUIPMENT_SRC) -o $(EQUIPMENT_TARGET)


# TCP server
server:
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_TARGET)


# TCP client
client:
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_TARGET)


# Run existing equipment program
run: equipment
	.\$(EQUIPMENT_TARGET)


# Clean executables
clean:
	del /Q build\*.exe 2>nul