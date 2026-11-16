#include "pos/adapters/console.hpp"

#include <algorithm>
#include <charconv>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace pos {
namespace {

struct EndOfInput {};
constexpr char menu[] =
    "\n1  Добавить студента        2  Список студентов\n"
    "3  Добавить критерий        4  Рубрикатор\n"
    "5  Выставить оценку         6  Показать журнал\n"
    "7  Исправить оценку         8  Отменить оценку\n"
    "9  Результат студента      10  Экспорт итогов в CSV для Excel\n"
    "0  Сохранить и выйти\n";

std::string compact(double value) {
    std::ostringstream out;
    out << std::setprecision(10) << value;
    return out.str();
}

std::string trim(const std::string& text) {
    const auto start = text.find_first_not_of(" \t");
    return start == std::string::npos ? "" :
        text.substr(start, text.find_last_not_of(" \t") - start + 1);
}

} // namespace

std::string Console::line(const char* prompt) {
    out_ << prompt << std::flush;
    std::string value;
    if (!std::getline(in_, value)) {
        throw EndOfInput{};
    }
    if (!value.empty() && value.back() == '\r') {
        value.pop_back();
    }
    return value;
}

Id Console::id(const char* prompt) {
    const auto text = trim(line(prompt));
    Id value = 0;
    const auto end = text.data() + text.size();
    const auto parsed = std::from_chars(text.data(), end, value);
    if (parsed.ec != std::errc{} || parsed.ptr != end) {
        throw std::invalid_argument("Введите целый неотрицательный номер.");
    }
    return value;
}

double Console::number(const char* prompt) {
    auto text = trim(line(prompt));
    std::replace(text.begin(), text.end(), ',', '.');
    std::istringstream fields(text);
    double value = 0;
    if (!(fields >> value) || fields.peek() != std::char_traits<char>::eof()) {
        throw std::invalid_argument("Введите число, например 8 или 7,5.");
    }
    return value;
}

void Console::students() {
    out_ << "Студенты группы «" << app_.journal().group() << "»\n";
    if (app_.journal().students().empty()) {
        out_ << "Список пока пуст.\n";
    }
    for (const auto& student : app_.journal().students()) {
        out_ << student.id << "  " << student.name << '\n';
    }
}

void Console::criteria() {
    out_ << "Рубрикатор\n";
    if (app_.journal().rubric().criteria().empty()) {
        out_ << "Критерии ещё не добавлены.\n";
    }
    for (const auto& criterion : app_.journal().rubric().criteria()) {
        out_ << criterion.id << "  " << criterion.name << "  (макс. "
             << compact(criterion.maximum) << ", вес " << compact(criterion.weight) << ")\n";
    }
}

void Console::table() {
    const Journal& journal = app_.journal();
    out_ << "Журнал «" << journal.group() << "» · — нет оценки\n";
    for (const auto& criterion : journal.rubric().criteria()) {
        out_ << "K" << criterion.id << " = " << criterion.name
             << " [" << compact(criterion.maximum) << "]\n";
    }

    out_ << "ID  ";
    for (const auto& criterion : journal.rubric().criteria()) {
        out_ << std::setw(12) << ("K" + std::to_string(criterion.id));
    }
    out_ << "  Студент\n";
    for (const auto& student : journal.students()) {
        out_ << std::setw(4) << std::left << student.id << std::right;
        for (const auto& grade : student.grades) {
            if (grade) {
                out_ << std::setw(12) << compact(grade->value());
            } else {
                out_ << std::setw(11) << " " << "—";
            }
        }
        out_ << "  " << student.name << '\n';
    }
}

void Console::editGrade(GradeAction action) {
    if (app_.journal().students().empty() || app_.journal().rubric().criteria().empty()) {
        throw std::invalid_argument("Сначала добавьте хотя бы одного студента и критерий.");
    }
    students();
    const Id student = id("ID студента: ");
    app_.journal().student(student);
    criteria();
    const Id criterion = id("ID критерия: ");
    const auto& selected = app_.journal().rubric().at(criterion);
    if (action == GradeAction::Cancel) {
        app_.cancel(student, criterion);
        out_ << "Оценка отменена.\n";
        return;
    }
    out_ << "Максимум: " << compact(selected.maximum) << '\n';
    const double value = number("Балл: ");
    if (action == GradeAction::Record) {
        app_.record(student, criterion, value);
    } else {
        app_.correct(student, criterion, value);
    }
    out_ << "Оценка сохранена.\n";
}

void Console::results() {
    if (app_.journal().students().empty()) {
        throw std::invalid_argument("Сначала добавьте студента.");
    }
    students();
    const Id student = id("ID студента: ");
    const auto final = app_.finalResult(student);
    out_ << "Оценено критериев: " << final.graded << '/' << final.total << '\n';
    if (final.percent) {
        out_ << "Итог: " << compact(*final.percent) << " %\n";
    } else {
        out_ << "Итог не выставлен.\n";
        if (const auto interim = app_.interimResult(student); interim.percent) {
            out_ << "Предварительно по оценённым критериям: "
                 << compact(*interim.percent) << " %\n";
        }
    }
}

void Console::execute(Id action) {
    switch (action) {
    case 1: {
        const auto added = app_.addStudent(line("Фамилия и имя: "));
        out_ << "Добавлен студент, ID " << added << ".\n";
        break;
    }
    case 2: students(); break;
    case 3: {
        auto name = line("Название критерия: ");
        const double maximum = number("Максимальный балл: ");
        const double weight = number("Вес: ");
        const auto added = app_.addCriterion(std::move(name), maximum, weight);
        out_ << "Добавлен критерий, ID " << added << ".\n";
        break;
    }
    case 4: criteria(); break;
    case 5: editGrade(GradeAction::Record); break;
    case 6: table(); break;
    case 7: editGrade(GradeAction::Correct); break;
    case 8: editGrade(GradeAction::Cancel); break;
    case 9: results(); break;
    case 10: {
        const auto file = app_.exportFinals();
        out_ << "Результаты выгружены: " << file << '\n';
        break;
    }
    default: out_ << "Нет такой команды.\n";
    }
}

void Console::run() {
    try {
        out_ << "ПОС · Преподавательская оценка студентов\n";
        while (!app_.hasJournal()) {
            try {
                app_.create(line("Новая группа. Название: "));
            } catch (const std::invalid_argument& error) {
                out_ << "Ошибка: " << error.what() << '\n';
            }
        }
        out_ << "Открыта группа «" << app_.journal().group() << "».\n";
        for (;;) {
            out_ << menu;
            try {
                const Id action = id("Выберите действие: ");
                if (action == 0) {
                    return;
                }
                execute(action);
            } catch (const std::exception& error) {
                out_ << "Ошибка: " << error.what() << '\n';
            }
        }
    } catch (const EndOfInput&) {
        out_ << "\nВвод завершён.\n";
    }
}

} // namespace pos
