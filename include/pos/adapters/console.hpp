#pragma once

#include "pos/application/journal_app.hpp"

#include <iosfwd>

namespace pos {

class Console {
public:
    Console(JournalApp& app, std::istream& input, std::ostream& output)
        : app_(app), in_(input), out_(output) {}
    void run();

private:
    std::string line(const char* prompt);
    Id id(const char* prompt);
    double number(const char* prompt);
    void students();
    void criteria();
    void table();
    enum class GradeAction { Record, Correct, Cancel };
    void editGrade(GradeAction action);
    void execute(Id action);
    void results();

    JournalApp& app_;
    std::istream& in_;
    std::ostream& out_;
};

} // namespace pos
