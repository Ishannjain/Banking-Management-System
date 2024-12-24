#include <iostream>     // For input and output operations
#include <fstream>      // For file handling
#include <vector>       // For using the vector container
#include <string>       // For string handling
#include <iomanip>      // For formatted output (e.g., precision control)
#include <ctime>        // For handling date and time operations
#include <conio.h>      // For _getch() function to hide password input
#include <bits/stdc++.h> // Include all standard libraries (not recommended in production)

using namespace std;

// Function to securely get hidden password input
string getHiddenPassword() {
    string password;
    char ch;
    while ((ch = _getch()) != 13) { // Loop until Enter key is pressed
        if (ch == 8 && !password.empty()) { // Handle backspace for password input
            password.pop_back();
            cout << "\b \b";
        } else if (ch != 8) { // Display '*' for any other character
            password.push_back(ch);
            cout << "*";
        }
    }
    cout << endl;
    return password;
}

// Base class for Person, provides general attributes for customers and managers
class Person {
protected:
    string name, contact;
    int age;

public:
    // Constructor to initialize base attributes
    Person(string n, int a, string c) : name(n), age(a), contact(c) {}

    // Virtual function to display person's details (overridden by derived classes)
    virtual void displayInfo() const {
        cout << "Name: " << name << "\nAge: " << age << "\nContact: " << contact << endl;
    }

    // Enforces derived classes to define their type
    virtual string getType() const = 0;
};

// Derived class representing a Customer
class Customer : public Person {
private:
    static int idCounter; // Static counter for generating unique customer IDs
    string customerID, password; // Unique ID and password for the customer
    double balance, dailyTransactionTotal; // Balance and daily transaction total
    double dailyTransactionLimit; // Limit for daily withdrawals
    time_t lastTransactionDate; // Timestamp of the last transaction

public:
    // Constructor to initialize customer details
    Customer(string n, int a, string c, string pwd, double initBal)
        : Person(n, a, c), password(pwd), balance(initBal), dailyTransactionLimit(5000), dailyTransactionTotal(0) {
        customerID = to_string(++idCounter); // Generate unique ID
        lastTransactionDate = time(nullptr); // Set initial transaction date
    }

    // Deposit function to add money to the balance
    void deposit(double amount) {
        if (amount <= 0) throw invalid_argument("Deposit must be > 0.");
        balance += amount;
    }

    // Withdrawal function with daily transaction limit check
    void withdraw(double amount) {
        if (amount <= 0 || amount > balance) throw runtime_error("Invalid withdrawal amount.");
        resetDailyLimit(); // Check and reset daily limit if needed
        if (dailyTransactionTotal + amount > dailyTransactionLimit)
            throw runtime_error("Daily transaction limit exceeded.");
        balance -= amount;
        dailyTransactionTotal += amount;
    }

    // Reset daily transaction limit if the day has changed
    void resetDailyLimit() {
        time_t now = time(nullptr);
        if (localtime(&now)->tm_yday != localtime(&lastTransactionDate)->tm_yday) {
            dailyTransactionTotal = 0;
            lastTransactionDate = now;
        }
    }

    // Verify if entered password matches
    bool verifyPassword(const string& pwd) const { return password == pwd; }

    // Allow customer to change their password
    void changePassword() {
        cout << "New password: ";
        password = getHiddenPassword();
    }

    // Display customer details
    void displayInfo() const override {
        Person::displayInfo();
        cout << "ID: " << customerID << "\nBalance: Rs" << fixed << setprecision(2) << balance << endl;
    }

    // Getter for customer ID
    string getCustomerID() const { return customerID; }

    // Returns the type of this class
    string getType() const override { return "Customer"; }

    // Overload << operator to save customer details to file
    friend ofstream& operator<<(ofstream& ofs, const Customer& c) {
        ofs << c.customerID << "," << c.name << "," << c.age << "," << c.contact << "," << c.balance << "," << c.password << endl;
        return ofs;
    }

    // Overload >> operator to load customer details from file
    friend istream& operator>>(istream& ifs, Customer& c) {
        ifs >> c.customerID >> c.name >> c.age >> c.contact >> c.balance >> c.password;
        return ifs;
    }
};

int Customer::idCounter = 0; // Initialize static variable for customer ID generation

// Derived class representing a Manager
class Manager : public Person {
private:
    string password;

public:
    // Constructor to initialize manager details
    Manager(string n, int a, string c, string pwd) : Person(n, a, c), password(pwd) {}

