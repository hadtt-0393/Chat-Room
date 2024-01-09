#ifndef __ERROR_H__
#define __ERROR_H__

//* Lỗi chung
#define ERR_SOCKET_INIT -1   /* không khởi tạo được socket */
#define ERR_CREATE_THREAD -2 /* không mở tạo được thread mới */

//* Lỗi phía server
#define ERR_CONN_ACCEPT -101 /* không kết nối được */
#define ERR_OPEN_FILE -102   /* không mở được file */

//* Lỗi phía client
#define ERR_CONNECT_TO_SERVER -201      /* không kết nối được đến server */
#define ERR_ACC_EXISTED -202            /* không đăng ký được tài khoản mới do tên tài khoản trùng với tài khoản trước đó */ 
#define ERR_ACTIVATE_FAIL -203          /* không kích hoạt được tài khoản mới*/
#define ERR_INCORRECT_ACC -204          /* không mật khẩu hoặc tài khoản không chính xác */
#define ERR_CHANGE_PASSWORD_FAIL -205   /* không đổi được mật khẩu */
#define ERR_WRONG_PASSWORD -206         /* Sai mật khẩu */
#define ERR_SIGNED_IN_ACC -207          /* đăng nhập vào tài khoản đã được đăng nhập từ trước */
#define ERR_INVALID_RECEIVER -208       /* không tìm thấy người nhận */
#define ERR_GROUP_NOT_FOUND -209        /* không tìm thấy group */
#define ERR_IVITE_MYSELF -210           /* mời bản thân vào nhóm */
#define ERR_USER_NOT_FOUND -211         /* không tìm thấy người dùng */
#define ERR_FULL_MEM -212               /* đầy người */
#define ERR_IS_MEM -213                 /* đã là thành viên trong nhóm */
/**
 * Hiển thị thông báo lỗi
 * @param err_code mã lỗi
 */
void report_err(int err_code);

#endif