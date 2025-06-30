#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdio> 
#include <algorithm>

// ---------------------
// Utils
std::string getCurrentTime() {
    time_t now = time(nullptr);
    tm* ltm = localtime(&now);
    char buf[20];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
        1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
        ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    return std::string(buf);
}

// H�m hash don gian 
std::string simpleHash(const std::string& input) {
    std::hash<std::string> hasher;
    size_t hashed = hasher(input);
    std::stringstream ss;
    ss << std::hex << hashed;
    return ss.str();
}

// Sinh mat khau tu dong 
std::string generatePassword(int length = 6) {
    std::string pwd;
    for (int i = 0; i < length; ++i) {
        pwd += '0' + rand() % 10;
    }
    return pwd;
}

// Sinh OTP (6 chu so)
std::string generateOTP(int length = 6) {
    return generatePassword(length);
}

// ---------------------
// Class quan l� t�i khoan nguoi d�ng

enum class UserRole { NormalUser = 0, Manager = 1 };

struct User {
    std::string username;
    std::string passwordHash;
    std::string fullName;
    std::string phone;
    bool isPasswordAutoGen;
    UserRole role;

    // Serialize ra d�ng file: d�ng dau | ph�n c�ch
    std::string serialize() const {
        return username + "|" + passwordHash + "|" + fullName + "|" + phone + "|" +
            (isPasswordAutoGen ? "1" : "0") + "|" + std::to_string(static_cast<int>(role));
    }

    static User deserialize(const std::string& line) {
        std::stringstream ss(line);
        std::string token;
        User u;
        std::getline(ss, u.username, '|');
        std::getline(ss, u.passwordHash, '|');
        std::getline(ss, u.fullName, '|');
        std::getline(ss, u.phone, '|');
        std::getline(ss, token, '|');
        u.isPasswordAutoGen = (token == "1");
        std::getline(ss, token, '|');
        u.role = (token == "1") ? UserRole::Manager : UserRole::NormalUser;
        return u;
    }
};

// ---------------------
// Class v� diem

struct Transaction {
    std::string fromWallet;
    std::string toWallet;
    int amount;
    std::string time;
    bool success;
    std::string note;

    std::string serialize() const {
        return fromWallet + "|" + toWallet + "|" + std::to_string(amount) + "|" + time + "|" +
            (success ? "1" : "0") + "|" + note;
    }

    static Transaction deserialize(const std::string& line) {
        std::stringstream ss(line);
        std::string token;
        Transaction t;
        std::getline(ss, t.fromWallet, '|');
        std::getline(ss, t.toWallet, '|');
        std::getline(ss, token, '|');
        t.amount = std::stoi(token);
        std::getline(ss, t.time, '|');
        std::getline(ss, token, '|');
        t.success = (token == "1");
        std::getline(ss, t.note, '|');
        return t;
    }
};

// ---------------------

class Database {
private:
    std::unordered_map<std::string, User> users;   // key=username
    std::unordered_map<std::string, int> wallets;  // key=username, value=balance
    std::vector<Transaction> transactions;

