#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// ---------------------
// Utils
string getCurrentTime() {
  time_t now = time(nullptr);
  tm *ltm = localtime(&now);
  char buf[20];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
           1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, ltm->tm_hour,
           ltm->tm_min, ltm->tm_sec);
  return string(buf);
}

// H m hash don gian
string simpleHash(const string &input) {
  hash<string> hasher;
  size_t hashed = hasher(input);
  stringstream ss;
  ss << hex << hashed;
  return ss.str();
}

// Sinh mat khau tu dong
string generatePassword(int length = 6) {
  string pwd;
  for (int i = 0; i < length; ++i) {
    pwd += '0' + rand() % 10;
  }
  return pwd;
}

// Sinh OTP (6 chu so)
string generateOTP(int length = 6) { return generatePassword(length); }

// ---------------------
// Class quản lý tài khoản người dùng

enum class UserRole { NormalUser = 0, Manager = 1 };

struct User {
  string username;
  string passwordHash;
  string fullName;
  string phone;
  bool isPasswordAutoGen;
  UserRole role;

  // Serialize ra d ng file: d ng dau | ph n c ch
  string serialize() const {
    return username + "|" + passwordHash + "|" + fullName + "|" + phone + "|" +
           (isPasswordAutoGen ? "1" : "0") + "|" +
           to_string(static_cast<int>(role));
  }

  static User deserialize(const string &line) {
    stringstream ss(line);
    string token;
    User u;
    getline(ss, u.username, '|');
    getline(ss, u.passwordHash, '|');
    getline(ss, u.fullName, '|');
    getline(ss, u.phone, '|');
    getline(ss, token, '|');
    u.isPasswordAutoGen = (token == "1");
    getline(ss, token, '|');
    u.role = (token == "1") ? UserRole::Manager : UserRole::NormalUser;
    return u;
  }
};

struct SumWallet {
  int balance;
  string address;
  SumWallet() {}
  string serialize() const { return address + "|" + address; }
  static SumWallet deserialize(const string &line) {
    stringstream ss(line);
    SumWallet u;
    string balanceStr;
    getline(ss, u.address);
    getline(ss, balanceStr);
    u.balance = stoi(balanceStr);
    return u;
  }

  void addPoint(int amount) {
    if (amount > 0)
      balance += amount;
  }

  void subtractPoint(int amount) {
    if (amount > 0 && balance >= amount)
      balance -= amount;
  }

  int getBalance() const { return balance; }
  string getAddress() const { return address; }
};

// ---------------------
// Lịch sử giao dịch

struct Transaction {
  string fromWallet;
  string toWallet;
  int amount;
  string time;
  bool success;
  string note;

  string serialize() const {
    return fromWallet + "|" + toWallet + "|" + to_string(amount) + "|" + time +
           "|" + (success ? "1" : "0") + "|" + note;
  }

  static Transaction deserialize(const string &line) {
    stringstream ss(line);
    string token;
    Transaction t;
    getline(ss, t.fromWallet, '|');
    getline(ss, t.toWallet, '|');
    getline(ss, token, '|');
    t.amount = stoi(token);
    getline(ss, t.time, '|');
    getline(ss, token, '|');
    t.success = (token == "1");
    getline(ss, t.note, '|');
    return t;
  }
};

// ---------------------

class Database {
private:
  unordered_map<string, User> users; // key=username
  unordered_map<string, pair<string, int>>
      wallets; // key=username, value=balance
  vector<Transaction> transactions;
  SumWallet sumWalletConfig; // Chỉ sử dụng nếu cần

  const string userFile = "user.txt";
  const string walletFile = "wallet.txt";
  const string transactionFile = "transactions.txt";
  const string sumWalletFile = "sumWallet.txt";

public:
  SumWallet getWalletConfig() const { return sumWalletConfig; }

