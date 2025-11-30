# P2P File Sharing System

A peer-to-peer file sharing system that allows users to share and download files through a centralized server for indexing.

## System Architecture

- **Server**: Manages user authentication, file indexing, and search functionality  
- **Client**: Handles user registration/login, file sharing, searching, and downloading

## Features

### Server Features
- User registration and authentication system
- Session management with automatic client ID generation
- File index management (tracks which clients have which files)
- File search functionality
- Activity logging
- Multi-threaded client handling

### Client Features
- Automatic client ID generation (saved in config.txt)
- User registration and login
- File sharing with local index management
- File search across the network
- Direct P2P file downloading
- Download server for serving files to other clients
- Automatic error reporting to server

## Quick Start

### Build and Run

**Windows:**
```cmd
# Build
gcc -Wall -Wextra -std=c99 -o server.exe server.c -lws2_32
gcc -Wall -Wextra -std=c99 -o client.exe client.c -lws2_32

# Run server
server.exe

# Run client (in another terminal)
client.exe
```

**Linux:**
```bash
# Build
make all

# Run server
./server

# Run client (in another terminal)
./client
```

### Usage Steps
1. Start server first
2. Start client(s)
3. Register new user account
4. Login with credentials
5. Share files using menu option 4
6. Search files using menu option 5
7. Download files using menu option 6

## Technical Details

### Client ID System
- Each client generates unique 32-bit ID on first run
- Stored in `config.txt` for session management
- Used by server to track active clients and files

### P2P Download Process
1. Client searches for file through server
2. Server returns list of clients with the file
3. Client connects directly to file owner
4. Direct TCP transfer on port 9090
5. Error reporting updates server index

### Security & Limitations
- Basic username/password authentication
- No encryption (educational use)
- No file integrity verification
- Cross-platform Windows/Linux support