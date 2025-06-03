#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <sstream> 
using namespace std;

const int MAX_ITEMS = 100;
const int MAX_ACCOUNTS = 5;
const int MAX_STOCK_TRANSACTIONS = 500;

struct Item {
    string code;
    string name;
    string category;
    int quantity;
    string supplier;
    double price;
    string date;
};

struct Admin {
    string email;
    string name;
    string dateOfBirth;
    string gender;
    string telephoneNumber;
    string username;
    string password;
};

struct Transaction {
    string transactionID;
    string itemCode;
    string itemName;      
    string type;          
    int quantityChange;   
    string date;           
};

Item items[MAX_ITEMS];
Admin admin[MAX_ACCOUNTS];
Transaction transactions[MAX_STOCK_TRANSACTIONS];

ifstream ifs;
ofstream ofs;

int itemsCount = 0;
int accountsCount = 0;
int transactionsCount = 0;
bool logedIn = false;
string logedInUser = "GUEST";
long long lastStockTransactionNumber = 0;

void loadAccountsFromFile() {
    ifs.open("admin.txt");
    accountsCount = 0;

    if (ifs.is_open()) {
        string line;
        while (getline(ifs, line)) {
            if (line.find("Biodata Admin") != string::npos) {
                getline(ifs, admin[accountsCount].username, ',');
                ifs.ignore(); 
                getline(ifs, admin[accountsCount].password);
                getline(ifs, admin[accountsCount].email);
                getline(ifs, admin[accountsCount].name);
                getline(ifs, admin[accountsCount].dateOfBirth);
                getline(ifs, admin[accountsCount].gender);
                getline(ifs, admin[accountsCount].telephoneNumber);
                accountsCount++;
            }
        }
        ifs.close();
    }
}

void loadItemsFromFile() {
    ifstream ifs("list_items.txt");
    itemsCount = 0;

    if (ifs.is_open()) {
        while (getline(ifs, items[itemsCount].code, ',') && 
               getline(ifs >> ws, items[itemsCount].name)) {
            itemsCount++;
        }
        ifs.close();
    } else {
        cout << "Failed to open file!" << endl;
        return;
    }

    ofs.open("list_items.txt");
    if (ofs.is_open()) {
        for (int i = 0; i < itemsCount; i++) {
            ofs << items[i].code << ", " << items[i].name << endl;
        }
        ofs.close();
    } else {
        cout << "Failed to open file!" << endl;
    }
}


bool isUsernameDuplicate(string username) {
    ifstream ifs("admin.txt");
    string line;

    while (getline(ifs, line)) {
        if (line.find("Biodata Admin") != string::npos) {
            getline(ifs, line); 

            string existingUsername = "";
            for (int i = 0; i < line.length(); ++i) {
                if (line[i] == ',') break;
                existingUsername += line[i];
            }

            if (existingUsername == username) {
                return true;
            }

            for (int i = 0; i < 5; ++i) {
                getline(ifs, line);
            }
        }
    }

    return false;
}

void registerAccount() {
    loadAccountsFromFile();

    ofs.open("admin.txt", ios::app);
    if (!ofs.is_open()) {
        cout << "Failed to open admin file." << endl;
        return;
    }
    else {
        cout << "Enter email: ";
        cin >> admin[accountsCount].email;
        cout << "Enter Full Name: ";
        getline(cin >> ws, admin[accountsCount].name);
        cout << "Date of Birth (DD-MM-YYYY): ";
        getline(cin >> ws, admin[accountsCount].dateOfBirth);
        cout << "Gender: ";
        getline(cin >> ws, admin[accountsCount].gender);
        cout << "Telephone Number: ";
        getline(cin >> ws, admin[accountsCount].telephoneNumber);
        cout << "Biodata has been entered";
        system("pause");
        system("cls");
        
        string newUsername;
        do {
            cout << "Enter username: ";
            cin >> newUsername;

            if (isUsernameDuplicate(newUsername)) {
                cout << "Username already exist. Please choose a different one!" << endl;
            }
            else {
                break;
            }
        }while (true);

        admin[accountsCount].username = newUsername;
        cout << "Enter password: ";
        cin >> admin[accountsCount].password;

        ofs << "[" << accountsCount + 1 << "]" << endl;
        ofs << "Biodata Admin " << accountsCount + 1 << ":" << endl;
        ofs << admin[accountsCount].username << ", " << admin[accountsCount].password << endl;
        ofs << admin[accountsCount].email << endl;
        ofs << admin[accountsCount].name << endl;
        ofs << admin[accountsCount].dateOfBirth << endl;
        ofs << admin[accountsCount].gender << endl;
        ofs << admin[accountsCount].telephoneNumber << endl;
        ofs.close();
        cout << "Registration successful.\n";
        system("pause");
        system("cls");
        accountsCount++;
        return;
    }
}

