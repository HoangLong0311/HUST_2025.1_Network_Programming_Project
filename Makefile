# Compiler configs
CC = gcc 
CFLAGS = -g -Wall -Wextra -pthread -Icommon -Iclient/include -Iserver/include

# Define Directory
BUILD_DIR = build
BIN_DIR = bin

# Define Sources
COMMON_SRCS = $(wildcard common/*.c)
SERVER_SRCS = $(wildcard server/src/*.c)
CLIENT_SRCS = $(wildcard client/src/*.c)

# Define Objects

COMMON_OBJS = $(patsubst common/%.c, $(BUILD_DIR)/common/%.o, $(COMMON_SRCS))
SERVER_OBJS = $(patsubst server/src/%.c, $(BUILD_DIR)/server/%.o, $(SERVER_SRCS))
CLIENT_OBJS = $(patsubst client/src/%.c, $(BUILD_DIR)/client/%.o, $(CLIENT_SRCS))

# Executables
SERVER_EXEC = $(BIN_DIR)/server_app
CLIENT_EXEC = $(BIN_DIR)/client_app

# Target

all: build

build: $(SERVER_EXEC) $(CLIENT_EXEC)

## Link server app 
$(SERVER_EXEC): $(SERVER_OBJS) $(COMMON_OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking Server..."
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Server built successfully!"

## Link client app 
$(CLIENT_EXEC): $(CLIENT_OBJS) $(COMMON_OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking Client..."
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Client built successfully!"

# Compile Rules 

## Rule for Common 
$(BUILD_DIR)/common/%.o: common/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

## Rule for Server 
$(BUILD_DIR)/server/%.o: server/src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

## Rule for Client 
$(BUILD_DIR)/client/%.o: client/src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Cleaned build and bin directories."