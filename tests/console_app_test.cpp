#include "doctest.h"
#include "console/ConsoleApp.h"
#include "services/ServiceCenter.h"

#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::string runConsole(FleetService& fleet, const std::string& script) {
    std::istringstream input(script);
    std::ostringstream output;
    ConsoleApp app(fleet, input, output);
    app.run();
    return output.str();
}

bool contains(const std::string& text, const std::string& fragment) {
    return text.find(fragment) != std::string::npos;
}

class CountingInspection : public InspectionService {
public:
    bool decision = true;
    mutable int calls = 0;

    bool inspect(const Vehicle&) const override {
        ++calls;
        return decision;
    }
};

class ThrowingInspection : public InspectionService {
public:
    bool inspect(const Vehicle&) const override {
        throw std::runtime_error("Inspection unavailable");
    }
};

} // namespace

TEST_CASE("Console adds all four types and prints reports and lists") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);
    const auto report = runConsole(fleet,
        "1\n1\nГородской велосипед\n001\n5\n1\n1\n"
        "1\n2\nУчебный самокат\nE001\n6\n1\n1\n1.25\n"
        "2\n1\nЗащитный шлем\nH001\n"
        "2\n2\nЗарядная станция\nC001\n2.4\n3\n0\n");

    CHECK(fleet.getVehicleCount() == 2);
    CHECK(fleet.getItemCount() == 2);
    CHECK(fleet.getTotalCount() == 4);
    CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(3.65));
    CHECK(contains(report, "Транспорт принят."));
    CHECK(contains(report, "Вещь принята."));
    CHECK(contains(report, "Транспорт: 2"));
    CHECK(contains(report, "Вещи: 2"));
    CHECK(contains(report, "Всего имущества: 4"));
    CHECK(contains(report, "Суммарная суточная энергия: 3.65 кВт·ч"));
    CHECK(contains(report, "дробная часть через точку"));

    const auto beginners = runConsole(fleet, "4\n0\n");
    CHECK(contains(beginners, "Название: Учебный самокат | Номер: E001"));
    CHECK_FALSE(contains(beginners, "Городской велосипед"));
    CHECK_FALSE(contains(beginners, "Защитный шлем"));
    CHECK_FALSE(contains(beginners, "Зарядная станция"));

    const auto inventory = runConsole(fleet, "5\n0\n");
    CHECK(contains(inventory, "Название: Городской велосипед | Номер: 001"));
    CHECK(contains(inventory, "Название: Учебный самокат | Номер: E001"));
    CHECK(contains(inventory, "Название: Защитный шлем | Номер: H001"));
    CHECK(contains(inventory, "Название: Зарядная станция | Номер: C001"));
    CHECK(fleet.getTotalCount() == 4);
}

TEST_CASE("Console rejects faulty transport and then accepts a sound replacement") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);
    const auto output = runConsole(fleet,
        "1\n2\nBroken\nSAME\n6\n0\n1\n9.5\n"
        "1\n1\nSound\nSAME\n6\n1\n1\n0\n");

    REQUIRE(fleet.getInventory().size() == 1);
    CHECK(fleet.getInventory()[0].name == "Sound");
    CHECK(fleet.getVehicleCount() == 1);
    CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(0.0));
    CHECK(contains(output, "Отказ: номер уже занят или техосмотр не пройден"));
    CHECK(contains(output, "Транспорт принят."));
}

TEST_CASE("Console delegates inspection without repeating or overriding it") {
    CountingInspection inspection;
    FleetService fleet(inspection);
    runConsole(fleet, "1\n1\nBroken\nB001\n6\n0\n0\n0\n");
    CHECK(fleet.getVehicleCount() == 1);
    CHECK(inspection.calls == 1);

    inspection.decision = false;
    const auto output = runConsole(fleet,
        "1\n1\nSound\nB002\n6\n1\n1\n"
        "2\n1\nHelmet\nH001\n3\n4\n5\n0\n");
    CHECK(fleet.getVehicleCount() == 1);
    CHECK(fleet.getItemCount() == 1);
    CHECK(inspection.calls == 2);
    CHECK(contains(output, "Отказ: номер уже занят или техосмотр не пройден"));
}

