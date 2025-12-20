# 1. CONFIGS
CC = gcc

COMMON_CFLAGS = -g -Wall -Wextra -pthread -Icommon
SERVER_CFLAGS = $(COMMON_CFLAGS) -Iserver/include
CLIENT_CFLAGS = $(COMMON_CFLAGS) -Iclient/include

PORT ?= 8080
IP ?= 127.0.0.1

# 2. Variables
BUILD_DIR = build
BIN_DIR   = bin
TEST_DIR  = test

COMMON_SRCS = $(wildcard common/*.c)
SERVER_SRCS = $(wildcard server/src/*.c)
CLIENT_SRCS = $(wildcard client/src/*.c)

COMMON_OBJS = $(patsubst common/%.c, $(BUILD_DIR)/common/%.o, $(COMMON_SRCS))
SERVER_OBJS = $(patsubst server/src/%.c, $(BUILD_DIR)/server/%.o, $(SERVER_SRCS))
CLIENT_OBJS = $(patsubst client/src/%.c, $(BUILD_DIR)/client/%.o, $(CLIENT_SRCS))

SERVER_EXEC = $(BIN_DIR)/server_app
CLIENT_EXEC = $(BIN_DIR)/client_app

# 3. Main targets

.PHONY: all clean build clean-test setup-test run-server run-client1 run-client2

all: build

build: $(SERVER_EXEC) $(CLIENT_EXEC)

# Link server app
$(SERVER_EXEC): $(SERVER_OBJS) $(COMMON_OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking Server..."
	$(CC) $(SERVER_CFLAGS) -o $@ $^
	@echo "Server built successfully!"

# Link client app
$(CLIENT_EXEC): $(CLIENT_OBJS) $(COMMON_OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking Client..."
	$(CC) $(CLIENT_CFLAGS) -o $@ $^
	@echo "Client built successfully!"

# Rule for Common objects
$(BUILD_DIR)/common/%.o: common/%.c
	@mkdir -p $(@D)
	$(CC) $(COMMON_CFLAGS) -c -o $@ $<

# Rule for Server objects
$(BUILD_DIR)/server/%.o: server/src/%.c
	@mkdir -p $(@D)
	$(CC) $(SERVER_CFLAGS) -c -o $@ $<

# Rule for Client objects
$(BUILD_DIR)/client/%.o: client/src/%.c
	@mkdir -p $(@D)
	$(CC) $(CLIENT_CFLAGS) -c -o $@ $<

# 4. Setup test environment

setup-test: build
	@mkdir -p $(TEST_DIR)/client1
	@cp $(CLIENT_EXEC) $(TEST_DIR)/client1/
	@mkdir -p $(TEST_DIR)/client2
	@cp $(CLIENT_EXEC) $(TEST_DIR)/client2/
	@echo "Test environment setup complete."

# 5. Run commands
run-server: build
	./$(SERVER_EXEC) $(PORT)

run-client: setup-test
	cd $(TEST_DIR)/client1 && ./client_app $(IP) $(PORT)

# 6. Clean 
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(TEST_DIR)
	@echo "All build and test files removed."

clean-test: 
	rm -rf $(TEST_DIR)
	@echo "Test directories removed."
