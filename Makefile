CC = gcc

CFLAGS = -Wall -Wextra -Iinclude -pthread


# Existing Day 2-4 equipment program
EQUIPMENT_TARGET = build/equipment_controller.exe

EQUIPMENT_SRC = src/main.c src/equipment.c


# Day 6 TCP server
SERVER_TARGET = build/persistent_tcp_server.exe

SERVER_SRC = src/persistent_tcp_server.c src/equipment.c src/equipment_config.c


# Day 6 TCP client
CLIENT_TARGET = build/persistent_tcp_client.exe

CLIENT_SRC = src/persistent_tcp_client.c src/equipment_config.c

# Day 8 Configuration management
CONFIG_TARGET = build/config_test

CONFIG_SRC = tests/config_test.c src/equipment_config.c


# Build everything
all: equipment server client config_test


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

# Configuration test
config_test:
	$(CC) $(CFLAGS) $(CONFIG_SRC) -o $(CONFIG_TARGET)