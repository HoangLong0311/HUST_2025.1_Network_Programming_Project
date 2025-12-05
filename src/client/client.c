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
#define MSG_REGISTER_REQ 1   // Yêu cầu đăng ký tài khoản
#define MSG_REGISTER_RES 2   // Phản hồi đăng ký
#define MSG_LOGIN_REQ 3      // Yêu cầu đăng nhập
#define MSG_LOGIN_RES 4      // Phản hồi đăng nhập
#define MSG_LOGOUT_REQ 5     // Yêu cầu đăng xuất
#define MSG_LOGOUT_RES 6     // Phản hồi đăng xuất

// Status codes - Mã trạng thái
#define STATUS_OK 1          // Thành công
#define STATUS_FAIL 0        // Thất bại

// Default listen port for P2P
#define DEFAULT_LISTEN_PORT 9090

/**
 * Cấu trúc Register Request - Yêu cầu đăng ký
 */
typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
} register_req_t;

/**
 * Cấu trúc Register Response - Phản hồi đăng ký
 */
typedef struct {
    int status;  // STATUS_OK hoặc STATUS_FAIL
} register_res_t;

/**
 * Cấu trúc Login Request - Yêu cầu đăng nhập
 */
typedef struct {
    uint32_t client_id;     // ID định danh từ config.txt
    int listen_port;        // Cổng để kết nối P2P
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
} login_req_t;

/**
 * Cấu trúc Login Response - Phản hồi đăng nhập
 */
typedef struct {
    int status;  // STATUS_OK hoặc STATUS_FAIL
} login_res_t;

/**
 * Cấu trúc Logout Request - Yêu cầu đăng xuất
 */
typedef struct {
    uint32_t client_id;
} logout_req_t;

/**
 * Cấu trúc Logout Response - Phản hồi đăng xuất
 */
typedef struct {
    int status;  // STATUS_OK hoặc STATUS_FAIL
} logout_res_t;

/**
 * Cấu trúc Message - Header + Payload
 * header: Loại thông điệp (MSG_*_REQ hoặc MSG_*_RES)
 * payload: Dữ liệu tương ứng với từng loại thông điệp
 */