string getCurrentDateForTransaction() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buffer[11];
    sprintf(buffer, "%02d-%02d-%04d", ltm->tm_mday, 1 + ltm->tm_mon, 1900 + ltm->tm_year);
    return string(buffer);
}


string longLongToString(long long n) {
    if (n == 0) {
        return "0";
    }

    char buffer[21];
    int index = 0;
    bool isNegative = false;

    if (n < 0) {
        isNegative = true;
        n = -n;
    }

    while (n > 0) {
        int digit = n % 10;
        buffer[index] = '0' + digit;
        index++;
        n /= 10;
    }

    if (isNegative) {
        buffer[index] = '-';
        index++;
    }

    string result = "";
    for (int i = index - 1; i >= 0; i--) {
        result += buffer[i];
    }

    return result;
}


void loadLastStockTransactionNumber() {
    ifstream configFile("config.txt");
    lastStockTransactionNumber = 0;

    if (configFile.is_open()) {
        string line;
        while (getline(configFile, line)) {
            char key[] = "LAST_STOCK_TX_ID=";
            int i = 0;
            bool match = true;

            while (key[i] != '\0') {
                if (line[i] != key[i]) {
                    match = false;
                    break;
                }
                i++;
            }

            if (match) {
                string valueStr = "";
                while (line[i] != '\0') {
                    valueStr += line[i];
                    i++;
                }

                long long tempNum = 0;
                bool validNum = true;

                if (valueStr == "") {
                    validNum = false;
                }

                for (int j = 0; valueStr[j] != '\0'; j++) {
                    if (valueStr[j] >= '0' && valueStr[j] <= '9') {
                        tempNum = tempNum * 10 + (valueStr[j] - '0');
                    } else {
                        validNum = false;
                        break;
                    }
                }

                if (validNum) {
                    lastStockTransactionNumber = tempNum;
                }

                break;
            }
        }

        configFile.close();
    } else {
        cout << "Failed to open config file!" << endl;
    }
}


void saveLastStockTransactionNumber() {
    ofstream configFile("config.txt");
    if (configFile.is_open()) {
        configFile << "LAST_STOCK_TX_ID=" << lastStockTransactionNumber << endl;
        configFile.close();
    } else {
        cout << "Error: Could not save to config.txt" << endl;
    }
}

void loadTransactionsFromFile() {
    ifstream ifs("stock_transactions.txt");
    transactionsCount = 0; 
    if (ifs.is_open()) {
        string line;
        while (transactionsCount < MAX_STOCK_TRANSACTIONS && getline(ifs, line)) {
            stringstream ss(line);
            string part;

            getline(ss, transactions[transactionsCount].transactionID, ',');
            getline(ss, transactions[transactionsCount].itemCode, ',');
            getline(ss, transactions[transactionsCount].itemName, ',');
            getline(ss, transactions[transactionsCount].type, ',');

            string qtyStr;
            getline(ss, qtyStr, ',');
            try {
                transactions[transactionsCount].quantityChange = stoi(qtyStr);
            } catch (const std::invalid_argument& ia) {
                transactions[transactionsCount].quantityChange = 0; 
            }

            getline(ss, transactions[transactionsCount].date, ',');
            transactionsCount++;
        }
        ifs.close();
    } 
}


