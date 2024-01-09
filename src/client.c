#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

char my_username[USERNAME_SIZE];
char curr_group_name[GROUP_NAME_SIZE];
int curr_group_id = -1;
int join_succ = 0;
int my_socket;

int connect_to_server()
{
    int client_socket;
    struct sockaddr_in server_addr;

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        report_err(ERR_SOCKET_INIT);
        exit(0);
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        report_err(ERR_CONNECT_TO_SERVER);
        exit(0);
    }

    return client_socket;
}
// fix
void login_menu()
{
    printf("\n------ USER MANAGEMENT PROGRAM ------\n");
    printf("1. Register\n");
    printf("2. Activate\n");
    printf("3. Sign in\n");
    printf("4. Exit\n");
}

void user_menu()
{
    printf("\n****** Chat Room ******\n");
    printf("1. Private chat\n");
    printf("2. Group chat\n");
    printf("3. Chat all\n");
    printf("4. Show online users\n");
    printf("5. Search users\n");
    printf("6. Change password\n");
    printf("7. Sign out\n");
}
// fix
//   nhom chat menu 17/01/2023
void group_chat_menu()
{
    printf("\n****** Group chat ******\n");
    printf("1. Show my group\n");
    printf("2. Make new group\n");
    printf("3. Join group\n");
    printf("4. Return main menu\n");
}
// fix
void sub_group_chat_menu(char *group_name)
{
    printf("\n****** %s ******\n", group_name);
    printf("1. Invite your friends\n");
    printf("2. Chat \n");
    printf("3. Show group infomation \n");
    printf("4. Leave the group chat\n");
    printf("5. Return group chat menu\n");
}

void ask_server(int client_socket)
{
    int choice, result;
    Package pkg;

    while (1)
    {
        login_menu();
        printf("Your choice: "); // fix
        scanf("%d", &choice);    // fix
        clear_stdin_buff();      // fix

        switch (choice)
        {
        case 1:
        {
            pkg.ctrl_signal = REGISTER_REQ;
            send(client_socket, &pkg, sizeof(pkg), 0);
            result = signup(client_socket);
            if (result == REGISTER_SUCC)
            {
                printf("Successful registration\n"); // fix
            }
            else
            {
                printf("Account existed\n"); // fix
            }
            break;
        }
        case 2:
        {
            pkg.ctrl_signal = ACTIVATE_REQ;
            send(client_socket, &pkg, sizeof(pkg), 0);
            result = activate(client_socket);
            if (result == ACTIVATE_SUCC)
            {
                printf("Successful activation\n"); // fix
            }
            else if (result == ACC_BLOCKED)
            {
                printf("This account is blocked\n"); // fix
            }
            else if (result == ACC_ACTIVATED)
            {
                printf("This account is activated\n"); // fix
            }
            else if (result == ERR_INCORRECT_ACC)
            {
                printf("This account isn't existed\n"); // fix
            }
            else
                printf("Activation fail\n"); // fix
            break;
        }
        case 3:
            pkg.ctrl_signal = LOGIN_REQ;
            send(client_socket, &pkg, sizeof(pkg), 0);
            result = login(client_socket);
            if (result == LOGIN_SUCC)
            {
                user_use(client_socket);
            }
            else if (result == ACC_BLOCKED)
            {
                printf("This account is blocked\n"); // fix
            }
            else if (result == ACC_IDLE)
            {
                printf("This account isn't activated\n"); // fix
            }
            else if (result == ERR_INCORRECT_ACC)
            {
                // report_err(ERR_INCORRECT_ACC);
                printf("Incorrect account\n"); // fix
            }
            else
            {
                // report_err(ERR_SIGNED_IN_ACC);
                printf("Already signed in account\n"); // fix
            }
            break;
        case 4:
            pkg.ctrl_signal = QUIT_REQ;
            send(client_socket, &pkg, sizeof(pkg), 0);
            exit(0);
        }
    }
}
int signup(int client_socket)
{
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    Package pkg;

    printf("Username: ");  // fix
    scanf("%s", username); // fix
    clear_stdin_buff();    // fix

    printf("Password: ");  // fix
    scanf("%s", password); // fix
    clear_stdin_buff();    // fix

    strcpy(pkg.msg, username);
    send(client_socket, &pkg, sizeof(pkg), 0);

    recv(client_socket, &pkg, sizeof(pkg), 0);

    strcpy(pkg.msg, password);
    send(client_socket, &pkg, sizeof(pkg), 0);

    recv(client_socket, &pkg, sizeof(pkg), 0);

    return pkg.ctrl_signal;
}

