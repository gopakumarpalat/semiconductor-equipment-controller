CC = gcc

CFLAGS = -Wall -Wextra -Iinclude

TARGET = build/equipment_controller.exe

SRC = src/main.c src/equipment.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: all
	.\$(TARGET)

clean:
	del /Q build\*.exe 2>nul