void saveTransactionsToFile() {
    ofs.open("stock_transactions.txt");
    if (ofs.is_open()) {
        for (int i = 0; i < transactionsCount; ++i) {
            ofs << transactions[i].transactionID << ","
                << transactions[i].itemCode << ","
                << transactions[i].itemName << ","
                << transactions[i].type << ","
                << transactions[i].quantityChange << ","
                << transactions[i].date << endl;
        }
        ofs.close();
    } else {
        cout << "Error: Could not save transactions to stock_transactions.txt" << endl;
    }
}

void recordTransaction(string itemCode, string itemName, string type, int quantityChange, string transactionDate) {
    loadTransactionsFromFile();

    if (transactionsCount >= MAX_STOCK_TRANSACTIONS) {
        cout << "Error: Transaction log is full. Cannot record new transaction." << endl;
        return;
    }
    lastStockTransactionNumber++;
    transactions[transactionsCount].transactionID = "TXN-" + longLongToString(lastStockTransactionNumber);
    transactions[transactionsCount].itemCode = itemCode;
    transactions[transactionsCount].itemName = itemName;
    transactions[transactionsCount].type = type;
    transactions[transactionsCount].quantityChange = quantityChange;
    transactions[transactionsCount].date = transactionDate;

    transactionsCount++;
    saveTransactionsToFile();
    saveLastStockTransactionNumber();
}

void receiptOfGoods() {
    loadItemsFromFile(); 

    if (itemsCount == 0) {
        cout << "No items data yet." << endl;
    } else {
        cout << "=== Items Available ===" << endl;
        for (int i = 0; i < itemsCount; i++) {
            cout << i + 1 << ". " << items[i].name << " (" << items[i].code << ")" << endl;
        }
        string itemCode;
        bool found = false;
        cout << "=== Goods receipt (goods enter the warehouse) ===" << endl;
        cout << "Enter the received Item Code: ";
        cin >> itemCode;

        for (int i = 0; i < itemsCount; i++) {
            if (items[i].code == itemCode) {
                found = true;

                string itemName = "items/" + items[i].code + ".txt";
                ifs.open(itemName);
                if (ifs.is_open()) {
                    ifs >> items[i].code; 
                    getline(ifs >> ws, items[i].name);
                    getline(ifs >> ws, items[i].category);
                    ifs >> items[i].quantity;
                    getline(ifs >> ws, items[i].supplier);
                    ifs >> items[i].price;
                    getline(ifs >> ws, items[i].date);
                    ifs.close();
                }

                cout << "Goods receipt for " << items[i].name 
                     << " (current stock: " << items[i].quantity << ")" << endl;

                int qtyReceived;
                cout << "Quantity of incoming goods: ";
                cin >> qtyReceived;

                if (qtyReceived <= 0) {
                    cout << "The number of items must be greater than 0!" << endl;
                    return;
                }

                string supplierName;
                cout << "Supplier name: ";
                getline(cin >> ws, supplierName);  

                items[i].quantity += qtyReceived;

                string fileName = "items/" + items[i].code + ".txt";
                ofstream ofs(fileName);
                if (ofs.is_open()) {
                    ofs << items[i].code << endl;
                    ofs << items[i].name << endl;
                    ofs << items[i].category << endl;
                    ofs << items[i].quantity << endl;
                    ofs << items[i].supplier << endl;
                    ofs << items[i].price << endl;
                    ofs << items[i].date << endl;
                    ofs.close();
                }

                string inputDate;
                cout << "Enter transaction date (DD-MM-YYYY) or type 'auto' for current date: ";
                cin >> inputDate; 
                if (inputDate == "auto") {
                    inputDate = getCurrentDateForTransaction();
                }
                recordTransaction(items[i].code, items[i].name, "IN", qtyReceived, inputDate);
                cout << "Stock updated successfully!" << endl;
                cout << "New stock for " << items[i].name << ": " << items[i].quantity << endl;
                cout << "Received from supplier: " << supplierName << endl;
                break;  
            }
        }

        if (!found) {
            cout << "Item with code \"" << itemCode << "\" not found!" << endl;
        }
    }
}