    const std::string userFile = "user.txt";
    const std::string walletFile = "wallet.txt";
    const std::string transactionFile = "transactions.txt";

public:
    void load() {
        users.clear();
        wallets.clear();
        transactions.clear();

        // Load users
        std::ifstream fin(userFile);
        std::string line;
        while (std::getline(fin, line)) {
            if (line.empty()) continue;
            User u = User::deserialize(line);
            users[u.username] = u;
        }
        fin.close();

        // Load wallets
        fin.open(walletFile);
        while (std::getline(fin, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string username;
            int balance;
            ss >> username >> balance;
            wallets[username] = balance;
        }
        fin.close();

        // Load transactions
        fin.open(transactionFile);
        while (std::getline(fin, line)) {
            if (line.empty()) continue;
            transactions.push_back(Transaction::deserialize(line));
        }
        fin.close();
    }

    void save() {
        // Backup truoc khi luu
        backupFiles();

        // Save users
        std::ofstream fout(userFile, std::ios::trunc);
        for (auto it = users.begin(); it != users.end(); ++it) {
    	fout << it->second.serialize() << "\n";
		}
		
        fout.close();

        // Save wallets
        fout.open(walletFile, std::ios::trunc);
        for (auto it = wallets.begin(); it != wallets.end(); ++it) {
        fout << it->first << " " << it->second << "\n";
		}

        fout.close();

        // Save transactions
        fout.open(transactionFile, std::ios::trunc);
        for (const auto& t : transactions) {
            fout << t.serialize() << "\n";
        }
        fout.close();
    }

    void backupFiles() {
        std::string backupDir = "backup/";
             system("mkdir -p backup");

        auto backupFile = [&](const std::string& filename) {
            std::string cmd = "cp " + filename + " " + backupDir + filename + "." + std::to_string(time(nullptr));
            system(cmd.c_str());
        };

        backupFile(userFile);
        backupFile(walletFile);
        backupFile(transactionFile);
    }

    // User management
    bool userExists(const std::string& username) {
        return users.find(username) != users.end();
    }

    void addUser(const User& u, int initialBalance = 0) {
        users[u.username] = u;
        wallets[u.username] = initialBalance;
    }

    User* getUser(const std::string& username) {
        auto it = users.find(username);
        if (it == users.end()) return nullptr;
        return &(it->second);
    }

    bool checkPassword(const std::string& username, const std::string& passwordHash) {
        User* u = getUser(username);
        if (!u) return false;
        return u->passwordHash == passwordHash;
    }

    void updatePassword(const std::string& username, const std::string& newPasswordHash, bool autoGen = false) {
        User* u = getUser(username);
        if (u) {
            u->passwordHash = newPasswordHash;
            u->isPasswordAutoGen = autoGen;
        }
    }

    void updateUserInfo(const std::string& username, const std::string& newName, const std::string& newPhone) {
        User* u = getUser(username);
        if (u) {
            u->fullName = newName;
            u->phone = newPhone;
        }
    }

    int getBalance(const std::string& username) {
        if (wallets.find(username) == wallets.end()) return 0;
        return wallets[username];
    }

    void setBalance(const std::string& username, int amount) {
        wallets[username] = amount;
    }

    const std::vector<Transaction>& getTransactions() const {
        return transactions;
    }

    void addTransaction(const Transaction& t) {
        transactions.push_back(t);
    }

    // Giao dich chuyen diem (atomic)
    bool transferPoints(const std::string& fromUser, const std::string& toUser, int amount, std::string& errMsg) {
        if (!userExists(fromUser)) {
            errMsg = "Nguoi dung chuyen khong ton tai.";
            return false;
        }
        if (!userExists(toUser)) {
            errMsg = "Nguoi dung nhan khong ton tai.";
            return false;
        }
        if (amount <= 0) {
            errMsg = "So diem chuyen phai > 0.";
            return false;
        }
        int fromBalance = getBalance(fromUser);
        if (fromBalance < amount) {
            errMsg = "So du khong du.";
            return false;
        }
        // Thuc hien giao dich atomic
        setBalance(fromUser, fromBalance - amount);
        setBalance(toUser, getBalance(toUser) + amount);

        Transaction t;
        t.fromWallet = fromUser;
        t.toWallet = toUser;
        t.amount = amount;
        t.time = getCurrentTime();
        t.success = true;
        t.note = "Chuyen diem thanh cong";
        std::string randomOtp = OTPService::generateOTP();
        std::string inputOtp;
        std::cin >> inputOtp;
        while(inputOtp != randomOtp) {
            std::cout << "Ma OTP khong dung. Vui long nhap lai: ";
            std::cin >> inputOtp;
        }
        addTransaction(t);

        return true;
    }
};

// ---------------------
// OTP x�c nhan (m� luu tam thoi tr�n bo nho)

class OTPService {
private:
    std::unordered_map<std::string, std::string> otpStorage; // username -> OTP
public:
    std::string createOTP(const std::string& username) {
        std::string otp = generateOTP();
        otpStorage[username] = otp;
        return otp;
    }

