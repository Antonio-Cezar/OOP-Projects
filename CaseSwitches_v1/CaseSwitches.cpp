#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

class Record {
public:
    int id;
    string name;
    string data;

    Record(int recordId, const string& recordName, const string& recordData)
        : id(recordId), name(recordName), data(recordData) {}

    void display() const {
        cout << "ID: " << id << "\nName: " << name << "\nData: " << data << "\n";
    }

    string toFileString() const {
        return to_string(id) + "|" + name + "|" + data + "\n";
    }

    static Record fromFileString(const string& fileString) {
        stringstream ss(fileString);
        string idStr, name, data;
        getline(ss, idStr, '|');
        getline(ss, name, '|');
        getline(ss, data, '|');
        return Record(stoi(idStr), name, data);
    }
};

// Function to load records from a file
vector<Record> loadRecords(const string& filename) {
    vector<Record> records;
    ifstream file(filename);
    if (!file) {
        cout << "File does not exist. A new one will be created when you save data.\n";
        return records;
    }

    string line;
    while (getline(file, line)) {
        if (!line.empty()) {
            records.push_back(Record::fromFileString(line));
        }
    }
    return records;
}

// Function to save records to a file
void saveRecords(const string& filename, const vector<Record>& records) {
    ofstream file(filename, ios::trunc);
    if (!file) {
        cout << "Error: Could not save data.\n";
        return;
    }

    for (const auto& record : records) {
        file << record.toFileString();
    }
    cout << "Data saved successfully to " << filename << ".\n";
}

// Function to display records
void viewRecords(const vector<Record>& records) {
    if (records.empty()) {
        cout << "No records to display.\n";
        return;
    }

    cout << "\n--- Records ---\n";
    for (const auto& record : records) {
        record.display();
        cout << "----------------\n";
    }
}

// Function to add a new record
void addRecord(vector<Record>& records) {
    int id;
    string name, data;

    cout << "Enter Record ID: ";
    cin >> id;
    cin.ignore();

    cout << "Enter Record Name: ";
    getline(cin, name);

    cout << "Enter Record Data: ";
    getline(cin, data);

    records.emplace_back(id, name, data);
    cout << "Record added successfully.\n";
}

// Function to delete a record by ID
void deleteRecord(vector<Record>& records) {
    int id;
    cout << "Enter the ID of the record to delete: ";
    cin >> id;

    auto it = remove_if(records.begin(), records.end(), [id](const Record& record) {
        return record.id == id;
    });

    if (it != records.end()) {
        records.erase(it, records.end());
        cout << "Record deleted successfully.\n";
    } else {
        cout << "Record with ID " << id << " not found.\n";
    }
}

int main() {
    const string filename = "records.txt";
    vector<Record> records = loadRecords(filename);

    while (true) {
        cout << "\n--- File Manager ---\n";
        cout << "1. View Records\n";
        cout << "2. Add Record\n";
        cout << "3. Delete Record\n";
        cout << "4. Save Records\n";
        cout << "9. Exit\n";
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;
        cin.ignore();

        switch (choice) {
        case 1:
            viewRecords(records);
            break;

        case 2:
            addRecord(records);
            break;

        case 3:
            deleteRecord(records);
            break;

        case 4:
            saveRecords(filename, records);
            break;

        case 9:
            saveRecords(filename, records);
            return 0;

        default:
            cout << "Invalid choice. Please try again.\n";
        }
    }
}
