#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace pos {

using Id = std::size_t;

struct Criterion {
    Id id;
    std::string name;
    double maximum;
    double weight;
};

class Rubric {
public:
    Rubric() = default;
    const std::vector<Criterion>& criteria() const { return criteria_; }
    const Criterion& at(Id id) const;

private:
    friend class RubricBuilder;
    std::vector<Criterion> criteria_;
};

class RubricBuilder {
public:
    explicit RubricBuilder(Rubric base = {}) : draft_(std::move(base)) {}
    void add(std::string name, double maximum, double weight);
    Rubric build() { return std::move(draft_); }

private:
    Rubric draft_;
};

class Score {
public:
    double value() const { return value_; }
    bool operator==(Score other) const { return value_ == other.value_; }

private:
    friend class Journal;
    Score(double value, const Criterion& criterion);
    double value_;
};

struct Student {
    Id id;
    std::string name;
    std::vector<std::optional<Score>> grades;
};

class Journal {
public:
    explicit Journal(std::string group, Rubric rubric = {});
    const std::string& group() const { return group_; }
    const Rubric& rubric() const { return rubric_; }
    const std::vector<Student>& students() const { return students_; }
    const Student& student(Id id) const;

    Id addStudent(std::string name);
    Id addCriterion(std::string name, double maximum, double weight);
    void record(Id student, Id criterion, double value);
    void correct(Id student, Id criterion, double value);
    void cancel(Id student, Id criterion);
    std::optional<Score> score(Id student, Id criterion) const;

private:
    std::size_t studentIndex(Id id) const;
    std::optional<Score>& cell(Id student, const Criterion& criterion);

    std::string group_;
    Rubric rubric_;
    std::vector<Student> students_;
};

} // namespace pos