    // Verify manager password
    bool verifyPassword(const string& pwd) const { return password == pwd; }

    // Allow manager to change their password
    void changePassword() {
        cout << "New password: ";
        password = getHiddenPassword();
    }

    // Return the type of this class
    string getType() const override { return "Manager"; }
};

// Bank class managing customers and operations
class Bank {
private:
    vector<Customer> customers; // List of customers
    Manager* manager;           // Manager of the bank

    // Save customer data to file
    void saveToFile() {
        ofstream ofs("customers.txt", ios::app);
        for (const auto& c : customers) ofs << c;
    }

    // Load customer data from file
    void loadFromFile() {
        ifstream ifs("customers.txt");
        Customer temp("", 0, "", "", 0.0);
        while (ifs >> temp) customers.push_back(temp);
    }

public:
    // Constructor to initialize the bank with a manager and load data
    Bank(Manager* m) : manager(m) { loadFromFile(); }
    ~Bank() { delete manager; }

    // Save all data to file
    void saveData() {
        saveToFile();
        cout << "Data saved successfully!" << endl;
    }

    // Add a new customer to the bank
    void addCustomer() {
        string name, contact, password;
        int age;
        double deposit;

        cout << "Name: "; cin.ignore(); getline(cin, name);
        cout << "Age: "; cin >> age;

        if (age < 18) throw runtime_error("Customer must be at least 18 years old.");

        cout << "Contact: "; cin.ignore(); getline(cin, contact);
        cout << "Password: "; password = getHiddenPassword();
        cout << "Initial Deposit (>= Rs500): Rs"; cin >> deposit;

        if (deposit < 500) throw runtime_error("Minimum deposit is Rs500.");

        customers.emplace_back(name, age, contact, password, deposit);
        cout << "Customer added. ID: " << customers.back().getCustomerID() << endl;
    }

    // Process deposit or withdrawal transaction
    void processTransaction(bool isDeposit) {
        string customerID, password;
        double amount;

        cout << "Customer ID: "; cin >> customerID;
        cout << "Password: "; password = getHiddenPassword();

        for (auto& c : customers) {
            if (c.getCustomerID() == customerID && c.verifyPassword(password)) {
                cout << (isDeposit ? "Deposit Amount: Rs" : "Withdraw Amount: Rs");
                cin >> amount;

                try {
                    isDeposit ? c.deposit(amount) : c.withdraw(amount);
                    cout << (isDeposit ? "Deposit" : "Withdrawal") << " successful!" << endl;
                } catch (const exception& e) {
                    cerr << e.what() << endl;
                }
                return;
            }
        }
        cout << "Invalid ID or password!" << endl;
    }

    // Manager login and display all customer details
    void managerLogin() {
        cout << "Manager Password: ";
        if (manager->verifyPassword(getHiddenPassword())) {
            cout << "\n[Customer List]\n";
            for (const auto& c : customers) c.displayInfo();
        } else {
            cout << "Access Denied!" << endl;
        }
    }

    // Remove a customer from the bank
    void removeCustomer() {
        string customerID;
        cout << "Customer ID to remove: ";
        cin >> customerID;

        auto it = remove_if(customers.begin(), customers.end(),
            [&](const Customer& c) { return c.getCustomerID() == customerID; });

        if (it != customers.end()) {
            customers.erase(it);
            cout << "Customer removed." << endl;
        } else {
            cout << "Customer not found!" << endl;
        }
    }
};

// Main function: Program entry point
int main() {
    Manager* manager = new Manager("Admin", 45, "admin@bank.com", "admin123");
    Bank bank(manager);

    cout << "Welcome to ABC Bank:";

    int choice;
    do {
        cout << "\n1. Add Customer\n2. Deposit\n3. Withdraw\n4. Manager Login\n5. Remove Customer\n6. Exit\nChoice: ";
        cin >> choice;

        try {
            switch (choice) {
                case 1: bank.addCustomer(); break;
                case 2: bank.processTransaction(true); break;
                case 3: bank.processTransaction(false); break;
                case 4: bank.managerLogin(); break;
                case 5: bank.removeCustomer(); break;
                case 6: bank.saveData(); cout << "Goodbye!" << endl; break;
                default: cout << "Invalid choice." << endl;
            }
        } catch (const exception& e) {
            cerr << e.what() << endl;
        }
    } while (choice != 6);

    return 0;
}
