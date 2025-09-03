#pragma once
#include <string>

class Task {
public:
    Task(int id, const std::string& title) : id_(id), title_(title) {}

    int id() const { return id_; }
    const std::string& title() const { return title_; }
    bool done() const { return done_; }

    void markDone() { done_ = true; }

private:
    int id_;
    std::string title_;
    bool done_ = false;
};
