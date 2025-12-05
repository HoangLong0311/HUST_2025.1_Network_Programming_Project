#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#define MAX_CLIENTS 100
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define BUFFER_SIZE 1024
#define SERVER_PORT 8080

// Message types - Các loại thông điệp
#define MSG_REGISTER_REQ 1 // Yêu cầu đăng ký tài khoản
#define MSG_REGISTER_RES 2 // Phản hồi đăng ký
#define MSG_LOGIN_REQ 3    // Yêu cầu đăng nhập
#define MSG_LOGIN_RES 4    // Phản hồi đăng nhập
#define MSG_LOGOUT_REQ 5   // Yêu cầu đăng xuất
#define MSG_LOGOUT_RES 6   // Phản hồi đăng xuất

// Status codes - Mã trạng thái
#define STATUS_OK 1   // Thành công
#define STATUS_FAIL 0 // Thất bại

/**
 * Cấu trúc User - Lưu trữ thông tin người dùng
 * username: Tên đăng nhập
 * password: Mật khẩu
 * is_online: Trạng thái online (1) hoặc offline (0)
 * client_id: ID duy nhất của client (32-bit)
 * ip_address: Địa chỉ IP của client hiện tại
 */
typedef struct
{
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    int is_online;
    uint32_t client_id;
    char ip_address[16];
} User;

/**
 * Cấu trúc Register Request - Yêu cầu đăng ký
 */
typedef struct
{
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
} register_req_t;

/**
 * Cấu trúc Register Response - Phản hồi đăng ký
 */
typedef struct
{
    int status; // STATUS_OK hoặc STATUS_FAIL
} register_res_t;

/**
 * Cấu trúc Login Request - Yêu cầu đăng nhập
 */
typedef struct
{
    uint32_t client_id; // ID định danh từ config.txt
    int listen_port;    // Cổng để kết nối P2P
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
} login_req_t;

/**
 * Cấu trúc Login Response - Phản hồi đăng nhập
 */
typedef struct
{
    int status; // STATUS_OK hoặc STATUS_FAIL
} login_res_t;

/**
 * Cấu trúc Logout Request - Yêu cầu đăng xuất
 */
typedef struct
{
    uint32_t client_id;
} logout_req_t;

/**
 * Cấu trúc Logout Response - Phản hồi đăng xuất
 */
typedef struct
{
    int status; // STATUS_OK hoặc STATUS_FAIL
} logout_res_t;

/**
 * Cấu trúc Message - Header + Payload
 * header: Loại thông điệp (MSG_*_REQ hoặc MSG_*_RES)
 * payload: Dữ liệu tương ứng với từng loại thông điệp
 */
typedef struct
{
    int header;
    union
    {
        register_req_t register_req;
        register_res_t register_res;
        login_req_t login_req;
        login_res_t login_res;
        logout_req_t logout_req;
        logout_res_t logout_res;
    } payload;
} Message;

// Global variables - Biến toàn cục
User users[MAX_CLIENTS];                                 // Mảng lưu trữ thông tin người dùng
int user_count = 0;                                      // Số lượng người dùng hiện tại
pthread_mutex_t users_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex bảo vệ dữ liệu users

// Function prototypes - Khai báo hàm
void *handle_client(void *arg);                                    // Xử lý client kết nối
int authenticate_user(const char *username, const char *password); // Xác thực người dùng
int register_user(const char *username, const char *password);     // Đăng ký người dùng mới
int find_user_by_username(const char *username);                   // Tìm user theo tên
int find_user_by_id(uint32_t client_id);                           // Tìm user theo ID
void load_users();                                                 // Tải danh sách user từ file
void save_users();                                                 // Lưu danh sách user vào file
void write_log(const char *message);                               // Ghi log hoạt động chi tiết

/**
 * Hàm main - Khởi tạo và chạy server
 */
int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread_id;

    printf("=== P2P File Sharing Server (Linux) ===\n");

    // Khởi tạo mutex bảo vệ dữ liệu users
    pthread_mutex_init(&users_mutex, NULL);

    // Tải danh sách người dùng từ file (nếu có)
    load_users();

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Setsockopt failed");
        exit(1);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        exit(1);
    }

    // Listen for connections
    if (listen(server_fd, 10) < 0)
    {
        perror("Listen failed");
        exit(1);
    }

    printf("File Sharing Server started on port %d\n", SERVER_PORT);

    // Ghi log khởi động server
    char start_log[512];
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    sprintf(start_log, "SERVER_START: P2P File Sharing Server started | Port: %d | Time: %s", SERVER_PORT, time_str);
    write_log(start_log);

    while (1)
    {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("New client connected from %s\n", inet_ntoa(client_addr.sin_addr));

        // Ghi log kết nối mới
        char connect_log[512];
        time_t now = time(NULL);
        char *time_str = ctime(&now);
        time_str[strlen(time_str) - 1] = '\0';
        sprintf(connect_log, "NEW_CONNECTION: Client connected | IP: %s | Time: %s",
                inet_ntoa(client_addr.sin_addr), time_str);
        write_log(connect_log);

        // Create thread to handle client
        int *client_socket = malloc(sizeof(int));
        *client_socket = client_fd;

        if (pthread_create(&thread_id, NULL, handle_client, client_socket) != 0)
        {
            perror("Thread creation failed");
            close(client_fd);
            free(client_socket);
        }

        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}