TEST_CASE("Console rejects duplicate numbers across vehicles and items") {
    CountingInspection inspection;
    FleetService fleet(inspection);
    const auto output = runConsole(fleet,
        "2\n2\nStation\nSHARED\n2\n"
        "1\n2\nDuplicate scooter\nSHARED\n6\n1\n1\n9\n"
        "2\n1\nDuplicate helmet\nSHARED\n"
        "1\n1\nBike\nB001\n6\n1\n1\n"
        "2\n2\nDuplicate station\nB001\n10\n0\n");

    CHECK(fleet.getVehicleCount() == 1);
    CHECK(fleet.getItemCount() == 1);
    CHECK(fleet.getTotalCount() == 2);
    CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(2.0));
    CHECK(inspection.calls == 1);
    CHECK(contains(output, "Отказ: номер уже занят или техосмотр не пройден\n"));
    CHECK(contains(output, "Отказ: номер уже занят\n"));
}

TEST_CASE("Console preserves names with spaces and exact string numbers") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);
    runConsole(fleet,
        "2\n1\n  City helmet  \n001\n"
        "2\n1\nSecond\n1\n"
        "2\n1\nThird\nabc\n"
        "2\n1\nFourth\nABC\n"
        "2\n1\nFifth\n ABC \n0\n");

    const auto entries = fleet.getInventory();
    REQUIRE(entries.size() == 5);
    CHECK(entries[0].name == "  City helmet  ");
    CHECK(entries[0].inventoryNumber == "001");
    CHECK(entries[1].inventoryNumber == "1");
    CHECK(entries[2].inventoryNumber == "abc");
    CHECK(entries[3].inventoryNumber == "ABC");
    CHECK(entries[4].inventoryNumber == " ABC ");
}

TEST_CASE("Console explains empty lists and reports zero totals") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);
    const auto output = runConsole(fleet, "3\n4\n5\n0\n");

    CHECK(contains(output, "Транспорт: 0"));
    CHECK(contains(output, "Вещи: 0"));
    CHECK(contains(output, "Всего имущества: 0"));
    CHECK(contains(output, "Суммарная суточная энергия: 0 кВт·ч"));
    CHECK(contains(output, "Транспорта для новичков пока нет."));
    CHECK(contains(output, "Имущество пока не добавлено."));
    CHECK(fleet.getTotalCount() == 0);
}

TEST_CASE("Console recovers from unknown and malformed menu commands") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);
    const auto output = runConsole(fleet,
        "99\nword\n1 2\n999999999999999999999999\n3\n0\n");

    CHECK(contains(output, "Ошибка: введите целое число от 0 до 5."));
    CHECK(contains(output, "Всего имущества: 0"));
    CHECK(contains(output, "До свидания!"));
    CHECK(fleet.getTotalCount() == 0);
}

TEST_CASE("Console retries invalid vehicle and item selections") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);
    const auto output = runConsole(fleet,
        "1\n0\n3\n2abc\n1\nBike\nB001\n6\n1\n1\n"
        "2\n0\n5\n1\nHelmet\nH001\n0\n");

    CHECK(contains(output, "Ошибка: введите целое число от 1 до 2."));
    CHECK(fleet.getVehicleCount() == 1);
    CHECK(fleet.getItemCount() == 1);
}

TEST_CASE("Console retries invalid integer syntax overflow and ease bounds") {
    for (const std::string invalid : {"", "6abc", "6 7", "6.0", "0", "11",
                                      "-1", "999999999999999999999999"}) {
        CAPTURE(invalid);
        const ServiceCenter inspection;
        FleetService fleet(inspection);
        const auto output = runConsole(fleet,
            "1\n1\nBike\nB001\n" + invalid + "\n6\n1\n1\n0\n");

        CHECK(contains(output, "Ошибка: введите целое число от 1 до 10."));
        CHECK(fleet.getVehicleCount() == 1);
        CHECK(fleet.getBeginnerVehicles().size() == 1);
    }
}

TEST_CASE("Console retries invalid energy for both consumer types") {
    const std::vector<std::string> prefixes = {
        "1\n2\nScooter\nE001\n6\n1\n1\n",
        "2\n2\nStation\nC001\n"
    };
    for (const auto& prefix : prefixes) {
        for (const std::string invalid : {"", "-0.1", "nan", "inf", "-inf",
                                          "1e999", "2.4abc", "2.4 3", "2,4"}) {
            CAPTURE(prefix);
            CAPTURE(invalid);
            const ServiceCenter inspection;
            FleetService fleet(inspection);
            const auto output = runConsole(fleet, prefix + invalid + "\n2.5\n0\n");

            CHECK(contains(output, "Ошибка: введите конечное неотрицательное число"));
            CHECK(fleet.getTotalCount() == 1);
            CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(2.5));
        }
    }
}

TEST_CASE("Console accepts zero daily energy") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);
    const auto output = runConsole(fleet,
        "1\n2\nScooter\nE001\n6\n1\n1\n0\n"
        "2\n2\nStation\nC001\n0\n0\n");

    CHECK(fleet.getVehicleCount() == 1);
    CHECK(fleet.getItemCount() == 1);
    CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(0.0));
    CHECK_FALSE(contains(output, "Ошибка:"));
}