  void addBalanceSumWallet(int amount) { sumWalletConfig.addPoint(amount); }
  void load() {
    users.clear();
    wallets.clear();
    transactions.clear();

    // Load users
    ifstream fin(userFile);
    string line;
    while (getline(fin, line)) {
      if (line.empty())
        continue;
      User u = User::deserialize(line);
      users[u.username] = u;
    }
    fin.close();

    // Load wallets
    fin.open(walletFile);
    while (getline(fin, line)) {
      if (line.empty())
        continue;
      stringstream ss(line);
      string username, address;
      int balance;
      ss >> username >> balance >> address;
      wallets[username] = make_pair(address, balance);
    }
    fin.close();

    // Load transactions
    fin.open(transactionFile);
    while (getline(fin, line)) {
      if (line.empty())
        continue;
      transactions.push_back(Transaction::deserialize(line));
    }
    fin.close();

    // load sum wallet
    ifstream walletFin(sumWalletFile);
    while (getline(walletFin, line)) {
      if (line.empty())
        break;
      sumWalletConfig = SumWallet::deserialize(line);
      break; // Chỉ cần đọc 1 dòng
    }
    walletFin.close();
  }

  void save() {
    // Backup truoc khi luu
    backupFiles();

    // Save users
    ofstream fout(userFile, ios::trunc);
    for (auto it = users.begin(); it != users.end(); ++it) {
      fout << it->second.serialize() << "\n";
    }

    fout.close();

    // Save wallets
    fout.open(walletFile, ios::trunc);
    for (auto it = wallets.begin(); it != wallets.end(); ++it) {
      fout << it->first << " " << it->second.first << " " << it->second.second
           << "\n";
    }

    fout.close();

    // Save transactions
    fout.open(transactionFile, ios::trunc);
    for (const auto &t : transactions) {
      fout << t.serialize() << "\n";
    }
    fout.close();

    // Save Config SumWallet
    fout.open(sumWalletFile, ios::trunc);
    fout << sumWalletConfig.serialize() << "\n";
    fout.close();
  }

  void backupFiles() {
    string backupDir = "backup/";
    system("mkdir -p backup");

    auto backupFile = [&](const string &filename) {
      string cmd = "cp " + filename + " " + backupDir + filename + "." +
                   to_string(time(nullptr));
      system(cmd.c_str());
    };

    backupFile(userFile);
    backupFile(walletFile);
    backupFile(transactionFile);
    backupFile(sumWalletFile);
  }

  // User management
  bool userExists(const string &username) {
    return users.find(username) != users.end();
  }

  void addUser(const User &u, int initialBalance = 0) {
    users[u.username] = u;
    string address = "0x" + to_string(wallets.size() + 1);
    wallets[u.username] = make_pair(address, initialBalance);
  }

  User *getUser(const string &username) {
    auto it = users.find(username);
    if (it == users.end())
      return nullptr;
    return &(it->second);
  }

  bool checkPassword(const string &username, const string &passwordHash) {
    User *u = getUser(username);
    if (!u)
      return false;
    return u->passwordHash == passwordHash;
  }

  void updatePassword(const string &username, const string &newPasswordHash,
                      bool autoGen = false) {
    User *u = getUser(username);
    if (u) {
      u->passwordHash = newPasswordHash;
      u->isPasswordAutoGen = autoGen;
    }
  }

  void updateUserInfo(const string &username, const string &newName,
                      const string &newPhone) {
    User *u = getUser(username);
    if (u) {
      u->fullName = newName;
      u->phone = newPhone;
    }
  }

  int getBalance(const string &username) {
    if (wallets.find(username) == wallets.end())
      return 0;
    return wallets[username].second;
  }

  void setBalance(const string &username, int amount) {
    string address = wallets[username].first;
    if (amount < 0) {
      cout << "So du khong the am.\n";
      return;
    }
    wallets[username] = make_pair(address, amount);
  }

  const vector<Transaction> &getTransactions() const { return transactions; }

  void addTransaction(const Transaction &t) { transactions.push_back(t); }

  // Giao dich chuyen diem (atomic)
  bool transferPoints(const string &fromUser, const string &toUser, int amount,
                      string &errMsg) {
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
    t.fromWallet = wallets[fromUser].first;
    t.toWallet = wallets[toUser].first;
    t.amount = amount;
    t.time = getCurrentTime();
    t.success = true;
    t.note = "Chuyen diem thanh cong";
    string randomOtp = generateOTP();
    string inputOtp;
    cin >> inputOtp;
    while (inputOtp != randomOtp) {
      cout << "Ma OTP khong dung. Vui long nhap lai: ";
      cin >> inputOtp;
    }
    addTransaction(t);

    return true;
  }

