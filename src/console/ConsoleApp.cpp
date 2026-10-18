#include "ConsoleApp.h"
#include "domain/Bicycle.h"
#include "domain/ChargingStation.h"
#include "domain/ElectricScooter.h"
#include "domain/Helmet.h"

#include <cmath>
#include <istream>
#include <locale>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {

template <typename T>
bool parseValue(const std::string& line, T& value) {
    std::istringstream parser(line);
    parser.imbue(std::locale::classic());
    if (!(parser >> value)) {
        return false;
    }
    parser >> std::ws;
    return parser.eof();
}

} // namespace

ConsoleApp::ConsoleApp(FleetService& fleet, std::istream& input,
                       std::ostream& output)
    : fleet_(fleet), input_(input), output_(output) {}

void ConsoleApp::run() {
    output_ << "ВышКат — учёт имущества\n";
    while (true) {
        output_ << "\n1 — Добавить транспорт\n"
                << "2 — Добавить вещь\n"
                << "3 — Показать отчёт\n"
                << "4 — Транспорт для новичков\n"
                << "5 — Общий инвентарь\n"
                << "0 — Выйти\n";

        int command = 0;
        if (!readInteger("Команда: ", 0, 5, command)) {
            return;
        }

        switch (command) {
        case 0:
            output_ << "До свидания!\n";
            return;
        case 1:
            if (!addVehicle()) {
                return;
            }
            break;
        case 2:
            if (!addItem()) {
                return;
            }
            break;
        case 3:
            showSummary();
            break;
        case 4:
            showEntries(fleet_.getBeginnerVehicles(), "Транспорт для новичков:",
                        "Транспорта для новичков пока нет.");
            break;
        case 5:
            showEntries(fleet_.getInventory(), "Общий инвентарь:",
                        "Имущество пока не добавлено.");
            break;
        }
    }
}

bool ConsoleApp::readLine(const std::string& prompt, std::string& value) {
    output_ << prompt << std::flush;
    if (std::getline(input_, value)) {
        return true;
    }
    output_ << (input_.eof() ? "\nВвод завершён.\n"
                            : "\nОшибка чтения. Работа завершена.\n");
    return false;
}

bool ConsoleApp::readText(const std::string& prompt, std::string& value) {
    while (readLine(prompt, value)) {
        if (value.find_first_not_of(" \t\r\n\f\v") != std::string::npos) {
            return true;
        }
        output_ << "Ошибка: значение не должно быть пустым или состоять из пробелов.\n";
    }
    return false;
}

bool ConsoleApp::readInteger(const std::string& prompt, int minimum, int maximum,
                             int& value) {
    std::string line;
    while (readLine(prompt, line)) {
        if (parseValue(line, value) && value >= minimum && value <= maximum) {
            return true;
        }
        output_ << "Ошибка: введите целое число от " << minimum
                << " до " << maximum << ".\n";
    }
    return false;
}

bool ConsoleApp::readBoolean(const std::string& prompt, bool& value) {
    std::string line;
    std::string token;
    while (readLine(prompt, line)) {
        if (parseValue(line, token) && (token == "0" || token == "1")) {
            value = token == "1";
            return true;
        }
        output_ << "Ошибка: введите только 0 или 1.\n";
    }
    return false;
}

bool ConsoleApp::readEnergy(double& value) {
    std::string line;
    while (readLine("Суточная энергия (кВт·ч, дробная часть через точку): ", line)) {
        if (parseValue(line, value) && std::isfinite(value) && value >= 0.0) {
            return true;
        }
        output_ << "Ошибка: введите конечное неотрицательное число; "
                << "дробный разделитель — точка.\n";
    }
    return false;
}

bool ConsoleApp::addVehicle() {
    int type = 0;
    int ease = 0;
    bool brakes = false;
    bool frame = false;
    double energy = 0.0;
    std::string name;
    std::string number;

    if (!readInteger("Вид транспорта (1 — велосипед, 2 — электросамокат): ",
                     1, 2, type)
        || !readText("Название: ", name)
        || !readText("Инвентарный номер: ", number)
        || !readInteger("Простота для новичка (1–10): ", 1, 10, ease)
        || !readBoolean("Тормоза исправны (0 — нет, 1 — да): ", brakes)
        || !readBoolean("Рама не повреждена (0 — нет, 1 — да): ", frame)
        || (type == 2 && !readEnergy(energy))) {
        return false;
    }

    std::unique_ptr<Vehicle> vehicle;
    try {
        if (type == 1) {
            vehicle = std::make_unique<Bicycle>(name, number, ease, brakes, frame);
        } else {
            vehicle = std::make_unique<ElectricScooter>(
                name, number, ease, brakes, frame, energy);
        }
    } catch (const std::invalid_argument& error) {
        output_ << "Ошибка данных: " << error.what() << ". Транспорт не добавлен.\n";
        return true;
    }

    output_ << (fleet_.addVehicle(std::move(vehicle))
                    ? "Транспорт принят.\n"
                    : "Отказ: номер уже занят или техосмотр не пройден\n");
    return true;
}

bool ConsoleApp::addItem() {
    int type = 0;
    double energy = 0.0;
    std::string name;
    std::string number;

    if (!readInteger("Вид вещи (1 — шлем, 2 — зарядная станция): ", 1, 2, type)
        || !readText("Название: ", name)
        || !readText("Инвентарный номер: ", number)
        || (type == 2 && !readEnergy(energy))) {
        return false;
    }

    std::unique_ptr<Item> item;
    try {
        if (type == 1) {
            item = std::make_unique<Helmet>(name, number);
        } else {
            item = std::make_unique<ChargingStation>(name, number, energy);
        }
    } catch (const std::invalid_argument& error) {
        output_ << "Ошибка данных: " << error.what() << ". Вещь не добавлена.\n";
        return true;
    }

    output_ << (fleet_.addItem(std::move(item))
                    ? "Вещь принята.\n"
                    : "Отказ: номер уже занят\n");
    return true;
}

void ConsoleApp::showSummary() {
    std::ostringstream report;
    report.imbue(std::locale::classic());
    report << "Транспорт: " << fleet_.getVehicleCount() << '\n'
           << "Вещи: " << fleet_.getItemCount() << '\n'
           << "Всего имущества: " << fleet_.getTotalCount() << '\n'
           << "Суммарная суточная энергия: "
           << fleet_.getTotalDailyEnergyConsumption() << " кВт·ч\n";
    output_ << report.str();
}

void ConsoleApp::showEntries(const std::vector<InventoryEntry>& entries,
                             const std::string& title,
                             const std::string& emptyMessage) {
    output_ << title << '\n';
    if (entries.empty()) {
        output_ << emptyMessage << '\n';
        return;
    }
    for (const auto& entry : entries) {
        output_ << "Название: " << entry.name
                << " | Номер: " << entry.inventoryNumber << '\n';
    }
}