void dispendingOfGoods() {
    loadItemsFromFile();

    if (itemsCount == 0) {
        cout << "No items data" << endl;
        return;
    }
    else {
        cout << "=== Items Available ===" << endl;
        for (int i = 0; i < itemsCount; i++) {
            cout << i + 1 << ". " << items[i].name << " (" << items[i].code << ")" << endl;
        }
        string inputItemCode;
        cout << "=== Goods Release (Goods Out of Warehouse) ===" << endl;
        cout << "Enter the issued item code: ";
        cin >> inputItemCode;

        for (int i = 0; i < itemsCount; i++) {
            bool found = false;
            if (items[i].code == inputItemCode) {
                found = true;

                string itemName = "items/" + items[i].code + ".txt";
                ifs.open(itemName);
                if (ifs.is_open()) {
                    ifs >> items[i].code; 
                    getline(ifs >> ws, items[i].name);
                    getline(ifs >> ws, items[i].category);
                    ifs >> items[i].quantity;
                    getline(ifs >> ws, items[i].supplier);
                    ifs >> items[i].price;
                    getline(ifs >> ws, items[i].date);
                    ifs.close();
                }

                cout << "Goods to be issued " << items[i].name << ". Current stock: " << items[i].quantity << endl;
                int qtyIssued;
                cout << "Quantity of outgoing goods: ";
                cin >> qtyIssued;

                if (qtyIssued <= 0) {
                    cout << "The number of outgoing goods must be greater than 0." << endl;
                    return;
                }
                else if (qtyIssued > items[i].quantity) {
                    cout << "Insufficient stock! Stock available only " << items[i].quantity << "." << endl;
                    return;
                }
                else {
                    string destination;
                    cout << "Purpose of spending: ";
                    getline(cin >> ws, destination);

                    items[i].quantity -= qtyIssued;

                    string fileName = "items/" + items[i].code + ".txt";
                    ofstream ofs(fileName);
                    if (ofs.is_open()) {
                        ofs << items[i].code << endl;
                        ofs << items[i].name << endl;
                        ofs << items[i].category << endl;
                        ofs << items[i].quantity << endl;
                        ofs << items[i].supplier << endl;
                        ofs << items[i].price << endl;
                        ofs << items[i].date << endl;
                        ofs.close();
                    }

                    string inputDate;
                    cout << "Enter transaction date (DD-MM-YYYY) or type 'auto' for current date: ";
                    cin >> inputDate; 
                    if (inputDate == "auto") {
                        inputDate = getCurrentDateForTransaction();
                    }
                    recordTransaction(items[i].code, items[i].name, "OUT", qtyIssued, inputDate);
                    cout << "Expenditure of goods in the amount of " << qtyIssued << " successfully!" << endl;
                    cout << "Current stock: " << items[i].quantity << "." << endl;
                }
            }
            else {
                cout << "Item with code " << inputItemCode << " not found" << endl;
                return;
            }
        }
    }
}

void itemsStockManagement() {
    int choice;

    do {
        cout << "==== Items Stock Management ====" << endl;
        cout << "1. Receipt of Goods" << endl;
        cout << "2. Dispending of Goods" << endl;
        cout << "3. Exit" << endl;
        cout << "Choose: ";
        cin >> choice;

        switch(choice) {
            case 1:
                receiptOfGoods();
                system("pause");
                system("cls");
                break;
            case 2:
                dispendingOfGoods();
                system("pause");
                system("cls");
                break;
            case 3: 
                return;
            default: 
                cout << "Invalid choice!" << endl;
                system("pause");
                system("cls");
                break;
        }
    }while (true);
}