int activate(int client_socket)
{
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    char capcha[10];
    Package pkg;

    printf("Username: ");  // fix
    scanf("%s", username); // fix
    clear_stdin_buff();    // fix

    printf("Password: ");  // fix
    scanf("%s", password); // fix
    clear_stdin_buff();    // fix

    strcpy(pkg.msg, username);
    send(client_socket, &pkg, sizeof(pkg), 0);

    recv(client_socket, &pkg, sizeof(pkg), 0);

    strcpy(pkg.msg, password);
    send(client_socket, &pkg, sizeof(pkg), 0);

    recv(client_socket, &pkg, sizeof(pkg), 0);

    if (pkg.ctrl_signal == ACC_BLOCKED || pkg.ctrl_signal == ACC_ACTIVATED || pkg.ctrl_signal == ERR_INCORRECT_ACC)
    {
        // printf("%s was %d\n", username, pkg.ctrl_signal);
        return pkg.ctrl_signal;
    }
    printf("Activate code: %s\n", pkg.msg); // fix
    printf("Input Activate code: ");        // fix
    scanf("%s", capcha);                    // fix
    clear_stdin_buff();                     // fix

    strcpy(pkg.msg, capcha);
    send(client_socket, &pkg, sizeof(pkg), 0);

    recv(client_socket, &pkg, sizeof(pkg), 0);

    return pkg.ctrl_signal;
}

int login(int client_socket)
{

    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    Package pkg;

    printf("Username: ");  // fix
    scanf("%s", username); // fix
    clear_stdin_buff();    // fix

    printf("Password: ");  // fix
    scanf("%s", password); // fix
    clear_stdin_buff();    // fix

    strcpy(pkg.msg, username);
    send(client_socket, &pkg, sizeof(pkg), 0);

    recv(client_socket, &pkg, sizeof(pkg), 0);

    strcpy(pkg.msg, password);
    send(client_socket, &pkg, sizeof(pkg), 0);

    recv(client_socket, &pkg, sizeof(pkg), 0);
    if (pkg.ctrl_signal == LOGIN_SUCC)
        strcpy(my_username, username);
    return pkg.ctrl_signal;
}

void user_use(int client_socket)
{
    printf("Login successfully\n"); // fix
    int login = 1;
    int choice, result;
    Package pkg;

    pthread_t read_st;
    if (pthread_create(&read_st, NULL, read_msg, (void *)&client_socket) < 0)
    {
        report_err(ERR_CREATE_THREAD);
        exit(0);
    }
    // pthread_detach(read_st);
    see_active_user(client_socket);

    while (login)
    {
        user_menu();
        printf("Your choice: \n"); // fix
        scanf("%d", &choice);      // fix
        clear_stdin_buff();        // fix

        switch (choice)
        {
        case 1: // private chat
            private_chat(client_socket);
            break;
        case 2: // group chat
        {
            group_chat_init(client_socket);
            break;
        }
        case 3:
            chat_all(client_socket);
            break;
        case 4: // show online users
            see_active_user(client_socket);
            break;

        case 5: // search
        {
            search_users(client_socket);
            break;
        }
        case 6: // change password
        {
            change_password(client_socket);
            break;
        }
        case 7: // sign out
            login = 0;
            pkg.ctrl_signal = LOG_OUT;
            send(client_socket, &pkg, sizeof(pkg), 0);
            strcpy(my_username, "");
            break;
        }
        if (login == 0)
            break;
    }
    pthread_detach(read_st);
}

