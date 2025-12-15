#include "Task.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>

/*
============================================================
 TaskList
 -----------------------------------------------------------
 A small in-memory task manager that stores Task objects.

 Responsibilities:
 - Add tasks (auto-assign an incremental id)
 - Mark tasks as done
 - Remove tasks by id
 - Print the task list
============================================================
*/
class TaskList {
public:
    /*
     * Add a new task with the given title.
     * Returns the id that was assigned to the new task.
     */
    int add(const std::string& title) {
        tasks_.emplace_back(nextId_++, title);  // create Task(id, title)
        return tasks_.back().id();              // return its id
    }

    /*
     * Mark a task as done by id.
     * Returns true if the task was found, otherwise false.
     */
    bool done(int id) {
        for (auto& t : tasks_) {
            if (t.id() == id) {
                t.markDone();
                return true;
            }
        }
        return false;
    }

    /*
     * Remove a task by id.
     * Uses the erase-remove idiom:
     * - remove_if moves "to be removed" elements to the end
     * - erase actually shrinks the vector
     * Returns true if something was removed.
     */
    bool remove(int id) {
        auto it = std::remove_if(
            tasks_.begin(),
            tasks_.end(),
            [&](const Task& t) { return t.id() == id; }
        );

        if (it != tasks_.end()) {
            tasks_.erase(it, tasks_.end());
            return true;
        }
        return false;
    }

    /*
     * Print all tasks to stdout.
     * Format:
     *   [id] ( ) Title     -> not done
     *   [id] (x) Title     -> done
     */
    void list() const {
        if (tasks_.empty()) {
            std::cout << "(no tasks)\n";
            return;
        }

        for (const auto& t : tasks_) {
            std::cout << "[" << t.id() << "] "
                      << (t.done() ? "(x) " : "( ) ")
                      << t.title() << "\n";
        }
    }

private:
    std::vector<Task> tasks_;  // all tasks stored in memory
    int nextId_ = 1;           // simple incremental id generator
};

/*
============================================================
 Command enum
 -----------------------------------------------------------
 Represents the supported command words from user input.
 Unknown is used when input does not match any known command.
============================================================
*/
enum class Command { Enter, Exit, List, Done, Remove, Unknown };

/*
 * Print help text for the user.
 * This is called after unknown commands, and can also be used
 * if you later add a 'help' command.
 */
static void help() {
    std::cout <<
    "Commands:\n"
    "  enter <title>    - Add a new task\n"
    "  list             - Show all tasks\n"
    "  done <id>        - Mark a task as completed\n"
    "  rm <id>          - Remove a task\n"
    "  exit             - Quit program\n";
}

/*
 * Convert a command word (first token of the line) into our Command enum.
 * This makes the main loop cleaner than comparing strings everywhere.
 */
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

    // Simple startup banner / instructions
    std::cout
        << "==========\n"
        << "Task Manager.\n\n"
        << "- Type 'enter <title>' to add tasks\n"
        << "- Type 'list' to see tasks\n"
        << "- Type 'exit' to quit.\n"
        << "==========\n";

    std::string line;

    // Main REPL loop (Read–Eval–Print Loop)
    while (true) {
        std::cout << "> ";

        // Read a full line from stdin (supports titles with spaces)
        if (!std::getline(std::cin, line)) break;

        // Parse the line:
        // - first token becomes the command word
        // - the rest of the line can be arguments (like title or id)
        std::istringstream iss(line);
        std::string cmdWord;
        iss >> cmdWord;

        Command cmd = parseCommand(cmdWord);

        switch (cmd) {

            // --------------------------------------------------
            // enter <title>  -> add a task
            // --------------------------------------------------
            case Command::Enter: {
                // Get the remainder of the line as the title
                std::string title;
                std::getline(iss, title);

                // Remove one leading space so " enter hello" becomes "hello"
                if (!title.empty() && title[0] == ' ')
                    title.erase(0, 1);

                if (title.empty()) {
                    std::cout << "Please provide a title.\n";
                    break;
                }

                int id = tm.add(title);
                std::cout << "Added task #" << id << "\n";
                break;
            }

            // --------------------------------------------------
            // exit / quit -> terminate program
            // --------------------------------------------------
            case Command::Exit:
                return 0;

            // --------------------------------------------------
            // list -> print all tasks
            // --------------------------------------------------
            case Command::List:
                tm.list();
                break;

            // --------------------------------------------------
            // done <id> -> mark task as completed
            // --------------------------------------------------
            case Command::Done: {
                int id;

                // Try to read an integer id from the stream
                if (iss >> id) {
                    std::cout << (tm.done(id) ? "Marked done.\n" : "No such id.\n");
                } else {
                    std::cout << "Usage: done <id>\n";
                }
                break;
            }

            // --------------------------------------------------
            // rm <id> -> remove task
            // --------------------------------------------------
            case Command::Remove: {
                int id;

                if (iss >> id) {
                    std::cout << (tm.remove(id) ? "Removed.\n" : "No such id.\n");
                } else {
                    std::cout << "Usage: rm <id>\n";
                }
                break;
            }

            // --------------------------------------------------
            // unknown command -> show message + help text
            // --------------------------------------------------
            case Command::Unknown:
            default:
                std::cout << "Unknown command. Type 'enter', 'list', 'done', 'rm', or 'exit'.\n";
                help();
                break;
        }
    }

    // If input stream closes (Ctrl+D / EOF), we just exit normally
    return 0;
}
