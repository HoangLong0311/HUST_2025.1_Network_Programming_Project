#include "auth.h"
#include "account.h"
#include "session.h"
#include "network_utils.h"

void handle_register(int sock, register_req_t *req) {
    register_res_t res;
    
    int ret = register_account(req->username, req->password);
    switch (ret) {
        case SUCCESS: res.status = STATUS_SUCCESS;
        case USERNAME_ALREADY_TAKEN: res.status = STATUS_ERR_USERNAME_ALREADY_TAKEN;
        default: res.status = STATUS_FAILURE;
    }
    // Send reply to client
    send_message(sock, MSG_REGISTER_RES, &res, sizeof(res));
}

void handle_login(int sock, login_req_t *req) {
    login_res_t res; 
    
    int result = login_account(req->username, req->password);
    
    switch (result) {
        case SUCCESS:
            if (is_user_online(req->username)) {
                res.status = STATUS_ERR_ALREADY_LOGGED_IN;
            } else {
                int session_idx = add_session(sock, req->username);

                if (session_idx == -1){
                    res.status = STATUS_FAILURE;
                } else {
                    res.status = STATUS_SUCCESS;
                }
            }
        case USER_NOT_FOUND:
            res.status = STATUS_ERR_USERNAME_NOTFOUND;
        case BAD_CREDENTIALS:
            res.status = STATUS_ERR_BAD_CREDENTIALS;
        default:
            res.status = STATUS_FAILURE;
    }

    // Send reply to client
    send_message(sock, MSG_LOGIN_RES, &res, sizeof(res));
}

void handle_logout(int sock, logout_req_t *req) {
    logout_res_t res;
    int ret;

    // Check owner first
    if (!check_session_owner(sock, req->username)) {
        res.status = STATUS_ERR_UNAUTHORIZED; 
        send_message(sock, MSG_LOGOUT_RES, &res, sizeof(res));
        return;
    }
    // Call logout 
    if ((ret = logout_account(req->username)) == SUCCESS) {
        remove_session(sock);
        res.status = STATUS_SUCCESS;
    } 
    else {
        res.status = STATUS_ERR_USERNAME_NOTFOUND;
    }
    // send reply to client
    send_message(sock, MSG_LOGOUT_RES, &res, sizeof(res));
}
