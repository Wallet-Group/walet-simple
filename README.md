
#  Hệ thống đăng nhập, OTP và ví điểm thưởng – Dự án C++

##  Giới thiệu dự án
Hệ thống được viết bằng C++ thuần (không dùng thư viện ngoài), mô phỏng chức năng:
- Đăng ký / Đăng nhập tài khoản
- Xác thực hai lớp qua OTP khi giao dịch và chỉnh sửa thông tin
- Quản lý ví điểm thưởng (nạp, chuyển điểm)
- Lịch sử giao dịch minh bạch
- s

##  Thành viên và phân công
| Họ tên           | MSSV | Vai trò           | Công việc được giao                    |
|----------------  |---------|-------------------|----------------------------------------|
| Nguyễn Cao Sơn   | K24DTCN418 | Trưởng nhóm       | - Thiết kế kiến trúc hệ thống, xử lý OTP |
| Trần Quốc Dũng   | K24DTCN657 | Thành viên        | Xử lý ví điểm và lịch sử giao dịch     |
| Vũ Thị Thuỷ      | B24DTCN497 | Thành viên         | - Đăng nhập, đăng xuất. Đổi password cho user   |
| Nguyễn Đình Hùng | K24DTCN397 | Thành viên         | - Xem thông tin user. Admin tạo account cho user. Chỉnh sửa thông tin user  |

##  Đặc tả chức năng
### Người dùng thông thường:
- Đăng nhập
- Xem ví điểm và lịch sử giao dịch
- Thay đổi thông tin cá nhân (phải xác nhận OTP)
- Chuyển điểm cho người khác (phải xác nhận OTP)

### Người quản lý:
- Đăng nhập
- Tạo tài khoản mới
- Xem danh sách người dùng
- Cập nhật tài khoản hộ người dùng (cần OTP từ chủ tài khoản)

##  Cách tải và biên dịch chương trình
1. Mở Dev-C++ hoặc Code::Blocks
2. Tạo Project mới
3. Dán nội dung file `main.cpp` vào
4. Biên dịch (`Ctrl+F9`) và chạy (`F10`)

Yêu cầu: C++11 trở lên

## Cách chạy chương trình
- Đăng nhập với tài khoản có sẵn (ví dụ: admin/123456)
- Chọn menu theo hướng dẫn trên màn hình
- Mật khẩu tự sinh cần đổi sau lần đăng nhập đầu

##  Các tập tin kèm theo
- `main.cpp`: mã nguồn chính
- `user.txt`: dữ liệu người dùng
- `wallet.txt`: dữ liệu ví điểm
- `transactions.txt`: lịch sử giao dịch

##  Lưu trữ bảo mật
- Mật khẩu được lưu ở dạng băm (hash)
- Tất cả thay đổi quan trọng cần mã OTP xác thực

##  Sao lưu
- Dữ liệu được ghi ra file văn bản
- Có thể copy file `.txt` làm bản sao lưu thủ công

##  Tài liệu tham khảo
- [1] https://en.wikipedia.org/wiki/One-time_password
- [2] https://cplusplus.com/reference/functional/hash/