  bool depositPoints(const string &username, int amount, string &errMsg) {
    if (!userExists(username)) {
      errMsg = "Nguoi dung khong ton tai.";
      return false;
    };
    sumWalletConfig.subtractPoint(amount);
    string walletAddress = sumWalletConfig.getAddress();
    Transaction t;
    t.fromWallet = walletAddress;
    t.toWallet = wallets[username].first;
    t.amount = amount;
    t.time = getCurrentTime();
    t.success = true;
    t.note = "Chuyen diem thanh cong";
    string randomOtp = generateOTP();
    string inputOtp;
    cin >> inputOtp;
    while (inputOtp != randomOtp) {
      cout << "Ma OTP khong dung. Vui long nhap lai: ";
      cin >> inputOtp;
    }
    addTransaction(t);
    setBalance(username, getBalance(username) + amount);
    return true;
  }
};

// ---------------------
// OTP x c nhan (m  luu tam thoi tr n bo nho)

class OTPService {
private:
  unordered_map<string, string> otpStorage; // username -> OTP
public:
  string createOTP(const string &username) {
    string otp = generateOTP();
    otpStorage[username] = otp;
    return otp;
  }

  bool verifyOTP(const string &username, const string &otp) {
    if (otpStorage.find(username) == otpStorage.end())
      return false;
    if (otpStorage[username] == otp) {
      otpStorage.erase(username);
      return true;
    }
    return false;
  }
};
// ---------------------
// C c chuc nang ch nh

void showMenu(bool isManager) {
  cout << "\n--- MENU ---\n";
  cout << "1. Xem thong tin ca nhan\n";
  cout << "2. Thay doi mat khau\n";
  cout << "3. Chuyen diem toi nguoi dung khac\n";
  cout << "4. Xem so du va lich su giao dich\n";
  if (isManager) {
    cout << "5. Tao tai khoan moi\n";
    cout << "6. Dieu chinh thong tin nguoi dung\n";
    cout << "7. Thêm điểm vào ví tổng\n";
    cout << "8. Nạp điểm vào ví\n";
  }
  cout << "0. Dang xuat\n";
}
// Hùng
void viewPersonalInfo(Database &db, const string &username) {
  User *u = db.getUser(username);
  if (u == nullptr) {
    cout << username << " Nguoi dung khong ton tai.\n";
    return;
  }

  cout << "Username: " << u->username << "\n";
  cout << "Ho ten: " << u->fullName << "\n";
  cout << "Dien thoai: " << u->phone << "\n";
  cout << "Mat khau tu sinh: " << (u->isPasswordAutoGen ? "Co" : "Khong")
       << "\n";
  cout << "Vai tro: "
       << (u->role == UserRole::Manager ? "Quan ly" : "Nguoi dung") << "\n";
}

// Thủy
void changePassword(Database &db, const string &username) {
  string oldPass, newPass, newPass2;
  cout << "Nhap mat khau cu: ";
  cin >> oldPass;
  if (!db.checkPassword(username, simpleHash(oldPass))) {
    cout << "Mat khau cu sai.\n";
    return;
  }
  cout << "Nhap mat khau moi: ";
  cin >> newPass;
  cout << "Nhap lai mat khau moi: ";
  cin >> newPass2;
  if (newPass != newPass2) {
    cout << "Mat khau moi khong khop.\n";
    return;
  }
  db.updatePassword(username, simpleHash(newPass), false);
  cout << "Doi mat khau thanh cong.\n";
}