void search_users(int client_socket)
{
    char username[USERNAME_SIZE];
    Package pkg;

    pkg.ctrl_signal = SEARCH_USERS;
    send(client_socket, &pkg, sizeof(pkg), 0);
    printf("Input username: "); // fix
    scanf("%s", username);      // fix
    clear_stdin_buff();         // fix

    strcpy(pkg.msg, username);
    send(client_socket, &pkg, sizeof(pkg), 0);

    recv(client_socket, &pkg, sizeof(pkg), 0);
    switch (pkg.ctrl_signal)
    {
    case ACC_BLOCKED:
        printf("This account is blocked\n"); // fix
        break;
    case ACC_IDLE:
        printf("This account is active\n"); // fix
        break;
    case ACC_ACTIVATED:
        printf("This account needs activation\n"); // fix
        break;
    default:
        break;
    }
}

void change_password(int client_socket)
{
    char password[PASSWORD_SIZE];
    char new_password[PASSWORD_SIZE];
    Package pkg;

    pkg.ctrl_signal = CHANGE_PASSWORD;
    strcpy(pkg.sender, my_username);
    send(client_socket, &pkg, sizeof(pkg), 0);
    // while (1)
    // {
    printf("Password: ");  // fix
    scanf("%s", password); // fix
    clear_stdin_buff();    // fix

    strcpy(pkg.msg, password);
    send(client_socket, &pkg, sizeof(pkg), 0);
    // printf(" %d Sent\n", res);

    recv(client_socket, &pkg, sizeof(pkg), 0);
    if (pkg.ctrl_signal != CORRECT_PASSWORD)
    {
        // printf("Password correct\n");
        printf("Wrong password. Please input your password.\n"); // fix
        sleep(1);
        return;
    }
    // }
    printf("Input new password: "); // fix
    scanf("%s", new_password);      // fix
    clear_stdin_buff();             // fix

    strcpy(pkg.msg, new_password);
    send(client_socket, &pkg, sizeof(pkg), 0);

    recv(client_socket, &pkg, sizeof(pkg), 0);
    if (pkg.ctrl_signal == CHANGE_PASSWORD_SUCC)
    {
        printf("Password changed successfully\n"); // fix
    }
    else
    {
        printf("Password and new password are the same\n"); // fix
    }
}

