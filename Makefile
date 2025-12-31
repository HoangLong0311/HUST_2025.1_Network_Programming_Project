# 1. Configs
CC = gcc

COMMON_CFLAGS = -g -Wall -Wextra -pthread -Icommon
SERVER_CFLAGS = $(COMMON_CFLAGS) -Iserver/include
CLIENT_CFLAGS = $(COMMON_CFLAGS) -Iclient/include

PORT ?= 8080
IP ?= 127.0.0.1
P2P_PORT1 = 20001
P2P_PORT2 = 20002
P2P_PORT3 = 20003

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

all: build

build: $(SERVER_EXEC) $(CLIENT_EXEC)

# Link server app
$(SERVER_EXEC): $(SERVER_OBJS) $(COMMON_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(SERVER_CFLAGS) -o $@ $^
	@echo "Server built successfully!"

# Link client app
$(CLIENT_EXEC): $(CLIENT_OBJS) $(COMMON_OBJS)
	@mkdir -p $(BIN_DIR)
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

setup-client1: clean-client1 build
	@mkdir -p $(TEST_DIR)/client1
	@cp $(CLIENT_EXEC) $(TEST_DIR)/client1/
	@mkdir $(TEST_DIR)/client1/shared
	@echo "Hello from client 1" > $(TEST_DIR)/client1/shared/hello.txt
	@echo "Bye from client 1" > $(TEST_DIR)/client1/shared/bye.txt

setup-client2: clean-client2 build
	@mkdir -p $(TEST_DIR)/client2
	@cp $(CLIENT_EXEC) $(TEST_DIR)/client2/
	@mkdir $(TEST_DIR)/client2/shared
	@echo "Hello from client 2" > $(TEST_DIR)/client2/shared/hello.txt
	@echo "Only contained in client 2" > $(TEST_DIR)/client2/shared/unique.txt
	@echo "Bye from client 2" > $(TEST_DIR)/client2/shared/bye.txt
	
setup-client3: clean-client3 build
	@mkdir -p $(TEST_DIR)/client3
	@cp $(CLIENT_EXEC) $(TEST_DIR)/client3/
	@mkdir -p $(TEST_DIR)/client3/downloads
	@echo "Test environment setup completed." 

# 5. Run commands
run-server: build
	./$(SERVER_EXEC) $(PORT)

run-client%: setup-client%
	cd $(TEST_DIR)/client$* && ./client_app $(IP) $(PORT) $(P2P_PORT$*)

# 6. Clean 
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(TEST_DIR)
	@echo "All build and test files removed."
clean-test: 
	rm -rf  $(TEST_DIR)
	@echo "Test environment removed"
clean-client%: 
	rm -rf $(TEST_DIR)/client$*

