#include "pos/application/journal_app.hpp"

#include <stdexcept>

namespace pos {

JournalApp::JournalApp(JournalRepository& repository, ResultExporter& exporter)
    : repository_(repository), exporter_(exporter), current_(repository.load()) {}

void JournalApp::create(std::string group) {
    if (hasJournal()) {
        throw std::logic_error("Журнал группы уже существует.");
    }
    save(Journal(std::move(group)));
}

const Journal& JournalApp::journal() const {
    if (!current_) {
        throw std::logic_error("Сначала создайте группу.");
    }
    return *current_;
}

void JournalApp::save(Journal updated) {
    repository_.save(updated);
    current_ = std::move(updated);
}

Id JournalApp::addStudent(std::string name) {
    Journal updated = journal();
    const Id id = updated.addStudent(std::move(name));
    save(std::move(updated));
    return id;
}

Id JournalApp::addCriterion(std::string name, double maximum, double weight) {
    Journal updated = journal();
    const Id id = updated.addCriterion(std::move(name), maximum, weight);
    save(std::move(updated));
    return id;
}

void JournalApp::record(Id student, Id criterion, double value) {
    Journal updated = journal();
    updated.record(student, criterion, value);
    save(std::move(updated));
}

void JournalApp::correct(Id student, Id criterion, double value) {
    Journal updated = journal();
    updated.correct(student, criterion, value);
    save(std::move(updated));
}

void JournalApp::cancel(Id student, Id criterion) {
    Journal updated = journal();
    updated.cancel(student, criterion);
    save(std::move(updated));
}

Result JournalApp::finalResult(Id student) const {
    return grading_.calculate(journal(), student, AllCriteria{});
}

Result JournalApp::interimResult(Id student) const {
    return grading_.calculate(journal(), student, GradedCriteria{});
}

std::string JournalApp::exportFinals() {
    std::vector<StudentResult> rows;
    for (const auto& student : journal().students()) {
        rows.push_back({student.id, student.name, finalResult(student.id)});
    }
    return exporter_.write(rows);
}

} // namespace pos
