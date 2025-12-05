# Compiler configs
CC = gcc 
CFLAGS = -g -Wall -Wextra -pthread -Isrc/common

# Define variable
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

COMMON_SRCS = $(wildcard $(SRC_DIR)/common/*.c)
SERVER_SRCS = $(wildcard $(SRC_DIR)/server/*.c)
CLIENT_SRCS = $(wildcard $(SRC_DIR)/client/*.c)

COMMON_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(COMMON_SRCS))
SERVER_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SERVER_SRCS))
CLIENT_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CLIENT_SRCS))

SERVER_EXEC = $(BIN_DIR)/server_app
CLIENT_EXEC = $(BIN_DIR)/client_app

# All
all: build

# Build
build: $(SERVER_EXEC) $(CLIENT_EXEC)
## Link server app 
$(SERVER_EXEC): $(SERVER_OBJS) $(COMMON_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Compiled successfully!"
## Link client app 
$(CLIENT_EXEC): $(CLIENT_OBJS) $(COMMON_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^
## Compile .c to .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Cleaned build and bin directories."