void addNewItem() {
    loadItemsFromFile();
    if (accountsCount > MAX_ITEMS) {
        cout << "Warehouse is full!" << endl;
        system("pause");
        system("cls");
        return;
    }
    else {
       string itemCodeInput;
        cout << "Code of item: ";
        cin >> itemCodeInput;

        bool isDuplicate = false;
        for (int i = 0; i < itemsCount; i++) {
            if (items[i].code == itemCodeInput) {
                isDuplicate = true;
                break;
            }
        }

        if (isDuplicate) {
            cout << "Item code already exists! Use a different code." << endl;
            system("pause");
            system("cls");
            return;
        }

        items[itemsCount].code = itemCodeInput;
        cout << "Name of item: ";
        getline(cin >> ws, items[itemsCount].name);

        ofs.open("list_items.txt", ios::app);
        ofs << items[itemsCount].code << ", " << items[itemsCount].name;
        ofs.close();

        cout << "Category: ";
        getline(cin >> ws, items[itemsCount].category);
        cout << "Quantity: ";
        cin >> items[itemsCount].quantity;
        cout << "Supplier: ";
        getline(cin >> ws,  items[itemsCount].supplier);
        cout << "Price: ";
        cin >> items[itemsCount].price;
        cout << "Date (DD-MM-YYYY): ";
        cin >> items[itemsCount].date;

        string itemsData = "items/" + items[itemsCount].code + ".txt";
        ofs.open(itemsData);
        ofs << items[itemsCount].code << endl;
        ofs << items[itemsCount].name << endl;
        ofs << items[itemsCount].category << endl;
        ofs << items[itemsCount].quantity << endl;
        ofs << items[itemsCount].supplier << endl;
        ofs << items[itemsCount].price << endl;
        ofs << items[itemsCount].date << endl;
        ofs.close();

        itemsCount++;
        loadItemsFromFile();
    }
}

void editItem() {
    string selectItemEdit;
    loadItemsFromFile();

    if (itemsCount == 0) {
        cout << "No items data." << endl;
        return;
    }

    cout << "=== Items Available ===" << endl;
    for (int i = 0; i < itemsCount; i++) {
        cout << i + 1 << ". " << items[i].name << " (" << items[i].code << ")" << endl;
    }

    cout << "Enter item code to edit: ";
    cin >> selectItemEdit;

    bool found = false;

    for (int i = 0; i < itemsCount; i++) {
        if (items[i].code == selectItemEdit) {
            found = true;

            cout << "Enter new name: ";
            getline(cin >> ws, items[i].name);
            cout << "Enter new category: ";
            getline(cin >> ws, items[i].category);
            cout << "Enter new quantity: ";
            cin >> items[i].quantity;
            cout << "Enter new supplier: ";
            getline(cin >> ws, items[i].supplier);
            cout << "Enter new price: ";
            cin >> items[i].price;
            cout << "Enter new date (DDMMYYYY): ";
            cin >> items[i].date;
            
            string itemFile = "items/" + items[i].code + ".txt";
            ofs.open(itemFile);
            if (ofs.is_open()) {
                ofs << items[i].code << endl;
                ofs << items[i].name << endl;
                ofs << items[i].category << endl;
                ofs << items[i].quantity << endl;
                ofs << items[i].supplier << endl;
                ofs << items[i].price << endl;
                ofs << items[i].date << endl;
                ofs.close();
            }

            break;
        }
    }

    if (!found) {
        cout << "Item with code '" << selectItemEdit << "' not found." << endl;
        return;
    }

    ofs.open("list_items.txt");
    if (ofs.is_open()) {
        for (int i = 0; i < itemsCount; i++) {
            ofs << items[i].code << ", " << items[i].name << endl;
        }
        ofs.close();
        cout << "Item updated successfully." << endl;
    } else {
        cout << "Failed to update list_items.txt!" << endl;
    }
}