void *read_msg(void *param)
{
    int *c_socket = (int *)param;
    int client_socket = *c_socket;
    // printf("\nmysoc: %d\n", client_socket);
    // int client_socket = my_socket;
    Package pkg;
    while (1)
    {
        recv(client_socket, &pkg, sizeof(pkg), 0);
        // printf("receive %d from server\n", pkg.ctrl_signal);
        switch (pkg.ctrl_signal)
        {
            // case REGISTER_SUCC:
            //     printf(" Successful Registration\n");
            //     break;

            // case ACC_EXISTED:
            //     printf("This account existed!!!\n");
            //     break;

        case ACTIVATE_SUCC:
            printf("Successfull activation\n"); // fix
            break;

        case ERR_ACTIVATE_FAIL:
            printf("Activation failed\n"); // fix

        case SHOW_USER:
            printf("Current online users: %s \n", pkg.msg); // fix
            break;

        case PRIVATE_CHAT:
            printf("%s: %s\n", pkg.sender, pkg.msg); // fix
            break;

        case GROUP_CHAT_INIT:
            printf("%s\n", pkg.msg); // fix
            break;
        case ERR_INVALID_RECEIVER:
            report_err(ERR_INVALID_RECEIVER); // fix
            break;
        case CHAT_ALL:
            printf("%s to all: %s\n", pkg.sender, pkg.msg); // fix
            break;
        case MSG_SENT_SUCC:
            printf("Message sent!\n"); // fix
            break;
        case SHOW_GROUP:
            printf("Your group: \n%s \n", pkg.msg); // fix
            break;

        case MSG_MAKE_GROUP_SUCC:
            printf("Your new group: %s \n", pkg.msg); // fix
            break;
        case JOIN_GROUP_SUCC:
            printf("Current group: %s \n", pkg.msg); // fix
            strcpy(curr_group_name, pkg.msg);
            curr_group_id = pkg.group_id;
            join_succ = 1;
            break;
        case INVITE_FRIEND:
            printf("Attention: %s \n", pkg.msg); // fix
            break;
        case ERR_GROUP_NOT_FOUND:
            report_err(ERR_GROUP_NOT_FOUND); // fix
            break;
        case ERR_IVITE_MYSELF:
            report_err(ERR_IVITE_MYSELF); // fix
            break;
        case ERR_USER_NOT_FOUND:
            report_err(ERR_USER_NOT_FOUND); // fix
            break;
        case ERR_FULL_MEM:
            report_err(ERR_FULL_MEM); // fix
            break;
        case INVITE_FRIEND_SUCC:
            printf("%s\n", pkg.msg); // fix
            break;
        case GROUP_CHAT:
            if (curr_group_id == pkg.group_id)
            {
                printf("%s: %s\n", pkg.sender, pkg.msg); // fix
            }
            else
            {
                printf("%s sent to Group_%d: %s\n", pkg.sender, pkg.group_id, pkg.msg); // fix
            }
            break;
        case SHOW_GROUP_NAME:
            printf("GROUP NAME: %s\n", pkg.msg); // fix
            break;
        case SHOW_GROUP_MEM:
            printf("%s\n", pkg.msg); // fix
            break;
        case LEAVE_GROUP_SUCC:
            printf("%s\n", pkg.msg); // fix
            break;
        default:
            break;
        }
    }
}
// -> oke
void see_active_user(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = SHOW_USER;
    send(client_socket, &pkg, sizeof(pkg), 0);

    sleep(1);

    // recv(client_socket, &pkg, sizeof(pkg), 0);
}

void private_chat(int client_socket)
{
    Package pkg;
    char username[USERNAME_SIZE];
    char msg[MSG_SIZE];
    pkg.ctrl_signal = PRIVATE_CHAT;
    // send(client_socket, &pkg, sizeof(pkg), 0);

    printf("Receiver: \n");                // fix
    fgets(username, USERNAME_SIZE, stdin); // fix
    username[strlen(username) - 1] = '\0';
    strcpy(pkg.receiver, username);
    strcpy(pkg.sender, my_username);
    // send(client_socket, &pkg, sizeof(pkg), 0);

    while (1)
    {
        printf("Message(leave blank to exit private chat): \n"); // fix
        fgets(msg, MSG_SIZE, stdin);                             // fix
        msg[strlen(msg) - 1] = '\0';
        if (strlen(msg) == 0)
        {
            break;
        }

        strcpy(pkg.msg, msg);
        send(client_socket, &pkg, sizeof(pkg), 0);

        sleep(1);
    }
}

void chat_all(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = CHAT_ALL;
    strcpy(pkg.sender, my_username);
    char msg[MSG_SIZE];
    while (1)
    {
        printf("Message(leave blank to exit group chat): \n"); // fix
        fgets(msg, MSG_SIZE, stdin);                           // fix
        msg[strlen(msg) - 1] = '\0';
        if (strlen(msg) == 0)
        {
            break;
        }

        strcpy(pkg.msg, msg);
        send(client_socket, &pkg, sizeof(pkg), 0);

        // sleep(1);
    }
}
// 17/01/2023
//  xu ly lua chon trong group chat menu
void group_chat_init(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = GROUP_CHAT_INIT;
    send(client_socket, &pkg, sizeof(pkg), 0);
    // xu ly
    int choice = 0;

    while (1)
    {
        sleep(1);

        group_chat_menu();
        printf("Your choice: \n"); // fix
        scanf("%d", &choice);      // fix
        clear_stdin_buff();        // fix

        switch (choice)
        {
        case 1:
            show_group(client_socket);
            break;
        case 2:
            new_group(client_socket);
            break;
        case 3:
            join_group(client_socket);
            break;
        default:
            return;
        }
    }
}

