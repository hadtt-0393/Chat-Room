#include "client.h"
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#define ENTER 10
// MENU
#define MAX_CHOOSE_HOME_MENU 4
#define MAX_CHOOSE_CHAT_MENU 7
#define MAX_CHOOSE_GROUP_CHAT_MENU 4
#define MAX_CHOOSE_SUB_GROUP_CHAT_MENU 5
const char HOME_MENU_TITLE[] = "USER MANAGEMENT PROGRAM";
const char HOME_MENU[MAX_CHOOSE_HOME_MENU][20] = {"REGISTER", "ACTIVATE", "SIGN IN", "EXIT"};
const char CHAT_MENU_TITLE[] = "CHAT ROOM";
const char CHAT_MENU[MAX_CHOOSE_CHAT_MENU][20] = {"PRIVATE CHAT", "GROUP CHAT", "CHAT ALL", "SHOW ONLINE USERS", "SEARCH USERS", "CHANGE PASSWORD", "SIGN OUT"};
const char GROUP_CHAT_MENU_TITLE[] = "GROUP CHAT";
const char GROUP_CHAT_MENU[MAX_CHOOSE_GROUP_CHAT_MENU][20] = {"SHOW MY GROUP", "CREATE NEW GROUP", "JOIN GROUP", "BACK"};
const char SUB_GROUP_CHAT_MENU[MAX_CHOOSE_SUB_GROUP_CHAT_MENU][20] = {"INVITE YOUR FRIENDS", "CHAT", "SHOW GROUP INFO", "LEAVE GROUP", "BACK"};
//
char PROMPT[10][2][50] = {0};
// SCREEN
WINDOW *menuW = NULL;
WINDOW *menuTopW = NULL;
WINDOW *chatW = NULL;
WINDOW *alertW = NULL;
WINDOW *promptW = NULL;
//
char sender[30][256] = {0};
char message[30][MSG_SIZE] = {0};
int messageN = 0;
// network
char my_username[USERNAME_SIZE];
char curr_group_name[GROUP_NAME_SIZE];
int curr_group_id = -1;
int join_succ = 0;
int my_socket;
bool isTyping = false;
bool isThreadFocus = false;
//
void initColors()
{
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_RED);
    init_pair(2, COLOR_BLACK, COLOR_CYAN);
    init_pair(3, COLOR_BLACK, COLOR_BLUE);
    init_pair(4, COLOR_BLACK, COLOR_YELLOW);
    init_pair(5, COLOR_BLACK, COLOR_WHITE);
    init_pair(6, COLOR_BLACK, COLOR_GREEN);
    init_pair(7, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(8, COLOR_BLACK, COLOR_BLACK);
    init_pair(9, COLOR_RED, COLOR_BLACK);
    init_pair(10, COLOR_CYAN, COLOR_BLACK);
    init_pair(11, COLOR_BLUE, COLOR_BLACK);
    init_pair(12, COLOR_YELLOW, COLOR_BLACK);
    init_pair(13, COLOR_WHITE, COLOR_BLACK);
    init_pair(14, COLOR_GREEN, COLOR_BLACK);
    init_pair(15, COLOR_MAGENTA, COLOR_BLACK);
}
void standoutChoose(WINDOW *win, int *y, int x, int min, int max, char const list[][20], int up /*1: up, 0: down*/)
{
    wattroff(win, A_STANDOUT | A_BLINK);
    mvwprintw(win, *y, x, "%-33s", list[*y - min]);
    wattron(win, A_STANDOUT | A_BLINK);
    if (up)
        *y = (--*y) < min ? max : *y;
    else
        *y = (++*y) > max ? min : *y;
    mvwprintw(win, *y, x, "%-33s", list[*y - min]);
    wattroff(win, A_STANDOUT | A_BLINK);
    wrefresh(win);
}
int showMenu(char const menuList[][20], int listN, char const *title)
{
    int xmax, ymax, ch, x = 3, y = 3, min = y, max = listN + y - 1;
    xmax = getmaxx(stdscr);
    menuTopW = newwin(5, 40, 5, 5);
    box(menuTopW, '#', '-');
    wbkgd(menuTopW, COLOR_PAIR(2) | A_BOLD);
    mvwaddstr(menuTopW, 0, 8, " _  _  ____  __ _  _  _ ");
    mvwaddstr(menuTopW, 1, 8, "( \\/ )(  __)(  ( \\/ )( \\");
    mvwaddstr(menuTopW, 2, 8, "/ \\/ \\ ) _) /  \\ /) \\/ (");
    mvwaddstr(menuTopW, 3, 8, "\\_)(_/(____)\\_)__)\\____/");
    menuW = newwin(6 + listN, 40, 9, 5);
    wattron(menuW, COLOR_PAIR(2) | A_BOLD);
    box(menuW, 0, 0);
    wattron(menuW, COLOR_PAIR(14));
    wattroff(menuW, A_BOLD);
    mvwaddstr(menuW, getmaxy(menuW) - 2, 2, "Use arrow keys to navigate");
    wattron(menuW, A_BOLD);
    wattron(menuW, COLOR_PAIR(10));
    mvwaddstr(menuW, 1, (getmaxx(menuW) - strlen(title)) / 2, title);
    wattrset(menuW, COLOR_PAIR(12) | A_BOLD);
    for (int i = 0; i < listN; i++)
    {
        if (!i)
            wattron(menuW, A_STANDOUT | A_BLINK);
        mvwprintw(menuW, min + i, 3, "%-33s", menuList[i]);
        if (!i)
            wattroff(menuW, A_STANDOUT | A_BLINK);
    }
    wrefresh(menuTopW);
    wrefresh(menuW);
    noecho();
    // curs_set(1);
    keypad(menuW, TRUE);
    while ((ch = wgetch(menuW)) != ENTER && ch != KEY_LEFT && ch != KEY_RIGHT)
    {
        switch (ch)
        {
        case KEY_UP:
            standoutChoose(menuW, &y, x, min, max, menuList, 1);
            break;
        case KEY_DOWN:
            standoutChoose(menuW, &y, x, min, max, menuList, 0);
            break;
        default:
            break;
        }
    }
    echo();
    keypad(menuW, false);
    return ch == KEY_LEFT ? max : y - min + 1;
}
void delMenu()
{
    wclear(menuW);
    wclear(menuTopW);
    wrefresh(menuW);
    wrefresh(menuTopW);
    delwin(menuW);
    delwin(menuTopW);
}
void showAlert(const char *mess)
{
    alertW = newwin(5, 40, getmaxy(stdscr) - 15, 5);
    wattron(alertW, COLOR_PAIR(12) | A_BOLD);
    mvwaddstr(alertW, 1, 2, mess);
    wattroff(alertW, COLOR_PAIR(12) | A_BOLD);
    wattron(alertW, COLOR_PAIR(14));
    mvwaddstr(alertW, 3, 2, "Automatically close after 2.5s!");
    wattroff(alertW, COLOR_PAIR(14));
    wattron(alertW, A_BOLD);
    box(alertW, 0, 0);
    wattroff(alertW, A_BOLD);
    wrefresh(alertW);
    // wgetch(menuW);
    napms(2500);
    wclear(alertW);
    wrefresh(alertW);
    delwin(alertW);
}
void showPrompt(int n, char *title)
{
    echo();
    curs_set(1);
    promptW = newwin(5 + n, 40, getmaxy(stdscr) - 15, 5);
    keypad(promptW, false);
    box(promptW, 0, 0);
    wattron(promptW, COLOR_PAIR(14) | A_BOLD);
    mvwaddstr(promptW, 1, (getmaxx(promptW) - strlen(title)) / 2, title);
    wattroff(promptW, COLOR_PAIR(14) | A_BOLD);
    for (int i = 0; i < n; i++)
    {
        mvwaddstr(promptW, 3 + i, 3, PROMPT[i][0]);
        wrefresh(promptW);
        wgetstr(promptW, PROMPT[i][1]);
    }
    keypad(promptW, true);
    noecho();
    curs_set(0);
    wclear(promptW);
    wrefresh(promptW);
    delwin(promptW);
}
void addChat(char _sender[], char mess[])
{
    if (strlen(mess) == 0)
    {
        return;
    }
    for (int i = messageN; i > 0; i--)
    {
        strcpy(sender[i], sender[i - 1]);
        strcpy(message[i], message[i - 1]);
    }
    strcpy(sender[0], _sender);
    strcpy(message[0], mess);
    if (++messageN > 16)
        messageN = 16;
}
void openChat()
{
    chatW = newwin(35, 60, 1, getmaxx(stdscr) - 65);
}
void renderChat()
{
    // keypad(chatW, false);
    wclear(chatW);
    wattron(chatW, A_BOLD);
    mvwaddstr(chatW, 1, getmaxx(chatW) / 2 - 2, "Chat");
    int maxy = getmaxy(chatW);
    wattron(chatW, COLOR_PAIR(10));
    box(chatW, 0, 0);
    wattroff(chatW, COLOR_PAIR(10));
    for (int i = 0; i < messageN; i++)
    {
        if (strcmp(sender[i], my_username) == 0)
        {
            wattron(chatW, COLOR_PAIR(15));
            mvwprintw(chatW, maxy - 4 - i * 2, 3, "You: %s", message[i]);
            wattroff(chatW, COLOR_PAIR(15));
        }
        else
        {
            wattron(chatW, COLOR_PAIR(14));
            mvwprintw(chatW, maxy - 4 - i * 2, 3, "%s: %s", sender[i], message[i]);
            wattroff(chatW, COLOR_PAIR(14));
        }
    }
    wattroff(chatW, COLOR_PAIR(2) | A_BOLD);
    if (isTyping)
    {
        mvwaddstr(chatW, getmaxy(chatW) - 2, 2, "Enter message: ");
        wmove(chatW, getmaxy(chatW) - 2, 17);
    }
    wrefresh(chatW);
}
void inputChat(char m[])
{
    echo();
    curs_set(1);
    isTyping = true;
    mvwaddstr(chatW, getmaxy(chatW) - 2, 2, "Enter message:");
    wrefresh(chatW);
    mvwgetstr(chatW, getmaxy(chatW) - 2, 17, m);
    noecho();
    curs_set(0);
    isTyping = false;
    addChat(my_username, m);
    renderChat();
}
void delChat()
{
    messageN = 0;
    memset(sender, 0, sizeof(sender));
    memset(message, 0, sizeof(message));
    wclear(chatW);
    wrefresh(chatW);
    delwin(chatW);
}
//
void setPrompt(int n, ...)
{
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++)
    {
        strcpy(PROMPT[i][0], va_arg(args, char *));
    }
    va_end(args);
}
void showAccPrompt()
{
    setPrompt(2, "Username: ", "Password: ");
    showPrompt(2, "Input username and password");
}
void report_err_hash(int err_code)
{
    // printf("Code: %d\n", err_code);
    char err[8] = "Code: ";
    showAlert(err);
}
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
        report_err_hash(ERR_SOCKET_INIT);
        exit(0);
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        report_err_hash(ERR_CONNECT_TO_SERVER);
        exit(0);
    }

    return client_socket;
}
void signup_hash(int sock)
{
    Package pkg;
    pkg.ctrl_signal = REGISTER_REQ;
    send(sock, &pkg, sizeof(pkg), 0);
    showAccPrompt();
    if (strlen(PROMPT[0][1]) == 0 || strlen(PROMPT[1][1]) == 0)
    {
        showAlert("Username or password is empty");
        return;
    }

    strcpy(pkg.msg, PROMPT[0][1]);
    send(sock, &pkg, sizeof(pkg), 0);

    recv(sock, &pkg, sizeof(pkg), 0);

    strcpy(pkg.msg, PROMPT[1][1]);
    send(sock, &pkg, sizeof(pkg), 0);

    recv(sock, &pkg, sizeof(pkg), 0);

    showAlert(pkg.ctrl_signal == REGISTER_SUCC ? "Successful registration" : "Account existed");
}
void activate_hash(int sock)
{
    Package pkg;
    pkg.ctrl_signal = ACTIVATE_REQ;
    send(sock, &pkg, sizeof(pkg), 0);
    showAccPrompt();

    if (strlen(PROMPT[0][1]) == 0 || strlen(PROMPT[1][1]) == 0)
    {
        showAlert("Username or password is empty");
        return;
    }
    strcpy(pkg.msg, PROMPT[0][1]);
    send(sock, &pkg, sizeof(pkg), 0);

    recv(sock, &pkg, sizeof(pkg), 0);

    strcpy(pkg.msg, PROMPT[1][1]);
    send(sock, &pkg, sizeof(pkg), 0);

    recv(sock, &pkg, sizeof(pkg), 0);

    if (pkg.ctrl_signal == ACC_IDLE)
    {
        setPrompt(1, "Input Activate code: ");
        char title[100] = "Activate code: ";
        strcat(title, pkg.msg);
        showPrompt(1, title);
        strcpy(pkg.msg, PROMPT[0][1]);
        send(sock, &pkg, sizeof(pkg), 0);

        recv(sock, &pkg, sizeof(pkg), 0);
        showAlert(pkg.ctrl_signal == ACTIVATE_SUCC ? "Successful activation" : "Activation fail");
        return;
    }
    char mess[100];
    switch (pkg.ctrl_signal)
    {
    case ACC_BLOCKED:
        strcpy(mess, "This account is blocked");
        /* code */
        break;
    case ACC_ACTIVATED:
        strcpy(mess, "This account is activated");
        /* code */
        break;
    case ERR_INCORRECT_ACC:
        strcpy(mess, "This account isn't existed");
        /* code */
        break;
    default:
        break;
    }
    showAlert(mess);
}
bool login_hash(int sock)
{
    Package pkg;
    pkg.ctrl_signal = LOGIN_REQ;
    send(sock, &pkg, sizeof(pkg), 0);
    showAccPrompt();

    if (strlen(PROMPT[0][1]) == 0 || strlen(PROMPT[1][1]) == 0)
    {
        showAlert("Username or password is empty");
        return false;
    }
    strcpy(pkg.msg, PROMPT[0][1]);
    send(sock, &pkg, sizeof(pkg), 0);

    recv(sock, &pkg, sizeof(pkg), 0);

    strcpy(pkg.msg, PROMPT[1][1]);
    send(sock, &pkg, sizeof(pkg), 0);

    recv(sock, &pkg, sizeof(pkg), 0);
    if (pkg.ctrl_signal == LOGIN_SUCC)
    {
        strcpy(my_username, PROMPT[0][1]);
        // showAlert("Login successfully");
        return true;
    }
    char mess[100];
    switch (pkg.ctrl_signal)
    {
    case ACC_BLOCKED:
        strcpy(mess, "This account is blocked");
        /* code */
        break;
    case ACC_IDLE:
        strcpy(mess, "This account isn't activated");
        /* code */
        break;
    case ERR_INCORRECT_ACC:
        strcpy(mess, "Incorrect account");
        /* code */
        break;
    case ERR_SIGNED_IN_ACC:
        strcpy(mess, "Already signed in account");
        /* code */
        break;
        endwin();
        exit(0);
    default:
        break;
    }
    showAlert(mess);
    return false;
}
void *read_msg_hash(void *sock)
{
    int client_socket = *(int *)sock;
    char str[MSG_SIZE];
    Package pkg;
    while (true)
    {
        recv(client_socket, &pkg, sizeof(pkg), 0);
        switch (pkg.ctrl_signal)
        {
        case SHOW_USER:
            strcpy(str, "Current online users: ");
            strcat(str, pkg.msg);
            showAlert(str);
            isThreadFocus = false;
            break;
        case PRIVATE_CHAT:
            strcat(pkg.sender, " to you");
            addChat(pkg.sender, pkg.msg);
            renderChat();
            break;
        case GROUP_CHAT_INIT:
            // napms(100);
            // showAlert("Group chat init");
            break;
        case ERR_INVALID_RECEIVER:
            showAlert("Invalid receiver");
            break;
        case CHAT_ALL:
            if (strcmp(pkg.sender, my_username) == 0)
            {
                break;
            }
            strcat(pkg.sender, " to all");
            addChat(pkg.sender, pkg.msg);
            renderChat();
            break;
        case MSG_SENT_SUCC:
            beep();
            break;
        case SHOW_GROUP:
            strcpy(str, "Your group: ");
            strcat(str, pkg.msg);
            str[strlen(str) - 1] = '\0';
            showAlert(str);
            break;
        case MSG_MAKE_GROUP_SUCC:
            strcpy(str, "Your new group: ");
            strcat(str, pkg.msg);
            showAlert(str);
            break;
        case JOIN_GROUP_SUCC:
            join_succ = 1;
            strcpy(str, "Current group: ");
            strcat(str, pkg.msg);
            strcpy(curr_group_name, pkg.msg);
            curr_group_id = pkg.group_id;
            showAlert(str);
            break;
        case ERR_GROUP_NOT_FOUND:
            join_succ = 0;
            showAlert("Group not found");
            break;
        case INVITE_FRIEND:
            strcpy(str, "Attention: ");
            strcat(str, pkg.msg);
            showAlert(str);
            break;
        case ERR_IVITE_MYSELF:
            showAlert("You can't invite yourself");
            break;
        case ERR_USER_NOT_FOUND:
            showAlert("User not found");
            break;
        case ERR_FULL_MEM:
            showAlert("Group is full");
            break;
        case INVITE_FRIEND_SUCC:
            showAlert(pkg.msg);
            break;
        case GROUP_CHAT:
            if (curr_group_id == pkg.group_id)
            {
                addChat(pkg.sender, pkg.msg);
                renderChat();
            }
            else
            {
                strcat(pkg.sender, " send to Group_ ");
                pkg.sender[strlen(pkg.sender) - 1] = pkg.group_id + '0';
                addChat(pkg.sender, pkg.msg);
                renderChat();
            }
            break;
        case SHOW_GROUP_NAME:
            printf("GROUP NAME: %s\n", pkg.msg); // fix
            strcpy(str, "GROUP NAME: ");
            strcat(str, pkg.msg);
            showAlert(str);
            break;
        case SHOW_GROUP_MEM:
            showAlert(pkg.msg);
            break;
        case LEAVE_GROUP_SUCC:
            showAlert(pkg.msg);
            break;
        case ERR_WRONG_PASSWORD:
            showAlert("Wrong password");
            break;
        case CORRECT_PASSWORD:
            strcpy(pkg.msg, PROMPT[1][1]);
            send(client_socket, &pkg, sizeof(pkg), 0);
            break;
        case CHANGE_PASSWORD_SUCC:
            showAlert("Change password successfully");
            break;
        case ERR_CHANGE_PASSWORD_FAIL:
            showAlert("Change password fail");
            break;
        case ACC_BLOCKED:
            showAlert("This account is blocked");
            break;
        case ACC_IDLE:
            showAlert("This account needs activation");
            break;
        case ACC_ACTIVATED:
            showAlert("This account is active");
            break;
        case ERR_INCORRECT_ACC:
            showAlert("Unknown error");
            break;
        default:
            break;
        }
    }
}
void see_active_user_hash(int sock)
{
    Package pkg;
    pkg.ctrl_signal = SHOW_USER;
    send(sock, &pkg, sizeof(pkg), 0);
}
void private_chat_hash(int sock)
{
    setPrompt(1, "Receiver: ");
    showPrompt(1, "Input receiver");
    if (!strcmp(my_username, PROMPT[0][1]))
    {
        showAlert("You can't chat with yourself");
        return;
    }
    Package pkg;
    char username[USERNAME_SIZE];
    char msg[MSG_SIZE];
    pkg.ctrl_signal = PRIVATE_CHAT;

    strcpy(pkg.receiver, PROMPT[0][1]);
    strcpy(pkg.sender, my_username);
    while (1)
    {
        inputChat(msg);
        if (strlen(msg) == 0)
        {
            break;
        }

        strcpy(pkg.msg, msg);
        send(sock, &pkg, sizeof(pkg), 0);

        napms(100);
    }
}
void show_group_hash(int sock)
{
    Package pkg;
    pkg.ctrl_signal = SHOW_GROUP;
    send(sock, &pkg, sizeof(pkg), 0);
}
void new_group_hash(int sock)
{
    Package pkg;
    pkg.ctrl_signal = NEW_GROUP;
    send(sock, &pkg, sizeof(pkg), 0);
}
void leave_group_hash(int sock)
{
    Package pkg;
    pkg.ctrl_signal = LEAVE_GROUP;
    pkg.group_id = curr_group_id;
    strcpy(pkg.sender, my_username);
    // curr_group_id = -1;
    send(sock, &pkg, sizeof(pkg), 0);
}
void invite_friend_hash(int sock)
{
    see_active_user_hash(sock);
    isThreadFocus = true;
    while (isThreadFocus)
        ;
    Package pkg;
    char friends_name[USERNAME_SIZE];

    setPrompt(1, "Friends name: ");
    showPrompt(1, "Input friends name");
    strcpy(pkg.receiver, PROMPT[0][1]);

    strcpy(pkg.msg, my_username);
    strcat(pkg.msg, " Added you to ");
    strcat(pkg.msg, curr_group_name);
    pkg.ctrl_signal = INVITE_FRIEND;
    pkg.group_id = curr_group_id;
    send(sock, &pkg, sizeof(pkg), 0);
}
void group_chat_hash(int sock)
{
    Package pkg;
    pkg.ctrl_signal = GROUP_CHAT;
    pkg.group_id = curr_group_id;
    strcpy(pkg.sender, my_username);
    char msg[MSG_SIZE];
    while (1)
    {
        inputChat(msg);
        if (strlen(msg) == 0)
        {
            break;
        }
        strcpy(pkg.msg, msg);
        send(sock, &pkg, sizeof(pkg), 0);
    }
}
void show_group_info_hash(int sock)
{
    Package pkg;
    pkg.ctrl_signal = GROUP_INFO;
    pkg.group_id = curr_group_id;
    send(sock, &pkg, sizeof(pkg), 0);
}
void handle_group_mess_hash(int sock)
{
    Package pkg;
    pkg.ctrl_signal = HANDLE_GROUP_MESS;
    send(sock, &pkg, sizeof(pkg), 0);
    // xu ly
    int choice = 0;
    int login_group = 1;
    while (login_group)
    {
        choice = showMenu(SUB_GROUP_CHAT_MENU, MAX_CHOOSE_SUB_GROUP_CHAT_MENU, curr_group_name);
        switch (choice)
        {
        case 1:
            invite_friend_hash(sock);
            break;
        case 2:
            group_chat_hash(sock);
            break;
        case 3:
            show_group_info_hash(sock);
            break;
        case 4:
            leave_group_hash(sock);
        default:
            goto backHandler;
            break;
        }
        delMenu();
    }
backHandler:
    login_group = 0;
    join_succ = 0;
    curr_group_id = -1;
    return;
}
void join_group_hash(int sock)
{
    // show_group_hash(sock);
    // napms(1000);
    Package pkg;
    pkg.ctrl_signal = JOIN_GROUP;
    /* chon group*/
    char group_name[GROUP_NAME_SIZE];
    setPrompt(1, "Group Name (Group_n): ");
    showPrompt(1, "Input group name");
    strcpy(group_name, PROMPT[0][1]);

    strcpy(pkg.sender, my_username);
    strcpy(pkg.msg, group_name);
    send(sock, &pkg, sizeof(pkg), 0);
    join_succ = -1;
    // Đợi luồng nhận gửi pkg về
    while (join_succ == -1)
        ;
    if (join_succ == 1)
    {
        delMenu();
        handle_group_mess_hash(sock);
    };
}
void group_chat_init_hash(int sock)
{
    Package pkg;
    pkg.ctrl_signal = GROUP_CHAT_INIT;
    send(sock, &pkg, sizeof(pkg), 0);
    int choice = 0;
    while (choice = showMenu(GROUP_CHAT_MENU, MAX_CHOOSE_GROUP_CHAT_MENU, GROUP_CHAT_MENU_TITLE))
    {
        switch (choice)
        {
        case 1:
            show_group_hash(sock);
            break;
        case 2:
            new_group_hash(sock);
            break;
        case 3:
            join_group_hash(sock);
            break;
        default:
            goto backG;
            return;
        }
        delMenu();
    }
backG:
}
void change_password_hash(int sock)
{
    char password[PASSWORD_SIZE];
    char new_password[PASSWORD_SIZE];
    Package pkg;
    setPrompt(2, "Password: ", "New password: ");
    showPrompt(2, "Input password and new password");
    if (strcmp(PROMPT[0][1], PROMPT[1][1]) == 0)
    {
        showAlert("New and old password are the same");
        return;
    }
    pkg.ctrl_signal = CHANGE_PASSWORD;
    strcpy(pkg.sender, my_username);
    send(sock, &pkg, sizeof(pkg), 0);
    strcpy(pkg.msg, PROMPT[0][1]);
    send(sock, &pkg, sizeof(pkg), 0);
}
void chat_all_hash(int sock)
{
    Package pkg;
    pkg.ctrl_signal = CHAT_ALL;
    strcpy(pkg.sender, my_username);
    char msg[MSG_SIZE];
    while (1)
    {
        inputChat(msg);
        if (strlen(msg) == 0)
        {
            break;
        }

        strcpy(pkg.msg, msg);
        send(sock, &pkg, sizeof(pkg), 0);
    }
}
void search_users_hash(int sock)
{
    Package pkg;
    pkg.ctrl_signal = SEARCH_USERS;
    send(sock, &pkg, sizeof(pkg), 0);
    setPrompt(1, "Username: ");
    showPrompt(1, "Input username");
    strcpy(pkg.msg, PROMPT[0][1]);
    send(sock, &pkg, sizeof(pkg), 0);
}
void showUsername()
{
    attron(A_BOLD | COLOR_PAIR(9));
    mvprintw(2, 10, "Hello %-10s", my_username);
    attroff(A_BOLD | COLOR_PAIR(9));
    refresh();
}
void clearUsername()
{
    mvprintw(1, 1, "%20s", "");
    refresh();
}
void user_use_hash(int sock)
{
    showUsername();
    openChat();
    renderChat();
    Package pkg;
    int choice = 0;
    pthread_t tid;
    if (pthread_create(&tid, NULL, read_msg_hash, (void *)&sock) < 0)
    {
        report_err_hash(ERR_CREATE_THREAD);
        exit(0);
    }
    pthread_detach(tid);
    while (choice = showMenu(CHAT_MENU, MAX_CHOOSE_CHAT_MENU, CHAT_MENU_TITLE))
    {
        switch (choice)
        {
        case 1:
            private_chat_hash(sock);
            break;
        case 2:
            delMenu();
            group_chat_init_hash(sock);
            break;
        case 3:
            chat_all_hash(sock);
            break;
        case 4:
            see_active_user_hash(sock);
            break;
        case 5:
            search_users_hash(sock);
            break;
        case 6:
            change_password_hash(sock);
            break;
        default:
            pkg.ctrl_signal = LOG_OUT;
            send(sock, &pkg, sizeof(pkg), 0);
            strcpy(my_username, "");
            recv(sock, &pkg, sizeof(pkg), 0);
            goto back;
            break;
        }
        delMenu();
    }
back:
    clearUsername();
    delChat();
    pthread_cancel(tid);
}
void quit_hash(int sock)
{
    Package pkg;
    pkg.ctrl_signal = QUIT_REQ;
    send(sock, &pkg, sizeof(pkg), 0);
}
int main(int argc, char const *argv[])
{
    initscr();
    initColors();
    curs_set(FALSE);
    echo();
    cbreak();
    int choice = 0;
    int client_socket = connect_to_server();
    my_socket = client_socket;
    while (choice = showMenu(HOME_MENU, MAX_CHOOSE_HOME_MENU, HOME_MENU_TITLE))
    {
        switch (choice)
        {
        case 1:
            // code
            signup_hash(client_socket);
            break;
        case 2:
            activate_hash(client_socket);
            break;
        case 3:
            // code
            if (login_hash(client_socket))
            {
                delMenu();
                user_use_hash(client_socket);
            }
            break;
        default:
            goto quit;
            break;
        }
        delMenu();
    }
quit:
    quit_hash(client_socket);
    close(client_socket);
    delMenu();
    endwin();
    return 0;
}
