#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define BUFFER_SIZE 1024
#define CONFIG_FILE "config.txt"

// Message types - Các loại thông điệp
#define MSG_REGISTER 1      // Đăng ký tài khoản
#define MSG_LOGIN 2         // Đăng nhập
#define MSG_LOGOUT 3        // Đăng xuất
#define MSG_SUCCESS 100     // Thành công
#define MSG_ERROR 101       // Lỗi

/**
 * Cấu trúc Message - Thông điệp giữa client và server
 * type: Loại thông điệp (MSG_REGISTER, MSG_LOGIN, MSG_LOGOUT)
 * client_id: ID duy nhất của client
 * username: Tên đăng nhập
 * password: Mật khẩu
 * data: Dữ liệu phản hồi từ server
 */
typedef struct {
    int type;
    uint32_t client_id;
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char data[BUFFER_SIZE];
} Message;

// Global variables - Biến toàn cục
uint32_t g_client_id = 0;           // ID duy nhất của client
char g_username[MAX_USERNAME];      // Tên đăng nhập hiện tại
int g_server_sock = -1;             // Socket kết nối đến server
int g_logged_in = 0;                // Trạng thái đăng nhập

// Function prototypes - Khai báo hàm
uint32_t generate_client_id();                        // Tạo ID ngẫu nhiên cho client
void load_config();                                   // Tải cấu hình từ file
void save_config();                                   // Lưu cấu hình vào file
int connect_to_server(const char *server_ip, int server_port); // Kết nối đến server
int register_user();                                  // Đăng ký tài khoản mới
int login_user();                                     // Đăng nhập vào hệ thống
void logout_user();                                   // Đăng xuất khỏi hệ thống
void display_menu();                                  // Hiển thị menu lựa chọn
void show_user_status();                              // Hiển thị trạng thái đăng nhập

/**
 * Hàm main - Chương trình chính của client
 */