void deleteItem() {
    loadItemsFromFile();

    if (itemsCount == 0) {
        cout << "No items data" << endl;
        return; 
    }

    string selectItemDelete;
    bool found = false;

    cout << "=== Items Available ===" << endl;
    for (int i = 0; i < itemsCount; i++) {
        cout << i + 1 << ". " << items[i].name << " (" << items[i].code << ")" << endl;
    }

    cout << "Enter item code to delete: ";
    cin >> selectItemDelete;

    for (int i = 0; i < itemsCount; i++) {
        if (items[i].code == selectItemDelete) {
            found = true;
            char confirm;

            cout << "You will delete " << items[i].name << " (Stock: " << items[i].quantity << ")" << endl;
            cout << "Are you sure you want to delete this item from the master data? (y/n): ";
            cin >> confirm;

            if (confirm == 'Y' || confirm == 'y') {
                string filenameToDelete = "items/" + items[i].code + ".txt";
                if (remove(filenameToDelete.c_str()) == 0) {
                    cout << "File " << filenameToDelete << " Item deleted successfully!" << endl;
                } else {
                    cout << "Failed to deleted file" << filenameToDelete << endl;
                }

                for (int j = i; j < itemsCount - 1; ++j) {
                    items[j] = items[j + 1];
                }

                itemsCount--;
                cout << "Item deleted successfully!" << endl;
            }
            else if (confirm == 'N' || confirm == 'n') {
                cout << "Deletion cancelled" << endl;
            }
            else {
                cout << "Invalid choice! Input 'Y' or 'N'." << endl;
            }
            break; 
        }
    }

    if (!found) {
        cout << "Item with code \"" << selectItemDelete << "\" not found." << endl;
    } 
    else {
        ofs.open("list_items.txt");
        if (ofs.is_open()) {
            for (int i = 0; i < itemsCount; i++) {
                ofs << items[i].code << ", " << items[i].name << endl;
            }
            ofs.close();
        } else {
            cout << "Failed to open file for saving." << endl;
        }
    }
}

void sortItemsByName() {
    for (int i = 0; i < itemsCount - 1; i++) {
        for (int j = 0; j < itemsCount - i - 1; j++) {
            if (items[j].name > items[j + 1].name) {
                Item temp = items[j];
                items[j] = items[j + 1];
                items[j + 1] = temp;
            }
        }
    }
}

void sortItemsByCode() {
    for (int i = 0; i < itemsCount - 1; i++) {
        for (int j = 0; j < itemsCount - i - 1; j++) {
            if (items[j].code > items[j + 1].code) {
                Item temp = items[j];
                items[j] = items[j + 1];
                items[j + 1] = temp;
            }
        }
    }
}

void sortItemsByPrice() {
    for (int i = 0; i < itemsCount - 1; i++) {
        for (int j = 0; j < itemsCount - i - 1; j++) {
            if (items[j].price > items[j + 1].price) {
                Item temp = items[j];
                items[j] = items[j + 1];
                items[j + 1] = temp;
            }
        }
    }
}

void displayItems() {
    cout << "--- List of All Items ---" << endl;
    cout << left 
         << setw(10) << "Code" << "| "
         << setw(25) << "Name of Item" << "| "
         << setw(15) << "Category" << "| "
         << right << setw(10) << "Quantity" << " | "
         << right << setw(15) << "Supplier" << " | "
         << right << setw(12) << "Price" << " | "
         << left  << setw(12) << "Date" << "|" << endl;
    cout << string(115, '-') << endl;

    for (int i = 0; i < itemsCount; ++i) {
        cout << left 
             << setw(10) << items[i].code << "| "
             << setw(25) << items[i].name << "| "
             << setw(15) << items[i].category << "| "
             << right << setw(10) << items[i].quantity << " | "
             << right << setw(15) << items[i].supplier << " | "
             << right << setw(12) << fixed << setprecision(2) << items[i].price << " | "
             << left  << setw(12) << items[i].date << "|" << endl;
    }
    cout << string(115, '-') << endl;
}

void viewAllItems() {
    loadItemsFromFile();
    if (itemsCount == 0) {
        cout << "No items data available." << endl;
        return;
    }

    cout << "\n[Default View - Unsorted Items]\n" << endl;
    displayItems();

    do {
         cout << "\nChoose sorting option:\n";
        cout << "1. Sort by Name\n";
        cout << "2. Sort by Code\n";
        cout << "3. Sort by Price\n";
        cout << "4. Exit\n";
        cout << "Enter choice: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1:
                sortItemsByName();
                cout << "\n[Sorted by Name]\n" << endl;
                displayItems();
                break;
            case 2:
                sortItemsByCode();
                cout << "\n[Sorted by Code]\n" << endl;
                displayItems();
                break;
            case 3:
                sortItemsByPrice();
                cout << "\n[Sorted by Price]\n" << endl;
                displayItems();
                break;
            case 4:
                return;
            default:
                cout << "Invalid choice.\n";
        }
    }while (true);
   
}

