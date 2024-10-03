#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <memory>
#include <functional>


struct Transaction {
    std::string type; 
    double amount;
    std::string date;
};


class Account {
private:
    std::string accountNumber;
    std::string ownerName;
    double balance;
    size_t pinHash; 
    std::vector<Transaction> transactions;

public:
    Account(const std::string& accNum, const std::string& name, double initialDeposit, size_t hashedPin)
        : accountNumber(accNum), ownerName(name), balance(initialDeposit), pinHash(hashedPin) {}


    std::string getAccountNumber() const { return accountNumber; }
    std::string getOwnerName() const { return ownerName; }
    double getBalance() const { return balance; }
    size_t getPinHash() const { return pinHash; }

    bool authenticate(const std::string& enteredPin) const {
        size_t enteredPinHash = std::hash<std::string>{}(enteredPin);
        return pinHash == enteredPinHash;
    }

    void deposit(double amount) {
        balance += amount;
        Transaction txn = { "Deposit", amount, getCurrentDateTime() };
        transactions.push_back(txn);
        saveTransaction(txn);
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Deposited $" << amount << " successfully.\n";
    }

    bool withdraw(double amount) {
        if (amount > balance) {
            std::cout << "Insufficient funds. Withdrawal failed.\n";
            return false;
        }
        balance -= amount;
        Transaction txn = { "Withdrawal", amount, getCurrentDateTime() };
        transactions.push_back(txn);
        saveTransaction(txn);
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Withdrew $" << amount << " successfully.\n";
        return true;
    }

    void viewTransactions() const {
        if (transactions.empty()) {
            std::cout << "No transactions found.\n";
            return;
        }
        std::cout << "\n===== Transaction History =====\n";
        for (const auto& txn : transactions) {
            std::cout << txn.date << " - " << txn.type << ": $" 
                      << std::fixed << std::setprecision(2) << txn.amount << "\n";
        }
    }

