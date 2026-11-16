#pragma once

#include "pos/domain/grading.hpp"

namespace pos {

class JournalRepository {
public:
    virtual ~JournalRepository() = default;
    virtual std::optional<Journal> load() const = 0;
    virtual void save(const Journal& journal) = 0;
};

struct StudentResult {
    Id studentId;
    std::string name;
    Result final;
};

class ResultExporter {
public:
    virtual ~ResultExporter() = default;
    virtual std::string write(const std::vector<StudentResult>& rows) = 0;
};

class JournalApp {
public:
    JournalApp(JournalRepository& repository, ResultExporter& exporter);
    bool hasJournal() const { return current_.has_value(); }
    void create(std::string group);
    const Journal& journal() const;
    Id addStudent(std::string name);
    Id addCriterion(std::string name, double maximum, double weight);
    void record(Id student, Id criterion, double value);
    void correct(Id student, Id criterion, double value);
    void cancel(Id student, Id criterion);
    Result finalResult(Id student) const;
    Result interimResult(Id student) const;
    std::string exportFinals();

private:
    void save(Journal updated);
    JournalRepository& repository_;
    ResultExporter& exporter_;
    GradingService grading_;
    std::optional<Journal> current_;
};

} // namespace pos
