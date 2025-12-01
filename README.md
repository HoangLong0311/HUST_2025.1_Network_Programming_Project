# P2P File Sharing System - Authentication Module
## Hệ thống chia sẻ file P2P - Module xác thực

Hệ thống chia sẻ file peer-to-peer với server trung tâm để quản lý xác thực và phiên đăng nhập.

## System Architecture - Kiến trúc hệ thống

- **Server**: Quản lý xác thực người dùng, phiên đăng nhập và ghi log hoạt động
- **Client**: Xử lý đăng ký, đăng nhập, đăng xuất với giao diện người dùng thân thiện

## Features - Tính năng

### Server Features - Tính năng Server
- Hệ thống đăng ký và xác thực người dùng
- Quản lý phiên đăng nhập với Client ID tự động
- Ghi log chi tiết hoạt động vào file log.txt
- Xử lý đa luồng cho nhiều client đồng thời
- Lưu trữ dữ liệu người dùng vào file users.txt

### Client Features - Tính năng Client
- Tạo Client ID tự động (lưu trong config.txt)
- Đăng ký tài khoản người dùng mới
- Đăng nhập và quản lý phiên làm việc
- Đăng xuất an toàn
- Hiển thị trạng thái người dùng hiện tại

## Quick Start - Hướng dẫn nhanh

### Prerequisites - Yêu cầu hệ thống
- **Linux**: GCC compiler, pthread library
- **Makefile**: Đã được cung cấp để build dễ dàng

### Build and Run - Biên dịch và chạy

**Linux (Recommended - Khuyến nghị):**
```bash
# Clone hoặc download project
# Mở terminal trong thư mục ProjectFinal

# Build tất cả
make all

# Hoặc build từng file riêng
make server    # Chỉ build server
make client    # Chỉ build client

# Chạy server (terminal 1)
make run-server
# Hoặc
./server

# Chạy client (terminal 2 - mở terminal mới)
make run-client  
# Hoặc
./client
```

**Manual Build (Nếu không có Makefile):**
```bash
# Build server
gcc -Wall -Wextra -std=c99 -pthread -o server server.c

# Build client  
gcc -Wall -Wextra -std=c99 -o client client.c

# Run server
./server

# Run client (in new terminal)
./client
```

### Usage Steps - Các bước sử dụng
1. **Khởi động server trước**: Chạy `./server` hoặc `make run-server`
2. **Khởi động client**: Mở terminal mới và chạy `./client` hoặc `make run-client`
3. **Nhập thông tin server**: 
   - Server IP (default: 127.0.0.1)
   - Server Port (default: 8080)
4. **Đăng ký tài khoản**: Chọn option 1, nhập username và password
5. **Đăng nhập**: Chọn option 2, nhập thông tin đăng nhập
6. **Kiểm tra trạng thái**: Chọn option 4 để xem thông tin client
7. **Đăng xuất**: Chọn option 3 khi hoàn thành
8. **Thoát**: Chọn option 0 để đóng ứng dụng

### Menu Options - Các lựa chọn menu
```
1. Register (Đăng ký)        - Tạo tài khoản mới
2. Login (Đăng nhập)         - Đăng nhập vào hệ thống  
3. Logout (Đăng xuất)        - Đăng xuất an toàn
4. User Status (Trạng thái)  - Xem thông tin client hiện tại
0. Exit (Thoát)              - Đóng ứng dụng
```

## Technical Details - Chi tiết kỹ thuật

### Message Protocol - Giao thức thông điệp
```c
// Message Structure - Cấu trúc thông điệp
typedef struct {
    int header;        // Loại thông điệp (MSG_*_REQ/RES)
    union {
        register_req_t register_req;
        register_res_t register_res;
        login_req_t login_req;
        login_res_t login_res;
        logout_req_t logout_req;
        logout_res_t logout_res;
    } payload;
} Message;
```

### Client ID System - Hệ thống Client ID
- Mỗi client tạo ID 32-bit duy nhất khi chạy lần đầu
- Lưu trữ trong file `config.txt` để quản lý phiên
- Server sử dụng để theo dõi client đang hoạt động
- Client ID được gửi trong login_req_t cùng với listen_port

### File Management - Quản lý file
- **users.txt**: Lưu trữ thông tin tài khoản (username, password)
- **config.txt**: Lưu trữ Client ID của mỗi client
- **log.txt**: Ghi log chi tiết hoạt động của server

### Logging System - Hệ thống ghi log
Server ghi log chi tiết các hoạt động:
- SERVER_START: Khởi động server
- NEW_CONNECTION: Client kết nối mới  
- REGISTER: Đăng ký tài khoản
- LOGIN: Đăng nhập thành công
- LOGOUT: Đăng xuất
- DISCONNECT: Client ngắt kết nối
- ERROR: Các lỗi xảy ra

### Example Log Entry - Ví dụ log entry
```
LOGIN: User 'admin' logged in successfully | Client ID: 1234567890 | IP: 127.0.0.1 | Port: 9090 | Time: Sun Dec  1 10:30:45 2024
```

## Development Commands - Lệnh phát triển

### Makefile Commands - Lệnh Makefile
```bash
make help          # Hiển thị trợ giúp
make all           # Build cả server và client
make server        # Build chỉ server
make client        # Build chỉ client  
make clean         # Xóa file thực thi
make run-server    # Chạy server
make run-client    # Chạy client
make show-log      # Xem log server
make clear-log     # Xóa file log
```

### Debug Mode - Chế độ debug
```bash
# Build với debug symbols
gcc -Wall -Wextra -std=c99 -g -pthread -o server server.c
gcc -Wall -Wextra -std=c99 -g -o client client.c

# Run với valgrind để kiểm tra memory
valgrind --leak-check=full ./server
valgrind --leak-check=full ./client
```

## Security & Limitations - Bảo mật và hạn chế
- Xác thực cơ bản bằng username/password
- Không mã hóa dữ liệu (chỉ dùng học tập)
- Không xác minh tính toàn vẹn dữ liệu
- Hỗ trợ đa luồng với mutex protection
- Thiết kế cho Linux (có thể port sang Windows)