int main() {
    printf("=== P2P File Sharing Client (Linux) ===\n");
    printf("=== Ứng dụng chia sẻ file P2P (Linux) ===\n");
    
    // Tải cấu hình client (ID) từ file
    load_config();
    
    // Kết nối đến server
    char server_ip[16];
    int server_port;
    
    printf("\nEnter server IP address (default: 127.0.0.1): ");
    printf("Nhập địa chỉ IP server (mặc định: 127.0.0.1): ");
    if (fgets(server_ip, sizeof(server_ip), stdin) == NULL || strlen(server_ip) <= 1) {
        strcpy(server_ip, "127.0.0.1");
    } else {
        server_ip[strlen(server_ip) - 1] = '\0'; // Xóa ký tự xuống dòng
    }
    
    printf("Enter server port (default: 8080): ");
    printf("Nhập port server (mặc định: 8080): ");
    char port_str[10];
    if (fgets(port_str, sizeof(port_str), stdin) == NULL || strlen(port_str) <= 1) {
        server_port = 8080;
    } else {
        server_port = atoi(port_str);
    }
    
    if (connect_to_server(server_ip, server_port) < 0) {
        printf("Failed to connect to server\\n");\n        printf("Không thể kết nối đến server\\n");\n        return 1;\n    }\n    \n    printf(\"Connected to server at %s:%d\\n\", server_ip, server_port);\n    printf(\"Đã kết nối đến server tại %s:%d\\n\", server_ip, server_port);\n    \n    int choice;\n    \n    while (1) {\n        display_menu();\n        printf(\"\\nEnter your choice (Nhập lựa chọn): \");\n        scanf(\"%d\", &choice);\n        getchar(); // consume newline\n        \n        switch (choice) {\n            case 1:\n                if (register_user() == 0) {\n                    printf(\"Registration successful! - Đăng ký thành công!\\n\");\n                } else {\n                    printf(\"Registration failed! - Đăng ký thất bại!\\n\");\n                }\n                break;\n                \n            case 2:\n                if (login_user() == 0) {\n                    printf(\"Login successful! - Đăng nhập thành công!\\n\");\n                    g_logged_in = 1;\n                } else {\n                    printf(\"Login failed! - Đăng nhập thất bại!\\n\");\n                }\n                break;\n                \n            case 3:\n                if (g_logged_in) {\n                    logout_user();\n                    g_logged_in = 0;\n                    printf(\"Logged out successfully! - Đăng xuất thành công!\\n\");\n                } else {\n                    printf(\"You are not logged in! - Bạn chưa đăng nhập!\\n\");\n                }\n                break;\n                \n            case 4:\n                show_user_status();\n                break;\n                \n            case 0:\n                if (g_logged_in) {\n                    logout_user();\n                }\n                close(g_server_sock);\n                printf(\"Goodbye! - Tạm biệt!\\n\");\n                return 0;\n                \n            default:\n                printf(\"Invalid choice! - Lựa chọn không hợp lệ!\\n\");\n        }\n    }\n    \n    return 0;\n}"}

/**
 * Tạo ID ngẫu nhiên 32-bit cho client
 * @return: ID ngẫu nhiên duy nhất
 */
uint32_t generate_client_id() {
    srand((unsigned int)time(NULL));
    return (uint32_t)rand() << 16 | (uint32_t)rand();
}

/**
 * Tải cấu hình client từ file config.txt
 * Nếu file không tồn tại, tạo ID mới và lưu vào file
 */
void load_config() {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (file == NULL) {
        // Tạo ID mới cho client
        g_client_id = generate_client_id();
        save_config();
        printf("Generated new client ID: %u\n", g_client_id);
        printf("Đã tạo ID client mới: %u\n", g_client_id);
    } else {
        fscanf(file, "%u", &g_client_id);
        fclose(file);
        printf("Loaded client ID: %u\n", g_client_id);
        printf("Đã tải ID client: %u\n", g_client_id);
    }
}

/**
 * Lưu cấu hình client vào file config.txt
 */
void save_config() {
    FILE *file = fopen(CONFIG_FILE, "w");
    if (file != NULL) {
        fprintf(file, "%u\n", g_client_id);
        fclose(file);
    }
}

/**
 * Kết nối đến server
 * @param server_ip: địa chỉ IP của server
 * @param server_port: port của server
 * @return: 0 nếu thành công, -1 nếu thất bại
 */

int connect_to_server(const char *server_ip, int server_port) {
    // Tạo socket TCP
    g_server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (g_server_sock < 0) {
        perror("Socket creation failed - Tạo socket thất bại");
        return -1;
    }
    
    // Cấu hình địa chỉ server
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    // Chuyển đổi địa chỉ IP từ string sang binary
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        printf("Invalid IP address - Địa chỉ IP không hợp lệ\n");
        close(g_server_sock);
        return -1;
    }
    
    // Kết nối đến server
    if (connect(g_server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed - Kết nối thất bại");
        close(g_server_sock);
        return -1;
    }
    
    return 0;
}

/**
 * Đăng ký tài khoản người dùng mới
 * @return: 0 nếu thành công, -1 nếu thất bại
 */
int register_user() {
    Message msg;
    memset(&msg, 0, sizeof(Message));
    msg.type = MSG_REGISTER;
    
    printf("\n=== User Registration - Đăng ký tài khoản ===\n");
    
    printf("Enter username (Nhập tên đăng nhập): ");
    fgets(msg.username, sizeof(msg.username), stdin);
    msg.username[strlen(msg.username) - 1] = '\0'; // Xóa ký tự xuống dòng
    
    printf("Enter password (Nhập mật khẩu): ");
    fgets(msg.password, sizeof(msg.password), stdin);
    msg.password[strlen(msg.password) - 1] = '\0'; // Xóa ký tự xuống dòng
    
    // Gửi yêu cầu đăng ký đến server
    if (send(g_server_sock, &msg, sizeof(Message), 0) < 0) {
        perror("Failed to send registration request");
        return -1;
    }
    
    // Nhận phản hồi từ server
    Message response;
    if (recv(g_server_sock, &response, sizeof(Message), 0) < 0) {
        perror("Failed to receive response");
        return -1;
    }
    
    if (response.type == MSG_SUCCESS) {
        printf("✓ %s\n", response.data);
        return 0;
    } else {
        printf("✗ Error: %s\n", response.data);
        return -1;
    }
}

/**
 * Đăng nhập vào hệ thống
 * @return: 0 nếu thành công, -1 nếu thất bại
 */
int login_user() {
    Message msg;
    memset(&msg, 0, sizeof(Message));
    msg.type = MSG_LOGIN;
    msg.client_id = g_client_id;
    
    printf("\n=== User Login - Đăng nhập ===\n");
    
    printf("Enter username (Nhập tên đăng nhập): ");
    fgets(msg.username, sizeof(msg.username), stdin);
    msg.username[strlen(msg.username) - 1] = '\0'; // Xóa ký tự xuống dòng
    
    printf("Enter password (Nhập mật khẩu): ");
    fgets(msg.password, sizeof(msg.password), stdin);
    msg.password[strlen(msg.password) - 1] = '\0'; // Xóa ký tự xuống dòng
    
    // Gửi yêu cầu đăng nhập đến server
    if (send(g_server_sock, &msg, sizeof(Message), 0) < 0) {
        perror("Failed to send login request");
        return -1;
    }
    
    // Nhận phản hồi từ server
    Message response;
    if (recv(g_server_sock, &response, sizeof(Message), 0) < 0) {
        perror("Failed to receive response");
        return -1;
    }
    
    if (response.type == MSG_SUCCESS) {
        strcpy(g_username, msg.username);
        printf("✓ %s\n", response.data);
        printf("Welcome %s! Client ID: %u\n", g_username, g_client_id);
        printf("Chào mừng %s! ID Client: %u\n", g_username, g_client_id);
        return 0;
    } else {
        printf("✗ Error: %s\n", response.data);
        return -1;
    }
}

/**
 * Đăng xuất khỏi hệ thống
 */
void logout_user() {
    Message msg;
    memset(&msg, 0, sizeof(Message));
    msg.type = MSG_LOGOUT;
    msg.client_id = g_client_id;
    
    // Gửi yêu cầu đăng xuất đến server
    if (send(g_server_sock, &msg, sizeof(Message), 0) < 0) {
        perror("Failed to send logout request");
        return;
    }
    
    // Nhận phản hồi từ server
    Message response;
    if (recv(g_server_sock, &response, sizeof(Message), 0) < 0) {
        perror("Failed to receive response");
        return;
    }
    
    printf("✓ %s\n", response.data);
    printf("Goodbye %s!\n", g_username);
    printf("Tạm biệt %s!\n", g_username);
    
    // Xóa thông tin đăng nhập
    memset(g_username, 0, sizeof(g_username));
}

/**
 * Hiển thị trạng thái đăng nhập hiện tại của user
 */
void show_user_status() {
    printf("\n=== User Status - Trạng thái người dùng ===\n");
    printf("Client ID: %u\n", g_client_id);
    
    if (g_logged_in) {
        printf("Status: Logged in as '%s' - Đã đăng nhập với tên '%s'\n", g_username);
        printf("✓ Connected to server - Đã kết nối đến server\n");
    } else {
        printf("Status: Not logged in - Chưa đăng nhập\n");
        printf("✗ Please login to use the system - Vui lòng đăng nhập để sử dụng\n");
    }
}

/**
 * Hiển thị menu lựa chọn chính
 */
void display_menu() {
    printf("\n" "================================\n");
    printf("=== P2P File Sharing Menu ===\n");
    printf("=== Menu Chia sẻ file P2P ===\n");
    printf("================================\n");
    printf("1. Register (Đăng ký)\n");
    printf("2. Login (Đăng nhập)\n");
    printf("3. Logout (Đăng xuất)\n");
    printf("4. User Status (Trạng thái)\n");
    printf("0. Exit (Thoát)\n");
    printf("================================\n");
}