// Hùng
void createAccount(Database &db) {
  string username, fullName, phone;
  int roleInput;
  bool autoGenPassword;
  cout << "Nhap username (khong duoc sua sau): ";
  cin >> username;
  if (db.userExists(username)) {
    cout << "Tai khoan da ton tai.\n";
    return;
  }
  cout << "Nhap ho ten: ";
  cin.ignore();
  getline(cin, fullName);
  cout << "Nhap so dien thoai: ";
  getline(cin, phone);
  cout << "Chon vai tro (0 - nguoi dung, 1 - quan ly): ";
  cin >> roleInput;

  cout << "Co sinh mat khau tu dong? (1: Co, 0: Khong): ";
  cin >> autoGenPassword;

  string password;
  if (autoGenPassword) {
    password = generatePassword();
    cout << "Mat khau sinh tu dong la: " << password << "\n";
  } else {
    cout << "Nhap mat khau: ";
    cin >> password;
  }
  User u{username,
         simpleHash(password),
         fullName,
         phone,
         autoGenPassword,
         (roleInput == 1) ? UserRole::Manager : UserRole::NormalUser};
  db.addUser(u, 0);
  db.save();
  cout << "Tao tai khoan thanh cong.\n";
}

void editUserInfo(Database &db, OTPService &otpService, const string &username,
                  bool isManager) {
  string targetUser;
  if (isManager) {
    cout << "Nhap username cua nguoi dung muon chinh sua: ";
    cin >> targetUser;
    if (!db.userExists(targetUser)) {
      cout << "Nguoi dung khong ton tai.\n";
      return;
    }
  } else {
    targetUser = username;
  }
  User *u = db.getUser(targetUser);
  if (!u)
    return;

  cout << "Thong tin hien tai:\n";
  cout << "Ho ten: " << u->fullName << "\n";
  cout << "Dien thoai: " << u->phone << "\n";

  cout << "Nhap ho ten moi (de trong de giu nguyen): ";
  cin.ignore();
  string newName;
  getline(cin, newName);
  if (newName.empty())
    newName = u->fullName;

  cout << "Nhap so dien thoai moi (de trong de giu nguyen): ";
  string newPhone;
  getline(cin, newPhone);
  if (newPhone.empty())
    newPhone = u->phone;

  // Gui OTP
  string otp = otpService.createOTP(targetUser);
  cout << "Ma OTP da gui toi nguoi dung (gi? l?p): " << otp << "\n";
  cout << "Thong bao thay doi: Ho ten va so dien thoai.\n";
  cout << "Nhap ma OTP de xac nhan thay doi: ";
  string otpInput;
  getline(cin, otpInput);
  if (!otpService.verifyOTP(targetUser, otpInput)) {
    cout << "Ma OTP khong dung. Huy thao tac.\n";
    return;
  }
  db.updateUserInfo(targetUser, newName, newPhone);
  db.save();
  cout << "Cap nhat thong tin thanh cong.\n";
}

void addMoneySumWallet(Database &db, int amount) {
  if (amount <= 0) {
    cout << "So tien nap phai > 0.\n";
    return;
  }
  db.addBalanceSumWallet(amount);
  db.save();
  cout << "Nap tien vao vi tong thanh cong.\n";
}

void transferPoints(Database &db, OTPService &otpService,
                    const string &username) {
  string toUser;
  int amount;
  cout << "Nhap username nguoi nhan diem: ";
  cin >> toUser;
  cout << "Nhap so diem can chuyen: ";
  cin >> amount;
  if (toUser == username) {
    cout << "Khong the chuyen diem cho chinh ban than.\n";
    return;
  }

  string otp = otpService.createOTP(username);
  cout << "Ma OTP da gui toi ban (gi? l?p): " << otp << "\n";
  cout << "Nhap ma OTP de xac nhan giao dich: ";
  string otpInput;
  cin.ignore();
  getline(cin, otpInput);
  if (!otpService.verifyOTP(username, otpInput)) {
    cout << "Ma OTP khong dung. Huy giao dich.\n";
    return;
  }

  string errMsg;
  if (db.transferPoints(username, toUser, amount, errMsg)) {
    db.save();
    cout << "Chuyen diem thanh cong.\n";
  } else {
    cout << "Chuyen diem that bai: " << errMsg << "\n";
  }
}