/**
 * Hàm xử lý client kết nối - chạy trong thread riêng
 * @param arg: con trỏ đến socket descriptor của client
 * @return NULL khi client ngắt kết nối
 */
void *handle_client(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);

    Message msg;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Lấy địa chỉ IP của client
    getpeername(client_fd, (struct sockaddr *)&client_addr, &addr_len);
    char client_ip[16];
    strcpy(client_ip, inet_ntoa(client_addr.sin_addr));

    while (1)
    {
        // Nhận thông điệp từ client
        int bytes_received = recv(client_fd, &msg, sizeof(Message), 0);
        if (bytes_received <= 0)
        {
            printf("Client %s disconnected\n", client_ip);

            // Ghi log ngắt kết nối
            char log_msg[512];
            time_t now = time(NULL);
            char *time_str = ctime(&now);
            time_str[strlen(time_str) - 1] = '\0';
            sprintf(log_msg, "DISCONNECT: Client disconnected | IP: %s | Time: %s", client_ip, time_str);
            write_log(log_msg);
            break;
        }

        Message response;
        memset(&response, 0, sizeof(Message));

        // Xử lý các loại thông điệp
        switch (msg.header)
        {
        case MSG_REGISTER_REQ:
        {
            printf("Registration request from %s (IP: %s)\n", msg.payload.register_req.username, client_ip);

            response.header = MSG_REGISTER_RES;
            if (register_user(msg.payload.register_req.username, msg.payload.register_req.password))
            {
                response.payload.register_res.status = STATUS_OK;

                // Ghi log đăng ký chi tiết
                char log_msg[512];
                time_t now = time(NULL);
                char *time_str = ctime(&now);
                time_str[strlen(time_str) - 1] = '\0';
                sprintf(log_msg, "REGISTER: New user '%s' registered successfully | IP: %s | Time: %s | Total users: %d",
                        msg.payload.register_req.username, client_ip, time_str, user_count);
                write_log(log_msg);
            }
            else
            {
                response.payload.register_res.status = STATUS_FAIL;
            }
            break;
        }

        case MSG_LOGIN_REQ:
        {
            printf("Login request from %s (IP: %s, Client ID: %u, Port: %d)\n",
                   msg.payload.login_req.username, client_ip,
                   msg.payload.login_req.client_id, msg.payload.login_req.listen_port);

            response.header = MSG_LOGIN_RES;
            int user_idx = authenticate_user(msg.payload.login_req.username, msg.payload.login_req.password);
            if (user_idx >= 0)
            {
                // Cập nhật thông tin phiên đăng nhập
                pthread_mutex_lock(&users_mutex);
                users[user_idx].is_online = 1;
                users[user_idx].client_id = msg.payload.login_req.client_id;
                strcpy(users[user_idx].ip_address, client_ip);
                pthread_mutex_unlock(&users_mutex);

                response.payload.login_res.status = STATUS_OK;

                // Ghi log đăng nhập chi tiết
                char log_msg[512];
                time_t now = time(NULL);
                char *time_str = ctime(&now);
                time_str[strlen(time_str) - 1] = '\0';
                sprintf(log_msg, "LOGIN: User '%s' logged in successfully | Client ID: %u | IP: %s | Port: %d | Time: %s",
                        msg.payload.login_req.username, msg.payload.login_req.client_id,
                        client_ip, msg.payload.login_req.listen_port, time_str);
                write_log(log_msg);
            }
            else
            {
                response.payload.login_res.status = STATUS_FAIL;
            }
            break;
        }

        case MSG_LOGOUT_REQ:
        {
            printf("Logout request from client ID %u (IP: %s)\n", msg.payload.logout_req.client_id, client_ip);

            response.header = MSG_LOGOUT_RES;
            pthread_mutex_lock(&users_mutex);
            int user_idx = find_user_by_id(msg.payload.logout_req.client_id);
            if (user_idx >= 0)
            {
                users[user_idx].is_online = 0;
                response.payload.logout_res.status = STATUS_OK;

                // Ghi log đăng xuất chi tiết
                char log_msg[512];
                time_t now = time(NULL);
                char *time_str = ctime(&now);
                time_str[strlen(time_str) - 1] = '\0';
                sprintf(log_msg, "LOGOUT: User '%s' logged out | Client ID: %u | IP: %s | Time: %s",
                        users[user_idx].username, msg.payload.logout_req.client_id, client_ip, time_str);
                write_log(log_msg);
            }
            else
            {
                response.payload.logout_res.status = STATUS_FAIL;
            }
            pthread_mutex_unlock(&users_mutex);
            break;
        }

        default:
            printf("Unknown message header %d from client %s\n", msg.header, client_ip);

            // Ghi log lỗi
            char error_log[512];
            time_t now = time(NULL);
            char *time_str = ctime(&now);
            time_str[strlen(time_str) - 1] = '\0';
            sprintf(error_log, "ERROR: Unknown message header %d | IP: %s | Time: %s",
                    msg.header, client_ip, time_str);
            write_log(error_log);
            continue; // Bỏ qua thông điệp không hợp lệ
        }

        // Gửi phản hồi về client
        send(client_fd, &response, sizeof(Message), 0);
    }

    close(client_fd);
    return NULL;
}

