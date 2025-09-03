#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <limits>

using namespace std;

class Database {
private:
    unordered_map<string, double> data;
    const string filename = "database.txt";

    void loadFromFile() {
        ifstream file(filename);
        if (file.is_open()) {
            string name;
            double value;
            while (file >> name >> value) {
                data[name] = value;
            }
            file.close();
        }
    }

    void saveToFile() {
        ofstream file(filename);
        if (file.is_open()) {
            for (const auto& [name, value] : data) {
                file << name << " " << value << endl;
            }
            file.close();
        }
    }

public:
    Database() {
        loadFromFile();
    }

    ~Database() {
        saveToFile();
    }

    void createEntry(const string& name, double value) {
        if (data.find(name) != data.end()) {
            cout << "Error: Entry with name '" << name << "' already exists.\n";
        } else {
            data[name] = value;
            cout << "Entry '" << name << "' created with value: " << value << "\n";
        }
    }

    void editEntry(const string& name, double value, bool add) {
        if (data.find(name) == data.end()) {
            cout << "Error: Entry with name '" << name << "' does not exist.\n";
        } else {
            if (add) {
                data[name] += value;
                cout << "Added " << value << " to '" << name << "'. New value: " << data[name] << "\n";
            } else {
                data[name] -= value;
                cout << "Subtracted " << value << " from '" << name << "'. New value: " << data[name] << "\n";
            }
        }
    }

    void deleteEntry(const string& name) {
        if (data.erase(name)) {
            cout << "Entry '" << name << "' deleted successfully.\n";
        } else {
            cout << "Error: Entry with name '" << name << "' does not exist.\n";
        }
    }

    void displayEntries() {
        if (data.empty()) {
            cout << "Database is empty.\n";
        } else {
            cout << "\n";
            cout << "\n";
            cout << "--- Database Entries ---\n";
            for (const auto& [name, value] : data) {
                cout << name << ": " << value << endl;
            }
            cout << "\n";
            cout << "\n";
        }
    }
};

class Calculator {
public:
    double add(double a, double b) {
        return a + b;
    }

    double subtract(double a, double b) {
        return a - b;
    }

    double multiply(double a, double b) {
        return a * b;
    }

    double divide(double a, double b) {
        if (b != 0) {
            return a / b;
        } else {
            cerr << "Error: Division by zero!\n";
            return 0;
        }
    }
};

int main() {
    Calculator calc;
    Database db;

    while (true) {
        cout << "\n--- C.A.A MAIN ---\n";
        cout << "--- Choose the operation ---\n";
        cout << "--- 1. Calculator\n";
        cout << "--- 2. Database\n";
        cout << "--- 5. Exit\n";
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (choice) {
            case 1: {
                while (true) {
                    cout << "\n--- C.A.A Calculator ---\n";
                    cout << "--- Choose the operation ---\n";
                    cout << "--- 1. Addition\n";
                    cout << "--- 2. Subtraction\n";
                    cout << "--- 3. Multiplication\n";
                    cout << "--- 4. Division\n";
                    cout << "--- 5. Return to Main Menu\n";
                    cout << "Enter your choice: ";

                    int operation;
                    cin >> operation;

                    if (cin.fail() || operation < 1 || operation > 5) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Invalid operation choice!\n";
                        continue;
                    }

                    if (operation == 5) {
                        cout << "Returning to Main Menu...\n";
                        this_thread::sleep_for(chrono::seconds(2));
                        break;
                    }

                    double a, b;
                    cout << "Enter number a: ";
                    cin >> a;
                    cout << "Enter number b: ";
                    cin >> b;

                    switch (operation) {
                        case 1:
                            cout << "The result of " << a << " + " << b << " is: " << calc.add(a, b) << endl;
                            break;
                        case 2:
                            cout << "The result of " << a << " - " << b << " is: " << calc.subtract(a, b) << endl;
                            break;
                        case 3:
                            cout << "The result of " << a << " x " << b << " is: " << calc.multiply(a, b) << endl;
                            break;
                        case 4:
                            if (b != 0)
                                cout << "The result of " << a << " / " << b << " is: " << calc.divide(a, b) << endl;
                            else
                                cout << "Error: Division by zero is not allowed.\n";
                            break;
                    }
                    this_thread::sleep_for(chrono::seconds(2));
                }
                break;
            }

            case 2: {
                while (true) {
                    cout << "\n--- C.A.A Database ---\n";
                    cout << "--- Choose the operation ---\n";
                    cout << "--- 1. Create Entry\n";
                    cout << "--- 2. Edit Entry\n";
                    cout << "--- 3. Delete Entry\n";
                    cout << "--- 4. Display Entries\n";
                    cout << "--- 5. Return to Main Menu\n";
                    cout << "Enter your choice: ";

                    int dbChoice;
                    cin >> dbChoice;

                    if (cin.fail() || dbChoice < 1 || dbChoice > 5) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Invalid database operation choice!\n";
                        continue;
                    }

                    if (dbChoice == 5) {
                        cout << "Returning to Main Menu...\n";
                        this_thread::sleep_for(chrono::seconds(2));
                        break;
                    }

                    string name;
                    double value;

                    switch (dbChoice) {
                        case 1:
                            cout << "Enter name for new entry: ";
                            cin >> name;
                            cout << "Enter value for '" << name << "': ";
                            cin >> value;
                            db.createEntry(name, value);
                            break;

                        case 2:
                            db.displayEntries();
                            cout << "Enter name of entry to edit: ";
                            cin >> name;
                            cout << "Enter value to add/subtract: ";
                            cin >> value;
                            cout << "Choose operation (1. Add, 2. Subtract): ";
                            int editChoice;
                            cin >> editChoice;
                            db.editEntry(name, value, editChoice == 1);
                            break;

                        case 3:
                            db.displayEntries();
                            cout << "Enter name of entry to delete: ";
                            cin >> name;
                            db.deleteEntry(name);
                            break;

                        case 4:
                            db.displayEntries();
                            break;
                    }
                    this_thread::sleep_for(chrono::seconds(2));
                }
                break;
            }

            case 5:
                cout << "Exiting the program. Goodbye!\n";
                this_thread::sleep_for(chrono::seconds(2));
                return 0;

            default:
                cout << "Invalid choice. Please try again.\n";
                break;
        }
    }

    return 0;
}