void depositPoints(Database &db, const string &username) {
  int amount;
  cin >> amount;
  if (amount <= 0) {
    cout << "So diem nap phai > 0.\n";
    return;
  }
  string errMsg;
  if (db.depositPoints(username, amount, errMsg)) {
    db.save();
    cout << "Nap diem thanh cong.\n";
  } else {
    cout << "Nap diem that bai: " << errMsg << "\n";
    return;
  }
}

void viewHitoryTransactions(Database &db, const string &username) {
  int balance = db.getBalance(username);
  cout << "So diem hien tai: " << balance << "\n";

  cout << "Lich su giao dich:\n";
  const auto &logs = db.getTransactions();
  for (const auto &t : logs) {
    if (t.fromWallet == username || t.toWallet == username) {
      cout << "[" << t.time << "] ";
      if (t.fromWallet == username)
        cout << "-" << t.amount << " diem den " << t.toWallet;
      else
        cout << "+" << t.amount << " diem tu " << t.fromWallet;
      cout << " | Thanh cong: " << (t.success ? "Co" : "Khong") << " | "
           << t.note << "\n";
    }
  }
}

// ---- state
struct StateManagement {
  bool isLoggedIn;
  User *currentUser;
  StateManagement() {
    isLoggedIn = false;
    currentUser = nullptr;
  }
};

// ---------------------
// Chương trình chính

// -- Thủy làm: login
bool do_login(StateManagement &state, Database &db, string username,
              string password) {
  cout << "Nhap username: ";
  cin >> username;
  cout << "Nhap mat khau: ";
  cin >> password;
  if (db.checkPassword(username, simpleHash(password))) {
    state.isLoggedIn = true;
    state.currentUser = db.getUser(username);
    cout << "Dang nhap thanh cong!\n";
    //
    if (state.currentUser->isPasswordAutoGen) {
      cout << "Mat khau hien tai la mat khau tu sinh. Vui long doi mat "
              "khau ngay.\n";
      changePassword(db, username);
      state.currentUser->isPasswordAutoGen = false;
      db.save();
    }
    return true;
  }
  cout << "Dang nhap that bai. Thu lai.\n";
  return false;
}

int main() {
  srand(time(nullptr));
  Database db;
  OTPService otpService;
  StateManagement state;

  db.load();

  cout << "=== He thong dang nhap ===\n";

  string username, password;
  int loginAttempts = 0;

  while (loginAttempts < 3 && !state.isLoggedIn) {
    while (do_login(state, db, username, password) == false) {
      loginAttempts++;
    }
  }

  if (!state.isLoggedIn) {
    cout << "Dang nhap that bai qua nhieu lan. Thoat chuong trinh.\n";
    return 0;
  }

  int choice = -1;
  while (choice != 0) {
    showMenu(state.currentUser->role == UserRole::Manager);
    cout << "Chon chuc nang: ";
    cin >> choice;

    switch (choice) {

    case 1:
      // Hùng làm
      viewPersonalInfo(db, state.currentUser->username);
      break;
    case 2:
      // Thủy làm
      changePassword(db, username);
      break;
    case 3:
      // Win làm
      transferPoints(db, otpService, username);
      break;
    case 4:
      // Win làm
      viewHitoryTransactions(db, username);
      break;
    case 5:
      // Hùng
      if (state.currentUser->role == UserRole::Manager) {
        createAccount(db);
      } else {
        cout << "Khong co quyen truy cap.\n";
      }
      break;
    case 6:
      // Hùng
      if (state.currentUser->role == UserRole::NormalUser) {
        editUserInfo(db, otpService, username, true);
      } else {
        cout << "Khong co quyen truy cap.\n";
      }
      break;

    case 7:
      if (state.currentUser->role == UserRole::Manager) {
        int amount;
        cout << "Nhap so tien muon nap vao vi tong: ";
        cin >> amount;
        if (amount <= 0) {
          cout << "So tien nap phai > 0.\n";
          break;
        }
        addMoneySumWallet(db, amount);
      } else {
        cout << "Khong co quyen truy cap.\n";
      }
      break;
    case 8:
      // Win làm
      depositPoints(db, username);
      break;
    case 0:
      cout << "Dang xuat...\n";
      break;
    default:
      cout << "Lua chon khong hop le.\n";
    }
  }

  return 0;
}