typedef struct {
    int header;
    union {
        register_req_t register_req;
        register_res_t register_res;
        login_req_t login_req;
        login_res_t login_res;
        logout_req_t logout_req;
        logout_res_t logout_res;
    } payload;
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
        printf("Failed to connect to server\n");
        printf("Không thể kết nối đến server\n");
        return 1;
    }
    
    printf("Connected to server at %s:%d\n", server_ip, server_port);
    printf("Đã kết nối đến server tại %s:%d\n", server_ip, server_port);
    
    int choice;
    
    while (1) {
        display_menu();
        printf("\nEnter your choice (Nhập lựa chọn): ");
        scanf("%d", &choice);
        getchar(); // consume newline
        
        switch (choice) {
            case 1:
                if (register_user() == 0) {
                    printf("Registration successful! - Đăng ký thành công!\n");
                } else {
                    printf("Registration failed! - Đăng ký thất bại!\n");
                }
                break;
                
            case 2:
                if (login_user() == 0) {
                    printf("Login successful! - Đăng nhập thành công!\n");
                    g_logged_in = 1;
                } else {
                    printf("Login failed! - Đăng nhập thất bại!\n");
                }
                break;
                
            case 3:
                if (g_logged_in) {
                    logout_user();
                    g_logged_in = 0;
                    printf("Logged out successfully! - Đăng xuất thành công!\n");
                } else {
                    printf("You are not logged in! - Bạn chưa đăng nhập!\n");
                }
                break;
                
            case 4:
                show_user_status();
                break;
                
            case 0:
                if (g_logged_in) {
                    logout_user();
                }
                close(g_server_sock);
                printf("Goodbye! - Tạm biệt!\n");
                return 0;
                
            default:
                printf("Invalid choice! - Lựa chọn không hợp lệ!\n");
        }
    }
    
    return 0;
}

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
    msg.header = MSG_REGISTER_REQ;
    
    printf("\n=== User Registration - Đăng ký tài khoản ===\n");
    
    printf("Enter username (Nhập tên đăng nhập): ");
    fgets(msg.payload.register_req.username, sizeof(msg.payload.register_req.username), stdin);
    msg.payload.register_req.username[strlen(msg.payload.register_req.username) - 1] = '\0';
    
    printf("Enter password (Nhập mật khẩu): ");
    fgets(msg.payload.register_req.password, sizeof(msg.payload.register_req.password), stdin);
    msg.payload.register_req.password[strlen(msg.payload.register_req.password) - 1] = '\0';
    
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
    
    if (response.header == MSG_REGISTER_RES && response.payload.register_res.status == STATUS_OK) {
        printf("✓ Registration successful - Đăng ký thành công\n");
        return 0;
    } else {
        printf("✗ Registration failed - Đăng ký thất bại\n");
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
    msg.header = MSG_LOGIN_REQ;
    msg.payload.login_req.client_id = g_client_id;
    msg.payload.login_req.listen_port = DEFAULT_LISTEN_PORT;
    
    printf("\n=== User Login - Đăng nhập ===\n");
    
    printf("Enter username (Nhập tên đăng nhập): ");
    fgets(msg.payload.login_req.username, sizeof(msg.payload.login_req.username), stdin);
    msg.payload.login_req.username[strlen(msg.payload.login_req.username) - 1] = '\0';
    
    printf("Enter password (Nhập mật khẩu): ");
    fgets(msg.payload.login_req.password, sizeof(msg.payload.login_req.password), stdin);
    msg.payload.login_req.password[strlen(msg.payload.login_req.password) - 1] = '\0';
    
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
    
    if (response.header == MSG_LOGIN_RES && response.payload.login_res.status == STATUS_OK) {
        strcpy(g_username, msg.payload.login_req.username);
        printf("✓ Login successful - Đăng nhập thành công\n");
        printf("Welcome %s! Client ID: %u | Listen Port: %d\n", g_username, g_client_id, DEFAULT_LISTEN_PORT);
        printf("Chào mừng %s! ID Client: %u | Port lắng nghe: %d\n", g_username, g_client_id, DEFAULT_LISTEN_PORT);
        return 0;
    } else {
        printf("✗ Login failed - Invalid username or password\n");
        printf("✗ Đăng nhập thất bại - Sai tên đăng nhập hoặc mật khẩu\n");
        return -1;
    }
}

/**
 * Đăng xuất khỏi hệ thống
 */
void logout_user() {
    Message msg;
    memset(&msg, 0, sizeof(Message));
    msg.header = MSG_LOGOUT_REQ;
    msg.payload.logout_req.client_id = g_client_id;
    
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
    
    if (response.header == MSG_LOGOUT_RES && response.payload.logout_res.status == STATUS_OK) {
        printf("✓ Logout successful - Đăng xuất thành công\n");
    } else {
        printf("✗ Logout failed - Đăng xuất thất bại\n");
    }
    
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
    printf("P2P Listen Port: %d\n", DEFAULT_LISTEN_PORT);
    
    if (g_logged_in) {
        printf("Status: Logged in as '%s' - Đã đăng nhập với tên '%s'\n", g_username, g_username);
        printf("✓ Connected to server - Đã kết nối đến server\n");
        printf("✓ Ready for P2P connections - Sẵn sàng kết nối P2P\n");
    } else {
        printf("Status: Not logged in - Chưa đăng nhập\n");
        printf("✗ Please login to use the system - Vui lòng đăng nhập để sử dụng\n");
    }
}

/**
 * Hiển thị menu lựa chọn chính
 */
void display_menu() {
    printf("1. Register (Đăng ký)\n");
    printf("2. Login (Đăng nhập)\n");
    printf("3. Logout (Đăng xuất)\n");
    printf("4. User Status (Trạng thái)\n");
    printf("0. Exit (Thoát)\n");
}
