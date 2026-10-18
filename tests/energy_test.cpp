#include "doctest.h"
#include "domain/Bicycle.h"
#include "domain/ChargingStation.h"
#include "domain/ElectricScooter.h"
#include "domain/EnergyConsumer.h"
#include "domain/Helmet.h"
#include "domain/Item.h"
#include "services/FleetService.h"
#include "services/ServiceCenter.h"

#include <limits>
#include <memory>
#include <stdexcept>

namespace {

class TestEnergyItem : public Item, public EnergyConsumer {
public:
    TestEnergyItem() : Item("Test consumer", "TEST-ENERGY") {}

    double getDailyEnergyConsumption() const override {
        return 0.75;
    }
};

} // namespace

TEST_CASE("An empty fleet consumes zero energy") {
    const ServiceCenter inspection;
    const FleetService fleet(inspection);

    CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(0.0));
}

TEST_CASE("A bicycle and a helmet contribute no energy") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);

    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("City bike", "B001", 7, true, true)));
    REQUIRE(fleet.addItem(std::make_unique<Helmet>("Helmet", "H001")));

    CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(0.0));
}

TEST_CASE("An accepted electric scooter contributes its daily energy") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);

    REQUIRE(fleet.addVehicle(std::make_unique<ElectricScooter>(
        "Scooter", "E001", 7, true, true, 1.25)));

    CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(1.25));
}

TEST_CASE("A charging station contributes its daily energy") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);

    REQUIRE(fleet.addItem(
        std::make_unique<ChargingStation>("Station", "C001", 2.4)));

    CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(2.4));
}

TEST_CASE("A mixed fleet sums only energy consumers") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);

    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("City bike", "B001", 7, true, true)));
    REQUIRE(fleet.addItem(std::make_unique<Helmet>("Helmet", "H001")));
    REQUIRE(fleet.addVehicle(std::make_unique<ElectricScooter>(
        "Scooter", "E001", 7, true, true, 1.25)));
    REQUIRE(fleet.addItem(
        std::make_unique<ChargingStation>("Station", "C001", 2.4)));

    CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(3.65));
}

TEST_CASE("An inspection rejection does not increase total energy") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);

    REQUIRE(fleet.addItem(
        std::make_unique<ChargingStation>("Station", "C001", 2.4)));

    CHECK_FALSE(fleet.addVehicle(std::make_unique<ElectricScooter>(
        "Broken scooter", "E001", 7, false, true, 9.5)));
    CHECK(fleet.getVehicleCount() == 0);
    CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(2.4));
}

TEST_CASE("Duplicate vehicles and items do not increase total energy") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);

    REQUIRE(fleet.addItem(
        std::make_unique<ChargingStation>("Station", "C001", 2.4)));
    REQUIRE(fleet.addVehicle(std::make_unique<ElectricScooter>(
        "Scooter", "E001", 7, true, true, 1.25)));

    CHECK_FALSE(fleet.addVehicle(std::make_unique<ElectricScooter>(
        "Duplicate scooter", "C001", 7, true, true, 10.0)));
    CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(3.65));

    CHECK_FALSE(fleet.addItem(
        std::make_unique<ChargingStation>("Duplicate station", "E001", 20.0)));
    CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(3.65));
    CHECK(fleet.getTotalCount() == 2);
}

TEST_CASE("ElectricScooter permits zero daily energy") {
    const ElectricScooter scooter("Scooter", "E001", 7, true, true, 0.0);

    CHECK(scooter.getDailyEnergyConsumption() == doctest::Approx(0.0));
}

TEST_CASE("ChargingStation permits zero daily energy") {
    const ChargingStation station("Station", "C001", 0.0);

    CHECK(station.getDailyEnergyConsumption() == doctest::Approx(0.0));
}

TEST_CASE("ElectricScooter rejects invalid daily energy") {
    const double invalidValues[] = {
        -0.1,
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity()
    };

    for (double value : invalidValues) {
        CAPTURE(value);
        CHECK_THROWS_AS(
            ElectricScooter("Scooter", "E001", 7, true, true, value),
            std::invalid_argument);
    }
}

TEST_CASE("ChargingStation rejects invalid daily energy") {
    const double invalidValues[] = {
        -0.1,
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity()
    };

    for (double value : invalidValues) {
        CAPTURE(value);
        CHECK_THROWS_AS(ChargingStation("Station", "C001", value),
                        std::invalid_argument);
    }
}

TEST_CASE("A new energy consumer is counted through its interface") {
    const ServiceCenter inspection;
    FleetService fleet(inspection);

    REQUIRE(fleet.addItem(
        std::make_unique<ChargingStation>("Station", "C001", 1.25)));
    REQUIRE(fleet.addItem(std::make_unique<TestEnergyItem>()));

    CHECK(fleet.getTotalDailyEnergyConsumption() == doctest::Approx(2.0));
}