    void loadTransactions(const std::string& transactionsFile) {
        std::ifstream txnFile(transactionsFile);
        if (!txnFile) {
            return;
        }

        std::string line;
        while (std::getline(txnFile, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string txnAccNum, type, amountStr, date;
            std::getline(ss, txnAccNum, ',');
            if (txnAccNum != accountNumber) continue; 
            std::getline(ss, type, ',');
            std::getline(ss, amountStr, ',');
            std::getline(ss, date, ',');

            Transaction txn;
            txn.type = type;
            try {
                txn.amount = std::stod(amountStr);
            }
            catch (...) {
                txn.amount = 0.0;
            }
            txn.date = date;

            transactions.push_back(txn);
        }
        txnFile.close();
    }

    void saveToFile(const std::string& accountsFile) const {
        std::ofstream outFile(accountsFile, std::ios::app);
        if (!outFile) {
            std::cerr << "Error opening accounts file for writing.\n";
            return;
        }
        outFile << accountNumber << "," << ownerName << "," 
                << std::fixed << std::setprecision(2) << balance << "," 
                << pinHash << "\n";
        outFile.close();
    }

    void addTransaction(const Transaction& txn) {
        transactions.push_back(txn);
        saveTransaction(txn);
    }

    static std::string getCurrentDateTime() {
        std::time_t now = std::time(nullptr);
        std::tm* ltm = std::localtime(&now);
        std::stringstream ss;
        ss << 1900 + ltm->tm_year << "-"
           << std::setw(2) << std::setfill('0') << 1 + ltm->tm_mon << "-"
           << std::setw(2) << std::setfill('0') << ltm->tm_mday << " "
           << std::setw(2) << std::setfill('0') << ltm->tm_hour << ":"
           << std::setw(2) << std::setfill('0') << ltm->tm_min << ":"
           << std::setw(2) << std::setfill('0') << ltm->tm_sec;
        return ss.str();
    }

private:
    void saveTransaction(const Transaction& txn) const {
        std::ofstream txnFile("transactions.txt", std::ios::app);
        if (!txnFile) {
            std::cerr << "Error opening transactions file for writing.\n";
            return;
        }
        txnFile << accountNumber << "," << txn.type << "," 
                << std::fixed << std::setprecision(2) << txn.amount << "," 
                << txn.date << "\n";
        txnFile.close();
    }
};

std::string generateAccountNumber() {
    std::ifstream inFile("account_number.txt");
    int lastNumber = 1000; 
    if (inFile) {
        inFile >> lastNumber;
        inFile.close();
    }

    int newNumber = lastNumber + 1;

    std::ofstream outFile("account_number.txt", std::ios::trunc);
    if (!outFile) {
        std::cerr << "Error opening account number file for writing.\n";
        return std::to_string(newNumber); 
    }
    outFile << newNumber;
    outFile.close();

    return std::to_string(newNumber);
}

bool updateAccountInFile(const std::string& accountsFile, const Account& account) {
    std::ifstream inFile(accountsFile);
    if (!inFile) {
        std::cerr << "Error opening accounts file for reading.\n";
        return false;
    }

    std::vector<std::string> lines;
    std::string line;
    std::string targetAccNum = account.getAccountNumber();

    while (std::getline(inFile, line)) {
        if (line.empty()) {
            lines.push_back(line);
            continue;
        }

        std::stringstream ss(line);
        std::string accNumber;
        std::getline(ss, accNumber, ',');

        if (accNumber == targetAccNum) {
            std::stringstream newLine;
            newLine << account.getAccountNumber() << "," 
                    << account.getOwnerName() << ","
                    << std::fixed << std::setprecision(2) << account.getBalance() << ","
                    << account.getPinHash();
            lines.push_back(newLine.str());
        }
        else {
            lines.push_back(line);
        }
    }
    inFile.close();

    std::ofstream outFile(accountsFile, std::ios::trunc);
    if (!outFile) {
        std::cerr << "Error opening accounts file for writing.\n";
        return false;
    }

    for (const auto& l : lines) {
        outFile << l << "\n";
    }

    outFile.close();
    return true;
}

void createAccount(const std::string& accountsFile) {
    std::string name, pin;
    double initialDeposit;

    std::cout << "Enter your full name: ";
    std::getline(std::cin, name);
    if (name.empty()) {
        std::cout << "Name cannot be empty. Account creation failed.\n";
        return;
    }

    std::cout << "Set a 4-digit PIN: ";
    std::getline(std::cin, pin);

    if (pin.length() != 4 || !std::all_of(pin.begin(), pin.end(), ::isdigit)) {
        std::cout << "Invalid PIN format. Account creation failed.\n";
        return;
    }

    std::cout << "Enter initial deposit amount: $";
    std::cin >> initialDeposit;
    std::cin.ignore();

    if (initialDeposit < 0) {
        std::cout << "Initial deposit cannot be negative. Account creation failed.\n";
        return;
    }

    std::string accNum = generateAccountNumber();
    size_t hashedPin = std::hash<std::string>{}(pin);

    Account newAccount(accNum, name, initialDeposit, hashedPin);
    newAccount.saveToFile(accountsFile);

    Transaction initialTxn = { "Deposit", initialDeposit, Account::getCurrentDateTime() };
    newAccount.addTransaction(initialTxn);

    std::cout << "Account created successfully!\n";
    std::cout << "Your Account Number: " << accNum << "\n";
}

std::unique_ptr<Account> login(const std::string& accountsFile) {
    std::string accNum, enteredPin;
    std::cout << "Enter your Account Number: ";
    std::getline(std::cin, accNum);
    if (accNum.empty()) {
        std::cout << "Account number cannot be empty.\n";
        return nullptr;
    }
    std::cout << "Enter your PIN: ";
    std::getline(std::cin, enteredPin);

    std::ifstream inFile(accountsFile);
    if (!inFile) {
        std::cerr << "Error opening accounts file.\n";
        return nullptr;
    }

    std::string line;
    std::unique_ptr<Account> foundAccount = nullptr;
    while (std::getline(inFile, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string accNumber, owner, balanceStr, pinHashStr;
        std::getline(ss, accNumber, ',');
        std::getline(ss, owner, ',');
        std::getline(ss, balanceStr, ',');
        std::getline(ss, pinHashStr, ',');

        if (accNumber == accNum) {
            double balance = 0.0;
            try {
                balance = std::stod(balanceStr);
            }
            catch (...) {
                balance = 0.0;
            }

            size_t storedPinHash = 0;
            try {
                storedPinHash = std::stoull(pinHashStr);
            }
            catch (...) {
                storedPinHash = 0;
            }

            Account tempAccount(accNumber, owner, balance, storedPinHash);
            if (tempAccount.authenticate(enteredPin)) {
                foundAccount = std::make_unique<Account>(accNumber, owner, balance, storedPinHash);
                foundAccount->loadTransactions("transactions.txt");
                std::cout << "Login successful. Welcome, " << foundAccount->getOwnerName() << "!\n";
                break;
            }
            else {
                break;
            }
        }
    }
    inFile.close();

    if (!foundAccount) {
        std::cout << "Account not found or incorrect PIN.\n";
    }

    return foundAccount;
}

int main() {
    std::string accountsFile = "accounts.txt";
    int choice;

    while (true) {
        std::cout << "\n===== Simple Banking System =====\n";
        std::cout << "1. Create Account\n";
        std::cout << "2. Login to Account\n";
        std::cout << "3. Exit\n";
        std::cout << "Enter your choice (1-3): ";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1:
                createAccount(accountsFile);
                break;
            case 2: {
                std::unique_ptr<Account> account = login(accountsFile);
                if (account) {
                    int subChoice;
                    while (true) {
                        std::cout << "\n===== Account Menu =====\n";
                        std::cout << "1. Deposit Funds\n";
                        std::cout << "2. Withdraw Funds\n";
                        std::cout << "3. Check Balance\n";
                        std::cout << "4. View Transaction History\n";
                        std::cout << "5. Logout\n";
                        std::cout << "Enter your choice (1-5): ";
                        std::cin >> subChoice;
                        std::cin.ignore(); 

                        switch (subChoice) {
                            case 1: { 
                                double amount;
                                std::cout << "Enter amount to deposit: $";
                                std::cin >> amount;
                                std::cin.ignore();
                                if (amount <= 0) {
                                    std::cout << "Invalid amount. Please enter a positive value.\n";
                                }
                                else {
                                    account->deposit(amount);

                                    if (!updateAccountInFile(accountsFile, *account)) {
                                        std::cout << "Error updating account data.\n";
                                    }
                                }
                                break;
                            }
                            case 2: { 
                                double amount;
                                std::cout << "Enter amount to withdraw: $";
                                std::cin >> amount;
                                std::cin.ignore(); 
                                if (amount <= 0) {
                                    std::cout << "Invalid amount. Please enter a positive value.\n";
                                }
                                else {
                                    if (account->withdraw(amount)) {
                                        if (!updateAccountInFile(accountsFile, *account)) {
                                            std::cout << "Error updating account data.\n";
                                        }
                                    }
                                }
                                break;
                            }
                            case 3:
                                std::cout << std::fixed << std::setprecision(2);
                                std::cout << "Current Balance: $" << account->getBalance() << "\n";
                                break;
                            case 4:
                                account->viewTransactions();
                                break;
                            case 5:
                                account->saveToFile(accountsFile);
                                std::cout << "Logged out successfully.\n";
                                account.reset();
                                goto endLogin;
                            default:
                                std::cout << "Invalid choice. Please try again.\n";
                        }
                    }
                    endLogin:
                    ;
                }
                break;
            }
            case 3:
                std::cout << "Exiting Banking System. Goodbye!\n";
                return 0;
            default:
                std::cout << "Invalid choice. Please try again.\n";
        }
    }

    return 0;
}