/**
 * Xác thực người dùng - kiểm tra username và password
 * @param username: tên đăng nhập
 * @param password: mật khẩu
 * @return: chỉ số của user trong mảng users nếu hợp lệ, -1 nếu không hợp lệ
 */
int authenticate_user(const char *username, const char *password)
{
    pthread_mutex_lock(&users_mutex);
    for (int i = 0; i < user_count; i++)
    {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0)
        {
            pthread_mutex_unlock(&users_mutex);
            return i;
        }
    }
    pthread_mutex_unlock(&users_mutex);
    return -1;
}

/**
 * Đăng ký người dùng mới
 * @param username: tên đăng nhập mới
 * @param password: mật khẩu mới
 * @return: 1 nếu thành công, 0 nếu thất bại
 */
int register_user(const char *username, const char *password)
{
    pthread_mutex_lock(&users_mutex);

    // Kiểm tra xem username đã tồn tại chưa
    for (int i = 0; i < user_count; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            pthread_mutex_unlock(&users_mutex);
            return 0; // Username đã tồn tại
        }
    }

    // Thêm user mới
    if (user_count < MAX_CLIENTS)
    {
        strcpy(users[user_count].username, username);
        strcpy(users[user_count].password, password);
        users[user_count].is_online = 0;
        users[user_count].client_id = 0;
        strcpy(users[user_count].ip_address, "");
        user_count++;

        // Lưu danh sách users vào file
        save_users();
        pthread_mutex_unlock(&users_mutex);
        return 1; // Thành công
    }

    pthread_mutex_unlock(&users_mutex);
    return 0; // Thất bại - quá nhiều user
}

/**
 * Tìm user theo tên đăng nhập
 * @param username: tên đăng nhập cần tìm
 * @return: chỉ số của user trong mảng, -1 nếu không tìm thấy
 */
int find_user_by_username(const char *username)
{
    for (int i = 0; i < user_count; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            return i;
        }
    }
    return -1;
}

/**
 * Tìm user theo client ID (chỉ user đang online)
 * @param client_id: ID của client cần tìm
 * @return: chỉ số của user trong mảng, -1 nếu không tìm thấy
 */
int find_user_by_id(uint32_t client_id)
{
    for (int i = 0; i < user_count; i++)
    {
        if (users[i].client_id == client_id && users[i].is_online)
        {
            return i;
        }
    }
    return -1;
}

/**
 * Tải danh sách người dùng từ file users.txt
 */

void load_users()
{
    FILE *file = fopen("users.txt", "r");
    if (file == NULL)
    {
        printf("No existing user database found. Starting with empty database.\n");
        printf("Không tìm thấy cơ sở dữ liệu người dùng. Bắt đầu với database rỗng.\n");
        return;
    }

    user_count = 0;
    while (user_count < MAX_CLIENTS &&
           fscanf(file, "%s %s\n", users[user_count].username, users[user_count].password) == 2)
    {
        users[user_count].is_online = 0;
        users[user_count].client_id = 0;
        strcpy(users[user_count].ip_address, "");
        user_count++;
    }

    fclose(file);
    printf("Loaded %d users from database\n", user_count);
    printf("Đã tải %d người dùng từ cơ sở dữ liệu\n", user_count);
}

/**
 * Lưu danh sách người dùng vào file users.txt
 */
void save_users()
{
    FILE *file = fopen("users.txt", "w");
    if (file == NULL)
    {
        perror("Cannot save users database - Không thể lưu cơ sở dữ liệu người dùng");
        return;
    }

    for (int i = 0; i < user_count; i++)
    {
        fprintf(file, "%s %s\n", users[i].username, users[i].password);
    }

    fclose(file);
    printf("Saved %d users to database\n", user_count);
}

/**
 * Ghi log hoạt động chi tiết vào file log.txt
 * @param message: thông điệp cần ghi vào log
 */
void write_log(const char *message)
{
    FILE *log_file = fopen("log.txt", "a");
    if (log_file != NULL)
    {
        // Ghi log với format đầy đủ thông tin
        fprintf(log_file, "%s\n", message);
        fclose(log_file);

        // Cũng in ra console để theo dõi
        printf("LOG: %s\n", message);
    }
    else
    {
        perror("Cannot write to log file - Không thể ghi vào file log");
    }
}