// hien thi nhom hien tai
void show_group(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = SHOW_GROUP;
    send(client_socket, &pkg, sizeof(pkg), 0);
    // sleep(1);
}

// tao group moi -> oke
void new_group(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = NEW_GROUP;
    send(client_socket, &pkg, sizeof(pkg), 0);
}

// vao group cua minh

void join_group(int client_socket)
{
    show_group(client_socket);
    sleep(1);
    Package pkg;
    pkg.ctrl_signal = JOIN_GROUP;
    /* chon group*/
    char group_name[GROUP_NAME_SIZE];
    printf("Group Name (Group_n): \n");        // fix
    fgets(group_name, GROUP_NAME_SIZE, stdin); // fix
    group_name[strlen(group_name) - 1] = '\0';
    strcpy(pkg.sender, my_username);
    strcpy(pkg.msg, group_name);
    send(client_socket, &pkg, sizeof(pkg), 0);
    sleep(1);
    if (join_succ == 1)
        handle_group_mess(client_socket);
    else
        return;
}

void handle_group_mess(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = HANDLE_GROUP_MESS;
    send(client_socket, &pkg, sizeof(pkg), 0);
    // xu ly
    int choice = 0;
    int login_group = 1;
    while (login_group)
    {
        sleep(1);

        sub_group_chat_menu(curr_group_name);
        printf("Your choice: \n"); // fix
        scanf("%d", &choice);      // fix
        clear_stdin_buff();        // fix

        switch (choice)
        {
        case 1:
            invite_friend(client_socket);
            break;
        case 2:
            group_chat(client_socket);
            break;
        case 3:
            show_group_info(client_socket);
            break;
        case 4:
            leave_group(client_socket);
            login_group = 0;
            break;
        default:
            login_group = 0;
            break;
        }
    }
    join_succ = 0;
    curr_group_id = -1;
    return;
}
// moi ban
void invite_friend(int client_socket)
{
    see_active_user(client_socket);
    Package pkg;
    char friends_name[USERNAME_SIZE];

    printf("Friends name: \n");                // fix
    fgets(friends_name, USERNAME_SIZE, stdin); // fix
    friends_name[strlen(friends_name) - 1] = '\0';

    strcpy(pkg.receiver, friends_name);
    strcpy(pkg.msg, my_username);
    strcat(pkg.msg, " Added you to ");
    strcat(pkg.msg, curr_group_name);
    pkg.ctrl_signal = INVITE_FRIEND;
    pkg.group_id = curr_group_id;
    send(client_socket, &pkg, sizeof(pkg), 0);
}

// chat trong nhom
void group_chat(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = GROUP_CHAT;
    pkg.group_id = curr_group_id;
    strcpy(pkg.sender, my_username);
    char msg[MSG_SIZE];
    while (1)
    {
        printf("Message(leave blank to exit group chat): \n"); // fix
        fgets(msg, MSG_SIZE, stdin);                           // fix
        msg[strlen(msg) - 1] = '\0';
        if (strlen(msg) == 0)
        {
            break;
        }

        strcpy(pkg.msg, msg);
        send(client_socket, &pkg, sizeof(pkg), 0);

        sleep(1);
    }
}

// hien thi thong tin phong -> oke
void show_group_info(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = GROUP_INFO;
    pkg.group_id = curr_group_id;
    send(client_socket, &pkg, sizeof(pkg), 0);
}
// Thoat nhom -> oke
void leave_group(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = LEAVE_GROUP;
    pkg.group_id = curr_group_id;
    strcpy(pkg.sender, my_username);
    // curr_group_id = -1;
    send(client_socket, &pkg, sizeof(pkg), 0);
}

int main()
{
    int client_socket = connect_to_server();
    my_socket = client_socket;
    ask_server(client_socket);
    close(client_socket);
    return 0;
}