#include "pos/domain/journal.hpp"

#include <cmath>
#include <stdexcept>

namespace pos {
namespace {

std::string checkedName(std::string name) {
    const auto first = name.find_first_not_of(' ');
    if (first == std::string::npos) {
        throw std::invalid_argument("Введите непустое название или имя.");
    }
    for (unsigned char ch : name) {
        if (ch < 32 || ch == 127) {
            throw std::invalid_argument("Имя не может содержать управляющие символы.");
        }
    }
    return name.substr(first, name.find_last_not_of(' ') - first + 1);
}

} // namespace

const Criterion& Rubric::at(Id id) const {
    if (id == 0 || id > criteria_.size()) {
        throw std::invalid_argument("Такого критерия нет в рубрикаторе.");
    }
    return criteria_[id - 1];
}

void RubricBuilder::add(std::string name, double maximum, double weight) {
    if (!std::isfinite(maximum) || maximum <= 0) {
        throw std::invalid_argument("Максимум должен быть конечным числом больше нуля.");
    }
    if (!std::isfinite(weight) || weight <= 0) {
        throw std::invalid_argument("Вес должен быть конечным числом больше нуля.");
    }
    draft_.criteria_.push_back({draft_.criteria_.size() + 1, checkedName(std::move(name)),
                                maximum, weight});
}

Score::Score(double value, const Criterion& criterion) : value_(value) {
    if (!std::isfinite(value) || value < 0 || value > criterion.maximum) {
        throw std::invalid_argument("Балл должен лежать между нулём и максимумом критерия.");
    }
}

Journal::Journal(std::string group, Rubric rubric)
    : group_(checkedName(std::move(group))), rubric_(std::move(rubric)) {}

std::size_t Journal::studentIndex(Id id) const {
    if (id == 0 || id > students_.size()) {
        throw std::invalid_argument("Студент с таким ID не найден.");
    }
    return id - 1;
}

const Student& Journal::student(Id id) const {
    return students_[studentIndex(id)];
}

Id Journal::addStudent(std::string name) {
    const Id id = students_.size() + 1;
    Student row{id, checkedName(std::move(name)), {}};
    row.grades.resize(rubric_.criteria().size());
    students_.push_back(std::move(row));
    return id;
}

Id Journal::addCriterion(std::string name, double maximum, double weight) {
    RubricBuilder builder(rubric_);
    builder.add(std::move(name), maximum, weight);
    const Id id = rubric_.criteria().size() + 1;
    for (auto& row : students_) {
        row.grades.resize(id);
    }
    rubric_ = builder.build();
    return id;
}

std::optional<Score>& Journal::cell(Id studentId, const Criterion& criterion) {
    return students_[studentIndex(studentId)].grades[criterion.id - 1];
}

void Journal::record(Id studentId, Id criterionId, double value) {
    const Criterion& criterion = rubric_.at(criterionId);
    auto& grade = cell(studentId, criterion);
    if (grade) {
        throw std::invalid_argument("Оценка уже выставлена. Выберите исправление.");
    }
    grade = Score(value, criterion);
}

void Journal::correct(Id studentId, Id criterionId, double value) {
    const Criterion& criterion = rubric_.at(criterionId);
    auto& grade = cell(studentId, criterion);
    if (!grade) {
        throw std::invalid_argument("В этой ячейке нет оценки для исправления.");
    }
    grade = Score(value, criterion);
}

void Journal::cancel(Id studentId, Id criterionId) {
    auto& grade = cell(studentId, rubric_.at(criterionId));
    if (!grade) {
        throw std::invalid_argument("В этой ячейке нет оценки для отмены.");
    }
    grade.reset();
}

std::optional<Score> Journal::score(Id studentId, Id criterionId) const {
    const auto& criterion = rubric_.at(criterionId);
    return student(studentId).grades[criterion.id - 1];
}

} // namespace pos
