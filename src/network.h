#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "account_manager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MSG_SIZE 256

//* Tín hiệu điều khiển
// chung

// server
#define ACC_BLOCKED 0
#define ACC_ACTIVATED 1
#define ACC_IDLE 2

#define REGISTER_SUCC 100
#define ACTIVATE_SUCC 101
#define LOGIN_SUCC 102
#define SIGNED_IN_ACC 103
#define RECV_SUCC 104
#define CHANGE_PASSWORD_SUCC 108
#define CORRECT_PASSWORD 109

// client
#define REGISTER_REQ 200
#define ACTIVATE_REQ 201
#define LOGIN_REQ 202
#define QUIT_REQ 205

#define SEARCH_REQ 203
#define CHANGE_PASS_REQ 204

#define MSG_SENT_SUCC 206
#define END_CHAT 207

#define SHOW_USER 208
#define PRIVATE_CHAT 209
#define CHAT_ALL 210

#define GROUP_CHAT_INIT 211
#define SHOW_GROUP 212
#define NEW_GROUP 213
#define MSG_MAKE_GROUP_SUCC 214
#define JOIN_GROUP 215
#define JOIN_GROUP_SUCC 216
#define HANDLE_GROUP_MESS 217
#define INVITE_FRIEND 218
#define INVITE_FRIEND_SUCC 219
#define GROUP_CHAT 220
#define GROUP_INFO 221
#define SHOW_GROUP_NAME 222
#define SHOW_GROUP_MEM 223
#define LEAVE_GROUP 224
#define LEAVE_GROUP_SUCC 225

#define SEARCH_USERS 226
#define CHANGE_PASSWORD 227
#define LOG_OUT 228

//* Cấu trúc gói tin
typedef struct Package_
{
    char msg[MSG_SIZE];           /* nội dung thông điệp */
    char sender[USERNAME_SIZE];   /* username người gửi */
    char receiver[USERNAME_SIZE]; /* username người nhận */
    int group_id;                 /*id group muốn gửi*/
    int ctrl_signal;              /* mã lệnh */
} Package;
#endif