void itemsMasterDataManagement() {
    int choice;

    do {
        cout << "==== Items Master Data Management ====" << endl;
        cout << "1. Add new item" << endl;
        cout << "2. Edit item information" << endl;
        cout << "3. Delete Item" << endl;
        cout << "4. View all items" << endl;
        cout << "5. Exit" << endl;
        cout << "Choose: ";
        cin >> choice;

        switch(choice) {
            case 1:
                addNewItem();
                system("pause");
                system("cls");
                break;
            case 2:
                editItem();
                system("pause");
                system("cls");
                break;
            case 3:
                deleteItem();
                system("pause");
                system("cls");
                break;
            case 4:
                viewAllItems();
                system("pause");
                system("cls");
                break;
            case 5:
                return;
            default:
                cout << "Invalid choice!" << endl;
                system("pause");
                system("cls");
        }
    }while (true);
}

void viewTransactionHistory() {
    loadTransactionsFromFile();
    if (transactionsCount == 0) {
        cout << "No transaction history found." << endl;
        return;
    }
    cout << "--- Transaction History ---" << endl;
    cout << left
         << setw(12) << "TXN ID" << "| " << setw(10) << "Item Code" << "| "
         << setw(20) << "Item Name" << "| " << setw(5) << "Type" << "| "
         << setw(8) << "Qty" << "| " << setw(12) << "Date" << "| " << endl;
    cout << string(105, '-') << endl;
    for (int i = 0; i < transactionsCount; i++) {
        cout << left
             << setw(12) << transactions[i].transactionID << "| "
             << setw(10) << transactions[i].itemCode << "| "
             << setw(20) << transactions[i].itemName << "| "
             << setw(5) << transactions[i].type << "| "
             << setw(8) << transactions[i].quantityChange << "| "
             << setw(12) << transactions[i].date << "| " << endl;
    }
    cout << string(105, '-') << endl;
}

void reportsAndMonitoringMenu() {
    int selectMenu;

    do {
        cout << "=== Reports & Monitoring ===" << endl
             << "1. Stock transaction history report" << endl
             << "2. Exit" << endl
             << "Select menu: ";
        cin >> selectMenu;

        switch(selectMenu) {
            case 1:
                viewTransactionHistory();
                break;
            case 2:
                return;
            default: 
                cout << "Invalid choice" << endl;
        }
    }while (true);
}

void mainMenu() {
    int menu;

    do {
        cout << "+=====================================+" << endl
             << "|     WAREHOUSE MANAGEMENT SYSTEM     |" << endl
             << "+=====================================+" << endl
             << "1. Items Stock Management" << endl
             << "2. Items Master Data Management" << endl
             << "3. Reports & Monitoring" << endl
             << "4. Exit" << endl
            << "Menu: ";
        cin >> menu;

        switch(menu) {
            case 1:
                itemsStockManagement();
                system("pause");
                system("cls");
                break;
            case 2:
                itemsMasterDataManagement();
                system("pause");
                system("cls");
                break;
            case 3:
                reportsAndMonitoringMenu();
                system("pause");
                system("cls");
                break;
            case 4: 
                break;
                return;
            default:
                cout << "Invalid choice!" << endl;
                system("pause");
                system("cls");
                break;
        }
    }while (true);
}

void login(int attempts, string fileName) {
    loadAccountsFromFile();
    int selectAccount;

    if (attempts == 0) {
        cout << "Too many tries! Try again later." << endl;
        system("pause");
        system("cls");
        return;
    }
    else {
        if (accountsCount == 0) {
        cout << "Admin account is empty." << endl;
        system("pause");
        system("cls");
        }
        else {
            ifs.open("admin.txt");
            if (ifs.is_open()) {
                cout << "      List of Admin Account      " << endl;
                for (int i = 0; i < accountsCount; i++) {
                    cout << i + 1 << ". " << admin[i].username << endl;
                }

                cout << "Select account: ";
                cin >> selectAccount;

                int selectIndex = selectAccount - 1;
                string inputEmail, inputPassword;

                cout << "Login for account " << admin[selectIndex].username << endl;
                cout << "Enter email: ";
                cin >> inputEmail;
                cout << "Enter password: ";
                cin >> inputPassword;

                if (admin[selectIndex].email == inputEmail && admin[selectIndex].password == inputPassword) {
                    cout << "Successfull login! Welcome " << admin[selectIndex].username << endl;
                    logedIn = true;
                    logedInUser = admin[selectIndex].username;
                    system("pause");
                    system("cls");
                }

                if (logedIn) {
                    mainMenu();
                }
                else {
                    cout << "Incorrect email or password! Attempts left " << (attempts - 1) << endl;
                    system("pause");
                    system("cls");
                    login(attempts - 1, fileName);
                }
            }
        }
    }
}

