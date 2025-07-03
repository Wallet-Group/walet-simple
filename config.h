#pragma once

// Đường dẫn thư mục chứa dữ liệu (nếu bạn để chung folder thì để "./")
const std::string DIRECTORY = "./";

// Tên các file dữ liệu
const std::string USERS_FILE = "users.txt";
const std::string WALLETS_FILE = "wallets.txt";
const std::string TRANSACTIONS_FILE = "transactions.txt";

// Cấu hình OTP
const int OTP_LENGTH = 6;
const int OTP_VALID_DURATION = 60; // đơn vị: giây

// Số lần đăng nhập sai tối đa
const int MAX_LOGIN_ATTEMPTS = 5;