TEST_CASE("Console accepts only literal zero or one for boolean fields") {
    for (const std::string invalid : {"", "2", "-1", "true", "01", "+1",
                                      "1.0", "1 0"}) {
        CAPTURE(invalid);
        const ServiceCenter inspection;
        FleetService fleet(inspection);
        const auto output = runConsole(fleet,
            "1\n1\nBike\nB001\n6\n" + invalid + "\n1\n" + invalid + "\n1\n0\n");

        CHECK(contains(output, "Ошибка: введите только 0 или 1."));
        CHECK(fleet.getVehicleCount() == 1);
    }
}

TEST_CASE("Console retries empty or whitespace names and inventory numbers") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);
    const auto output = runConsole(fleet,
        "1\n1\n\n  \nCity bike\n\n \t\n B001 \n6\n1\n1\n"
        "2\n1\n \nHelmet\n\nH001\n0\n");

    const auto entries = fleet.getInventory();
    REQUIRE(entries.size() == 2);
    CHECK(entries[0].name == "City bike");
    CHECK(entries[0].inventoryNumber == " B001 ");
    CHECK(entries[1].name == "Helmet");
    CHECK(entries[1].inventoryNumber == "H001");
    CHECK(contains(output, "Ошибка: значение не должно быть пустым"));
}

TEST_CASE("Exit stops before reading subsequent commands") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);
    const auto output = runConsole(fleet, "0\n2\n1\nUnused\nH001\n");

    CHECK(fleet.getTotalCount() == 0);
    CHECK(contains(output, "До свидания!"));
    CHECK_FALSE(contains(output, "Название: "));
    CHECK_FALSE(contains(output, "Ввод завершён."));
}

TEST_CASE("EOF in the menu ends the console session") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);
    const auto output = runConsole(fleet, "");

    CHECK(fleet.getTotalCount() == 0);
    CHECK(contains(output, "Ввод завершён."));
}

TEST_CASE("EOF at every unfinished field preserves previously accepted inventory") {
    const std::vector<std::vector<std::string>> additions = {
        {"1", "1", "Bike", "B001", "6", "1", "1"},
        {"1", "2", "Scooter", "E001", "6", "1", "1", "1.25"},
        {"2", "1", "Helmet", "H001"},
        {"2", "2", "Station", "C001", "2.4"}
    };
    for (const auto& fields : additions) {
        for (std::size_t length = 1; length < fields.size(); ++length) {
            std::string partial;
            for (std::size_t i = 0; i < length; ++i) {
                partial += fields[i] + '\n';
            }
            CAPTURE(partial);
            const ServiceCenter inspection;
            FleetService fleet(inspection);
            const auto output = runConsole(fleet,
                "2\n2\nKept station\nKEPT\n0.5\n" + partial);

            REQUIRE(fleet.getInventory().size() == 1);
            CHECK(fleet.getInventory()[0].inventoryNumber == "KEPT");
            CHECK(fleet.getVehicleCount() == 0);
            CHECK(fleet.getItemCount() == 1);
            CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(0.5));
            CHECK(contains(output, "Ввод завершён."));
        }
    }
}

TEST_CASE("EOF during a retry does not create a partial object") {
    for (const std::string script : {
             "1\n1\nBike\nB001\n6abc\n",
             "1\n1\nBike\nB001\n6\n2\n",
             "2\n2\nStation\nC001\nnan\n",
             "2\n1\n   \n"}) {
        CAPTURE(script);
        const ServiceCenter inspection;
        FleetService fleet(inspection);
        const auto output = runConsole(fleet, script);

        CHECK(fleet.getTotalCount() == 0);
        CHECK(contains(output, "Ошибка:"));
        CHECK(contains(output, "Ввод завершён."));
    }
}

TEST_CASE("A complete final field needs no trailing newline") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);
    const auto output = runConsole(fleet, "2\n1\nHelmet\nH001");

    CHECK(fleet.getItemCount() == 1);
    CHECK(contains(output, "Вещь принята."));
    CHECK(contains(output, "Ввод завершён."));
}

TEST_CASE("Console does not disguise service failures as input errors") {
    const ThrowingInspection inspection;
    FleetService fleet(inspection);

    CHECK_THROWS_AS(runConsole(fleet, "1\n1\nBike\nB001\n6\n1\n1\n0\n"),
                    std::runtime_error);
    CHECK(fleet.getTotalCount() == 0);
}