// chat_sim_en.cpp
// Small console chat simulator (English code/UI) with Norwegian user names.
// Uses a background thread + queue to simulate network/IO with a tiny random delay.

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <random>
#include <sstream>

using Clock = std::chrono::system_clock;

struct Message {
    uint64_t id{};
    std::string from;
    std::string to;
    std::string text;
    Clock::time_point timestamp{Clock::now()};
};

static std::string timeToString(const Clock::time_point& tp) {
    std::time_t t = Clock::to_time_t(tp);
#if defined(_MSC_VER)
    std::tm tm_buf;
    localtime_s(&tm_buf, &t);
    std::tm* tm = &tm_buf;
#else
    std::tm* tm = std::localtime(&t);
#endif
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

class User {
public:
    explicit User(std::string name) : name_(std::move(name)) {}

    const std::string& name() const { return name_; }

    void receive(const Message& m) {
        std::lock_guard<std::mutex> lk(inbox_m_);
        inbox_.push_back(m);
        // Small heads-up to make it feel alive
        std::lock_guard<std::mutex> io(io_m_);
        std::cout << "🔔  (" << name_ << ") New message from " << m.from
                  << " #" << m.id << "\n";
    }

    void showInbox() const {
        std::lock_guard<std::mutex> lk(inbox_m_);
        if (inbox_.empty()) {
            std::cout << "(" << name_ << ") Inbox is empty for now.\n";
            return;
        }
        std::cout << "=== Inbox for " << name_ << " ===\n";
        for (const auto& m : inbox_) {
            std::cout << "#" << m.id << " [" << timeToString(m.timestamp) << "] "
                      << m.from << " -> " << m.to << ": " << m.text << "\n";
        }
        std::cout << "=============================\n";
    }

private:
    std::string name_;
    mutable std::mutex inbox_m_;
    std::vector<Message> inbox_;

    static std::mutex io_m_; // shared print lock for nicer output
};
std::mutex User::io_m_;

class ChatServer {
public:
    ChatServer()
        : running_(true),
          worker_([this]{ this->run(); }),
          rng_(std::random_device{}()),
          latency_ms_(100, 500) // tiny, human-ish delay
    {}

    ~ChatServer() {
        {
            std::lock_guard<std::mutex> lk(q_m_);
            running_ = false;
        }
        q_cv_.notify_all();
        if (worker_.joinable()) worker_.join();
    }

    void registerUser(const std::shared_ptr<User>& user) {
        std::lock_guard<std::mutex> lk(users_m_);
        users_[user->name()] = user;
    }

    bool hasUser(const std::string& name) const {
        std::lock_guard<std::mutex> lk(users_m_);
        return users_.count(name) > 0;
    }

    void send(const std::string& from, const std::string& to, const std::string& text) {
        Message msg;
        msg.id = next_id_++;
        msg.from = from;
        msg.to = to;
        msg.text = text;
        msg.timestamp = Clock::now();

        {
            std::lock_guard<std::mutex> lk(q_m_);
            queue_.push(std::move(msg));
        }
        q_cv_.notify_one();
    }

private:
    void run() {
        while (true) {
            Message msg;
            {
                std::unique_lock<std::mutex> lk(q_m_);
                q_cv_.wait(lk, [this]{ return !queue_.empty() || !running_; });
                if (!running_ && queue_.empty()) break;
                msg = std::move(queue_.front());
                queue_.pop();
            }

            // Simulate "network"
            std::this_thread::sleep_for(std::chrono::milliseconds(latency_ms_(rng_)));

            std::shared_ptr<User> recipient;
            {
                std::lock_guard<std::mutex> lk(users_m_);
                auto it = users_.find(msg.to);
                if (it != users_.end()) recipient = it->second;
            }

            std::lock_guard<std::mutex> io(io_m_);
            if (recipient) {
                recipient->receive(msg);
                std::cout << "(server) Delivered #" << msg.id
                          << " from " << msg.from << " to " << msg.to << ".\n";
            } else {
                std::cout << "(server) Unknown recipient \"" << msg.to
                          << "\" – message #" << msg.id << " not delivered.\n";
            }
        }
    }

    // Users
    mutable std::mutex users_m_;
    std::unordered_map<std::string, std::shared_ptr<User>> users_;

    // Queue / "network"
    std::queue<Message> queue_;
    std::mutex q_m_;
    std::condition_variable q_cv_;

    // IDs, thread, printing
    std::atomic<uint64_t> next_id_{1};
    std::atomic<bool> running_{false};
    std::thread worker_;
    std::mutex io_m_;

    // Random latency
    std::mt19937 rng_;
    std::uniform_int_distribution<int> latency_ms_;
};

static void printHelp() {
    std::cout <<
        "\nCommands:\n"
        "  users                       - list users\n"
        "  send <from> <to> <text>     - send a message\n"
        "  inbox <user>                - show inbox\n"
        "  demo                        - send a small demo scenario\n"
        "  help                        - show this help\n"
        "  quit                        - exit\n\n";
}

int main() {
    std::ios::sync_with_stdio(false);

    ChatServer server;

    // Norwegian names only (as requested)
    auto kari   = std::make_shared<User>("Kari");
    auto ola    = std::make_shared<User>("Ola");
    auto nora   = std::make_shared<User>("Nora");
    auto ahmed  = std::make_shared<User>("Ahmed");

    server.registerUser(kari);
    server.registerUser(ola);
    server.registerUser(nora);
    server.registerUser(ahmed);

    std::cout << "Welcome to the tiny chat! 💬\n";
    printHelp();

    std::string cmd;
    while (true) {
        std::cout << "> ";
        if (!(std::cin >> cmd)) break;

        if (cmd == "users") {
            std::cout << "Users: Kari, Ola, Nora, Ahmed\n";
        } else if (cmd == "send") {
            std::string from, to;
            if (!(std::cin >> from >> to)) {
                std::cout << "Missing parameters. Usage: send <from> <to> <text>\n";
                std::cin.clear();
                continue;
            }
            std::string text;
            std::getline(std::cin, text); // rest of the line is the text
            if (!text.empty() && text.front() == ' ') text.erase(0, 1);
            if (text.empty()) {
                std::cout << "Message text cannot be empty.\n";
                continue;
            }
            if (!server.hasUser(from)) {
                std::cout << "Unknown sender '" << from << "'. Type 'users' to see the list.\n";
                continue;
            }
            server.send(from, to, text);
            std::cout << "✈️  Sending from " << from << " to " << to << "...\n";
        } else if (cmd == "inbox") {
            std::string who;
            if (!(std::cin >> who)) {
                std::cout << "Usage: inbox <user>\n";
                std::cin.clear();
                continue;
            }
            if      (who == "Kari")   kari->showInbox();
            else if (who == "Ola")    ola->showInbox();
            else if (who == "Nora")   nora->showInbox();
            else if (who == "Ahmed")  ahmed->showInbox();
            else std::cout << "Unknown user. Type 'users' to see the list.\n";
        } else if (cmd == "demo") {
            server.send("Kari",  "Ola",   "Hi! Coffee after lunch?");
            server.send("Ola",   "Kari",  "Yes! 13:00 by the cafeteria?");
            server.send("Nora",  "Ahmed", "Got a minute to look at a bug later?");
            server.send("Ahmed", "Nora",  "Sure, I’ll ping you when free.");
            server.send("Kari",  "Nora",  "We’re planning coffee—join us? ☕️");
            std::cout << "Demo messages sent. Try 'inbox <user>' in a moment.\n";
        } else if (cmd == "help") {
            printHelp();
        } else if (cmd == "quit") {
            std::cout << "Goodbye! 👋\n";
            break;
        } else {
            std::cout << "Unknown command. Type 'help'.\n";
        }
    }

    return 0;
}
