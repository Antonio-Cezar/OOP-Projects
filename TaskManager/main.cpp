#include "Task.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>

class TaskList {
public:
    int add(const std::string& title) {
        tasks_.emplace_back(nextId_++, title);
        return tasks_.back().id();
    }

    bool done(int id) {
        for (auto& t : tasks_) if (t.id() == id) { t.markDone(); return true; }
        return false;
    }

    bool remove(int id) {
        auto it = std::remove_if(tasks_.begin(), tasks_.end(),
                                 [&](const Task& t){ return t.id() == id; });
        if (it != tasks_.end()) {
            tasks_.erase(it, tasks_.end());
            return true;
        }
        return false;
    }

    void list() const {
        if (tasks_.empty()) { std::cout << "(no tasks)\n"; return; }
        for (const auto& t : tasks_) {
            std::cout << "[" << t.id() << "] "
                      << (t.done() ? "(x) " : "( ) ")
                      << t.title() << "\n";
        }
    }

private:
    std::vector<Task> tasks_;
    int nextId_ = 1;
};

enum class Command { Enter, Exit, List, Done, Remove, Unknown };

static void help() {
    std::cout <<
    "Commands:\n"
    "  enter <title>    - Add a new task\n"
    "  list             - Show all tasks\n"
    "  done <id>        - Mark a task as completed\n"
    "  rm <id>          - Remove a task\n"
    "  exit             - Quit program\n";
}

static Command parseCommand(const std::string& cmd) {
    if (cmd == "enter") return Command::Enter;
    if (cmd == "exit" || cmd == "quit") return Command::Exit;
    if (cmd == "list") return Command::List;
    if (cmd == "done") return Command::Done;
    if (cmd == "rm") return Command::Remove;
    return Command::Unknown;
}

int main() {
    TaskList tm;
    std::cout << "==========\nTask Manager. \n\n- Type 'enter' to manage tasks \n- 'exit' to quit.\n==========\n";

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;

        std::istringstream iss(line);
        std::string cmdWord; iss >> cmdWord;
        Command cmd = parseCommand(cmdWord);

        switch (cmd) {
            case Command::Enter: {
                std::string title; std::getline(iss, title);
                if (!title.empty() && title[0]==' ') title.erase(0,1);
                if (title.empty()) { std::cout << "Please provide a title.\n"; break; }
                int id = tm.add(title);
                std::cout << "Added task #" << id << "\n";
                break;
            }
            case Command::Exit:
                return 0;
            case Command::List:
                tm.list();
                break;
            case Command::Done: {
                int id; if (iss >> id) {
                    std::cout << (tm.done(id) ? "Marked done.\n" : "No such id.\n");
                } else std::cout << "Usage: done <id>\n";
                break;
            }
            case Command::Remove: {
                int id; if (iss >> id) {
                    std::cout << (tm.remove(id) ? "Removed.\n" : "No such id.\n");
                } else std::cout << "Usage: rm <id>\n";
                break;
            }
            case Command::Unknown:
            default:
                std::cout << "Unknown command. Type 'enter', 'list', 'done', 'rm', or 'exit'.\n";
                help();
                break;
        }
    }
}