void deleteAccount() {
    loadAccountsFromFile();

    if (accountsCount == 0) {
        cout << "No admin accounts to delete." << endl;
        system("pause");
        system("cls");
        return;
    }

    cout << "--- List of Admin Accounts ---" << endl;
    for (int i = 0; i < accountsCount; i++) {
        cout << i + 1 << ". " << admin[i].username << endl;
    }
    cout << "-----------------------------" << endl;

    int selection;
    cout << "Enter the number of the account to delete (or 0 to cancel): ";
    cin >> selection;

    if (cin.fail() || selection < 0 || selection > accountsCount) {
        cin.clear();
        cin.ignore(10000, '\n'); 
        cout << "Invalid selection." << endl;
        system("pause");
        system("cls");
        return;
    }
    if (selection == 0) {
        cout << "Deletion cancelled." << endl;
        system("pause");
        system("cls");
        return;
    }

    int accountIndexToDelete = selection - 1;
    string usernameToDelete = admin[accountIndexToDelete].username;
    string passwordAttempt;
    cout << "To delete account '" << usernameToDelete << "', please enter its password: ";
    cin >> passwordAttempt;

    if (admin[accountIndexToDelete].password == passwordAttempt) {
        char confirm;
        cout << "Are you sure you want to delete account '" << usernameToDelete << "'? (y/n): ";
        cin >> confirm;
        if (confirm == 'y' || confirm == 'Y') {
            for (int i = accountIndexToDelete; i < accountsCount - 1; ++i) {
                admin[i] = admin[i + 1];
            }
            accountsCount--;
            ofs.open("admin.txt"); 
            if (!ofs.is_open()) {
                cout << "Error: Failed to open admin.txt for saving." << endl;
                return;
            }
            for (int i = 0; i < accountsCount; i++) {
                ofs << "[" << i + 1 << "]" << endl;
                ofs << "Biodata Admin " << i + 1 << ":" << endl;
                ofs << admin[i].username << "," << admin[i].password << endl;
                ofs << admin[i].email << endl;
                ofs << admin[i].name << endl;
                ofs << admin[i].dateOfBirth << endl;
                ofs << admin[i].gender << endl;
                ofs << admin[i].telephoneNumber << endl;
            }
            ofs.close(); 
            cout << "Account '" << usernameToDelete << "' deleted successfully." << endl;
            if (logedInUser == usernameToDelete) { 
                logedIn = false;
                logedInUser = "GUEST"; 
            }
        } else {
            cout << "Deletion cancelled by user." << endl;
        }
    } else {
        cout << "Incorrect password. Deletion failed." << endl;
    }
    system("pause");
    system("cls");
}


void loginMenu() {
    int selectMenuLogin;

    do {
        cout << "======= LOGIN PAGE ========" << endl
            << "1. Register" << endl
            << "2. Login" << endl
            << "3. Delete Account" << endl
            << "4. Exit" << endl
            << "Menu: ";
        cin >> selectMenuLogin;

        switch (selectMenuLogin) {
            case 1:
                registerAccount();
                system("pause");
                system("cls");
                break;
            case 2:
                login(5, "admin.txt");
                system("pause");
                system("cls");
                break;
            case 3:
                deleteAccount();
                system("pause");
                system("cls");
                break;
            case 4:
                return;
            default:
                cout << "Invalid Choice!" << endl;
                system("pause");
                system("cls");
        }
    }while (true);
}

int main() {
    loadLastStockTransactionNumber();
    loginMenu();
}