    bool verifyOTP(const std::string& username, const std::string& otp) {
        if (otpStorage.find(username) == otpStorage.end()) return false;
        if (otpStorage[username] == otp) {
            otpStorage.erase(username);
            return true;
        }
        return false;
    }
};

// ---------------------
// C�c chuc nang ch�nh

void showMenu(bool isManager) {
    std::cout << "\n--- MENU ---\n";
    std::cout << "1. Xem thong tin ca nhan\n";
    std::cout << "2. Thay doi mat khau\n";
    std::cout << "3. Chuyen diem toi nguoi dung khac\n";
    std::cout << "4. Xem so du va lich su giao dich\n";
    if (isManager) {
        std::cout << "5. Tao tai khoan moi\n";
        std::cout << "6. Dieu chinh thong tin nguoi dung\n";
    }
    std::cout << "0. Dang xuat\n";
}

void viewPersonalInfo(Database& db, const std::string& username) {
    User* u = db.getUser(username);
    if (!u) return;
    std::cout << "Username: " << u->username << "\n";
    std::cout << "Ho ten: " << u->fullName << "\n";
    std::cout << "Dien thoai: " << u->phone << "\n";
    std::cout << "Mat khau tu sinh: " << (u->isPasswordAutoGen ? "Co" : "Khong") << "\n";
    std::cout << "Vai tro: " << (u->role == UserRole::Manager ? "Quan ly" : "Nguoi dung") << "\n";
}

void changePassword(Database& db, const std::string& username) {
    std::string oldPass, newPass, newPass2;
    std::cout << "Nhap mat khau cu: ";
    std::cin >> oldPass;
    if (!db.checkPassword(username, simpleHash(oldPass))) {
        std::cout << "Mat khau cu sai.\n";
        return;
    }
    std::cout << "Nhap mat khau moi: ";
    std::cin >> newPass;
    std::cout << "Nhap lai mat khau moi: ";
    std::cin >> newPass2;
    if (newPass != newPass2) {
        std::cout << "Mat khau moi khong khop.\n";
        return;
    }
    db.updatePassword(username, simpleHash(newPass), false);
    std::cout << "Doi mat khau thanh cong.\n";
}

void createAccount(Database& db) {
    std::string username, fullName, phone;
    int roleInput;
    bool autoGenPassword;
    std::cout << "Nhap username (khong duoc sua sau): ";
    std::cin >> username;
    if (db.userExists(username)) {
        std::cout << "Tai khoan da ton tai.\n";
        return;
    }
    std::cout << "Nhap ho ten: ";
    std::cin.ignore();
    std::getline(std::cin, fullName);
    std::cout << "Nhap so dien thoai: ";
    std::getline(std::cin, phone);
    std::cout << "Chon vai tro (0 - nguoi dung, 1 - quan ly): ";
    std::cin >> roleInput;

    std::cout << "Co sinh mat khau tu dong? (1: Co, 0: Khong): ";
    std::cin >> autoGenPassword;

    std::string password;
    if (autoGenPassword) {
        password = generatePassword();
        std::cout << "Mat khau sinh tu dong la: " << password << "\n";
    }
    else {
        std::cout << "Nhap mat khau: ";
        std::cin >> password;
    }
    User u{ username, simpleHash(password), fullName, phone, autoGenPassword, (roleInput == 1) ? UserRole::Manager : UserRole::NormalUser };
    db.addUser(u, 0);
    db.save();
    std::cout << "Tao tai khoan thanh cong.\n";
}

void editUserInfo(Database& db, OTPService& otpService, const std::string& username, bool isManager) {
    std::string targetUser;
    if (isManager) {
        std::cout << "Nhap username cua nguoi dung muon chinh sua: ";
        std::cin >> targetUser;
        if (!db.userExists(targetUser)) {
            std::cout << "Nguoi dung khong ton tai.\n";
            return;
        }
    }
    else {
        targetUser = username;
    }
    User* u = db.getUser(targetUser);
    if (!u) return;

    std::cout << "Thong tin hien tai:\n";
    std::cout << "Ho ten: " << u->fullName << "\n";
    std::cout << "Dien thoai: " << u->phone << "\n";

    std::cout << "Nhap ho ten moi (de trong de giu nguyen): ";
    std::cin.ignore();
    std::string newName;
    std::getline(std::cin, newName);
    if (newName.empty()) newName = u->fullName;

    std::cout << "Nhap so dien thoai moi (de trong de giu nguyen): ";
    std::string newPhone;
    std::getline(std::cin, newPhone);
    if (newPhone.empty()) newPhone = u->phone;

    // Gui OTP
    std::string otp = otpService.createOTP(targetUser);
    std::cout << "Ma OTP da gui toi nguoi dung (gi? l?p): " << otp << "\n";
    std::cout << "Thong bao thay doi: Ho ten va so dien thoai.\n";
    std::cout << "Nhap ma OTP de xac nhan thay doi: ";
    std::string otpInput;
    std::getline(std::cin, otpInput);
    if (!otpService.verifyOTP(targetUser, otpInput)) {
        std::cout << "Ma OTP khong dung. Huy thao tac.\n";
        return;
    }
    db.updateUserInfo(targetUser, newName, newPhone);
    db.save();
    std::cout << "Cap nhat thong tin thanh cong.\n";
}

void transferPoints(Database& db, OTPService& otpService, const std::string& username) {
    std::string toUser;
    int amount;
    std::cout << "Nhap username nguoi nhan diem: ";
    std::cin >> toUser;
    std::cout << "Nhap so diem can chuyen: ";
    std::cin >> amount;

    if (toUser == username) {
        std::cout << "Khong the chuyen diem cho chinh ban than.\n";
        return;
    }

    // Tao OTP
    std::string otp = otpService.createOTP(username);
    std::cout << "Ma OTP da gui toi ban (gi? l?p): " << otp << "\n";
    std::cout << "Nhap ma OTP de xac nhan giao dich: ";
    std::string otpInput;
    std::cin.ignore();
    std::getline(std::cin, otpInput);
    if (!otpService.verifyOTP(username, otpInput)) {
        std::cout << "Ma OTP khong dung. Huy giao dich.\n";
        return;
    }

    std::string errMsg;
    if (db.transferPoints(username, toUser, amount, errMsg)) {
        db.save();
        std::cout << "Chuyen diem thanh cong.\n";
    }
    else {
        std::cout << "Chuyen diem that bai: " << errMsg << "\n";
    }
}

void viewWallet(Database& db, const std::string& username) {
    int balance = db.getBalance(username);
    std::cout << "So diem hien tai: " << balance << "\n";

    std::cout << "Lich su giao dich:\n";
    const auto& logs = db.getTransactions();
    for (const auto& t : logs) {
        if (t.fromWallet == username || t.toWallet == username) {
            std::cout << "[" << t.time << "] ";
            if (t.fromWallet == username)
                std::cout << "-" << t.amount << " diem den " << t.toWallet;
            else
                std::cout << "+" << t.amount << " diem tu " << t.fromWallet;
            std::cout << " | Thanh cong: " << (t.success ? "Co" : "Khong") << " | " << t.note << "\n";
        }
    }
}

// ---------------------
// Chuong tr�nh ch�nh



int main() {
    srand(time(nullptr));
    Database db;
    OTPService otpService;

    db.load();

    std::cout << "=== He thong dang nhap ===\n";

    std::string username, password;
    int loginAttempts = 0;
    bool loggedIn = false;
    User* currentUser = nullptr;


    // -- Thủy làm 
    while (loginAttempts < 3 && !loggedIn) {
        std::cout << "Nhap username: ";
        std::cin >> username;
        std::cout << "Nhap mat khau: ";
        std::cin >> password;

        if (db.checkPassword(username, simpleHash(password))) {
            loggedIn = true;
            currentUser = db.getUser(username);
            std::cout << "Dang nhap thanh cong!\n";

            // 
            if (currentUser->isPasswordAutoGen) {
                std::cout << "Mat khau hien tai la mat khau tu sinh. Vui long doi mat khau ngay.\n";
                changePassword(db, username);
                currentUser->isPasswordAutoGen = false;
                db.save();
            }
            break;
        }
        else {
            std::cout << "Dang nhap that bai. Thu lai.\n";
            loginAttempts++;
        }
    }

    if (!loggedIn) {
        std::cout << "Dang nhap that bai qua nhieu lan. Thoat chuong trinh.\n";
        return 0;
    }

    int choice = -1;
    while (choice != 0) {
        showMenu(currentUser->role == UserRole::Manager);
        std::cout << "Chon chuc nang: ";
        std::cin >> choice;

        switch (choice) {
            /**
Thiếu

user có thể nạp điểm bằng cách nạp tiền
admin có thể nạp điểm cho ví tổng
ví tổng : 10000

user a nạp 10ngan lấy 10 điểm(transaction tu vi tong sang ví user a)
*/
        case 1:
            // Thủy làm
            viewPersonalInfo(db, username);
            break;
        case 2:
        // Thủy làm
            changePassword(db, username);
            db.save();
            break;
        case 3:
        // Win làm
            transferPoints(db, otpService, username);
            break;
        case 4:
        // Win làm
            viewWallet(db, username);
            break;
        case 5:
        // Hùng
            if (currentUser->role == UserRole::Manager) {
                createAccount(db);
            }
            else {
                std::cout << "Khong co quyen truy cap.\n";
            }
            break;
        case 6:
        // Dũng làm
            if (currentUser->role == UserRole::Manager) {
                editUserInfo(db, otpService, username, true);
            }
            else {
                std::cout << "Khong co quyen truy cap.\n";
            }
            break;
        case 0:
            std::cout << "Dang xuat...\n";
            break;
        default:
            std::cout << "Lua chon khong hop le.\n";
        }
    }

    return 0;
}

