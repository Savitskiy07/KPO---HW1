#include "pos/adapters/files.hpp"

#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <system_error>

namespace pos {
namespace fs = std::filesystem;

namespace {

void atomicWrite(const fs::path& target, const std::string& text) {
    auto temp = target;
    temp += ".tmp";
    fs::remove(temp);
    try {
        std::ofstream out;
        out.exceptions(std::ios::failbit | std::ios::badbit);
        out.open(temp, std::ios::binary | std::ios::trunc);
        out << text;
        out.close();
        fs::rename(temp, target);
    } catch (...) {
        std::error_code ignored;
        fs::remove(temp, ignored);
        throw;
    }
}

std::string readLine(std::istream& in) {
    std::string line;
    if (!std::getline(in, line)) {
        throw std::runtime_error("Неожиданный конец файла журнала.");
    }
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    return line;
}

void checkFields(std::istringstream& fields) {
    if (!fields) {
        throw std::runtime_error("Некорректное поле записи.");
    }
    fields >> std::ws;
    if (!fields.eof()) {
        throw std::runtime_error("Лишние поля в записи.");
    }
}

std::string nameField(const std::string& line) {
    std::istringstream fields(line);
    std::string name;
    fields >> std::quoted(name);
    checkFields(fields);
    return name;
}

Journal readJournal(std::istream& in) {
    if (readLine(in) != "POS-JOURNAL 1") {
        throw std::runtime_error("Неизвестный формат журнала.");
    }
    const auto group = nameField(readLine(in));
    if (readLine(in) != "CRITERIA") {
        throw std::runtime_error("Отсутствует раздел критериев.");
    }
    RubricBuilder builder;
    for (auto line = readLine(in); line != "STUDENTS"; line = readLine(in)) {
        std::istringstream fields(line);
        std::string name;
        double maximum = 0, weight = 0;
        fields >> std::quoted(name) >> maximum >> weight;
        checkFields(fields);
        builder.add(std::move(name), maximum, weight);
    }
    Journal journal(group, builder.build());
    for (auto line = readLine(in); line != "GRADES"; line = readLine(in)) {
        journal.addStudent(nameField(line));
    }
    for (auto line = readLine(in); line != "END"; line = readLine(in)) {
        std::istringstream fields(line);
        Id student = 0, criterion = 0;
        double value = 0;
        fields >> student >> criterion >> value;
        checkFields(fields);
        journal.record(student, criterion, value);
    }
    in >> std::ws;
    if (!in.eof() || in.bad()) {
        throw std::runtime_error("Ошибка чтения или лишние данные после END.");
    }
    return journal;
}

// UTF-8 text column for a spreadsheet. Preserve quotes and neutralize formula prefixes.
std::string csvName(const std::string& name) {
    std::string field = "\"";
    if (name.find_first_of("=+-@") == 0) {
        field += '\'';
    }
    for (char ch : name) {
        if (ch == '"') {
            field += '"';
        }
        field += ch;
    }
    return field + '"';
}

std::string percentText(double percent) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << percent;
    auto text = out.str();
    text.at(text.find('.')) = ',';
    return text;
}

} // namespace

std::optional<Journal> FileRepository::load() const {
    if (!fs::exists(file_)) {
        return std::nullopt;
    }
    std::ifstream in(file_, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Не удалось открыть журнал для чтения.");
    }
    try {
        return readJournal(in);
    } catch (const std::exception& error) {
        throw std::runtime_error(std::string("Не удалось прочитать журнал: ") + error.what());
    }
}

void FileRepository::save(const Journal& journal) {
    std::ostringstream out;
    out << std::setprecision(std::numeric_limits<double>::max_digits10)
        << "POS-JOURNAL 1\n" << std::quoted(journal.group()) << "\nCRITERIA\n";
    for (const auto& criterion : journal.rubric().criteria()) {
        out << std::quoted(criterion.name) << ' ' << criterion.maximum << ' '
            << criterion.weight << '\n';
    }
    out << "STUDENTS\n";
    for (const auto& student : journal.students()) {
        out << std::quoted(student.name) << '\n';
    }
    out << "GRADES\n";
    for (const auto& student : journal.students()) {
        for (const auto& criterion : journal.rubric().criteria()) {
            if (const auto mark = journal.score(student.id, criterion.id)) {
                out << student.id << ' ' << criterion.id << ' ' << mark->value() << '\n';
            }
        }
    }
    out << "END\n";
    atomicWrite(file_, out.str());
}

std::string CsvExporter::write(const std::vector<StudentResult>& rows) {
    std::ostringstream out;
    out << "\xEF\xBB\xBF" "ID;Студент;Оценено;Критериев;Итог (%);Статус\r\n";
    for (const auto& row : rows) {
        out << row.studentId << ';' << csvName(row.name) << ';'
            << row.final.graded << ';' << row.final.total << ';';
        if (row.final.percent) {
            out << percentText(*row.final.percent) << ";Готов";
        } else {
            out << ";Не выставлен";
        }
        out << "\r\n";
    }
    atomicWrite(file_, out.str());
    return file_.u8string();
}

} // namespace pos
