#include "pos/application/journal_app.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace pos;

namespace {

void expect(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}

template<class Exception, class Action>
void rejects(Action action) {
    try {
        action();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error("Expected an exception");
}

void near(const Result& result, double expected) {
    expect(result.percent && std::abs(*result.percent - expected) < 1e-9, "Unexpected result");
}

Journal sample() {
    Journal journal("БПИ");
    journal.addStudent("Смирнова");
    journal.addCriterion("Проект", 10, 3);
    journal.addCriterion("Экзамен", 20, 2);
    return journal;
}

void rubricAndIdentity() {
    auto journal = sample();
    expect(journal.addStudent("Смирнова") == 2, "Namesakes have different IDs");
    expect(journal.student(1).name == journal.student(2).name, "Same display name");
    rejects<std::invalid_argument>([&] { journal.addCriterion("X", 0, 1); });
    rejects<std::invalid_argument>([&] { journal.addCriterion("X", 5, -1); });
    rejects<std::invalid_argument>([&] { journal.addCriterion(" ", 5, 1); });
    rejects<std::invalid_argument>([&] {
        journal.addCriterion("X", 5, std::numeric_limits<double>::infinity());
    });
    expect(journal.rubric().criteria().size() == 2, "Invalid rubric was published");
}

void newRowAndColumn() {
    Journal journal("Группа");
    journal.addCriterion("Проект", 10, 1);
    journal.addStudent("А");
    journal.record(1, 1, 0);

    journal.addStudent("Б");
    journal.addCriterion("Зачёт", 5, 1);
    expect(journal.score(1, 1)->value() == 0, "Existing zero lost");
    expect(!journal.score(2, 1) && !journal.score(1, 2), "New cells must be empty");
    journal.record(2, 2, 5);
    expect(!journal.score(2, 1) && !journal.score(1, 2), "Changed the wrong cell");
}

void boundsAndReferences() {
    auto journal = sample();
    rejects<std::invalid_argument>([&] { journal.record(1, 1, -1); });
    rejects<std::invalid_argument>([&] { journal.record(1, 1, 12); });
    rejects<std::invalid_argument>([&] { journal.record(1, 1, std::nan("")); });
    rejects<std::invalid_argument>([&] { journal.record(1, 3, 5); });
    rejects<std::invalid_argument>([&] { journal.record(0, 1, 5); });
    expect(!journal.score(1, 1), "Invalid assignment changed a cell");
    journal.record(1, 1, 10);
    journal.record(1, 2, 0);
    expect(journal.score(1, 1)->value() == 10, "Maximum excluded");
    expect(journal.score(1, 2)->value() == 0, "Missing zero");
}

void editAndCancel() {
    auto journal = sample();
    rejects<std::invalid_argument>([&] { journal.correct(1, 1, 1); });
    journal.record(1, 1, 4.5);
    rejects<std::invalid_argument>([&] { journal.record(1, 1, 1); });
    rejects<std::invalid_argument>([&] { journal.correct(1, 1, 12); });
    expect(journal.score(1, 1)->value() == 4.5, "Failed edit changed score");
    journal.correct(1, 1, 0);
    expect(journal.score(1, 1).has_value(), "Zero treated as missing");
    journal.cancel(1, 1);
    expect(!journal.score(1, 1), "Cancellation did not clear cell");
    rejects<std::invalid_argument>([&] { journal.cancel(1, 1); });
}

void weightedAndCurrent() {
    auto journal = sample();
    GradingService service;
    AllCriteria final;
    GradedCriteria interim;
    expect(!service.calculate(journal, 1, interim).percent, "Empty progress");
    journal.record(1, 1, 8);
    expect(!service.calculate(journal, 1, final).percent, "Premature final");
    near(service.calculate(journal, 1, interim), 80);
    journal.record(1, 2, 10);
    near(service.calculate(journal, 1, final), 68);
    journal.correct(1, 2, 20);
    near(service.calculate(journal, 1, final), 88);
    journal.cancel(1, 1);
    expect(!service.calculate(journal, 1, final).percent, "Stale final");
    near(service.calculate(journal, 1, interim), 100);
}

void emptyAndLargeWeights() {
    Journal journal("Группа");
    journal.addStudent("А");
    GradingService service;
    AllCriteria final;
    expect(!service.calculate(journal, 1, final).percent, "Empty rubric");
    const double large = std::numeric_limits<double>::max();
    journal.addCriterion("A", 1, large);
    journal.addCriterion("B", 1, large);
    journal.record(1, 1, 1);
    journal.record(1, 2, 0);
    near(service.calculate(journal, 1, final), 50);
}

class MemoryRepository final : public JournalRepository {
public:
    std::optional<Journal> load() const override { return saved; }
    void save(const Journal& journal) override {
        if (failSave) throw std::runtime_error("Disk full");
        saved = journal;
    }
    std::optional<Journal> saved;
    bool failSave = false;
};

class MemoryExporter final : public ResultExporter {
public:
    std::string write(const std::vector<StudentResult>& value) override {
        rows = value;
        return "memory";
    }
    std::vector<StudentResult> rows;
};

void saveAndRollback() {
    MemoryRepository repository;
    MemoryExporter exporter;
    JournalApp app(repository, exporter);
    app.create("Группа");
    app.addStudent("А");
    app.addCriterion("X", 10, 1);
    app.record(1, 1, 5);
    repository.failSave = true;
    rejects<std::runtime_error>([&] { app.correct(1, 1, 8); });
    near(app.finalResult(1), 50);
    expect(repository.saved->score(1, 1)->value() == 5, "Stored data changed");
    repository.failSave = false;
    app.correct(1, 1, 8);
    app.exportFinals();
    expect(exporter.rows.size() == 1, "Missing report row");
    near(exporter.rows[0].final, 80);
}

} // namespace

int main() {
    const std::pair<const char*, void(*)()> cases[] = {
        {"rubric and identity", rubricAndIdentity},
        {"adding a row and a column", newRowAndColumn},
        {"score bounds and references", boundsAndReferences},
        {"edit and cancellation", editAndCancel},
        {"weighted and current results", weightedAndCurrent},
        {"empty rubric and large weights", emptyAndLargeWeights},
        {"save failure and export", saveAndRollback}
    };
    for (const auto& test : cases) {
        try {
            test.second();
            std::cout << "OK: " << test.first << '\n';
        } catch (const std::exception& error) {
            std::cerr << "FAIL: " << test.first << ": " << error.what() << '\n';
            return 1;
        }
    }
}
