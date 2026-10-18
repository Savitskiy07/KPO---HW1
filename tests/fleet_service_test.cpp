#include "doctest.h"
#include "domain/Bicycle.h"
#include "services/FleetService.h"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

namespace {

class StubInspectionService : public InspectionService {
public:
    explicit StubInspectionService(bool decision) : decision_(decision) {}

    bool inspect(const Vehicle&) const override {
        ++callCount_;
        return decision_;
    }

    void setDecision(bool decision) {
        decision_ = decision;
    }

    std::size_t getCallCount() const {
        return callCount_;
    }

private:
    bool decision_;
    mutable std::size_t callCount_ = 0;
};

} // namespace

TEST_CASE("A new fleet is empty") {
    const StubInspectionService inspection(true);
    const FleetService fleet(inspection);

    CHECK(fleet.getVehicleCount() == 0);
    CHECK(inspection.getCallCount() == 0);
}

TEST_CASE("A positive inspection adds a vehicle") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);
    auto bicycle = std::make_unique<Bicycle>("City bike", "F001", 7, true, true);

    CHECK(fleet.addVehicle(std::move(bicycle)));
    CHECK(bicycle == nullptr);
    CHECK(fleet.getVehicleCount() == 1);
    CHECK(inspection.getCallCount() == 1);
}

TEST_CASE("A negative inspection rejects even a sound vehicle") {
    const StubInspectionService inspection(false);
    FleetService fleet(inspection);
    auto bicycle = std::make_unique<Bicycle>("City bike", "F002", 7, true, true);

    CHECK_FALSE(fleet.addVehicle(std::move(bicycle)));
    CHECK(bicycle == nullptr);
    CHECK(fleet.getVehicleCount() == 0);
    CHECK(inspection.getCallCount() == 1);
}

TEST_CASE("The fleet trusts a positive inspection despite broken brakes") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);

    CHECK(fleet.addVehicle(
        std::make_unique<Bicycle>("City bike", "F003", 7, false, true)));
    CHECK(fleet.getVehicleCount() == 1);
    CHECK(inspection.getCallCount() == 1);
}

TEST_CASE("A rejection preserves previously accepted vehicles") {
    StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("City bike", "F004", 7, true, true)));
    REQUIRE(fleet.getVehicleCount() == 1);

    inspection.setDecision(false);

    CHECK_FALSE(fleet.addVehicle(
        std::make_unique<Bicycle>("Another bike", "F005", 7, true, true)));
    CHECK(fleet.getVehicleCount() == 1);
    CHECK(inspection.getCallCount() == 2);
}

TEST_CASE("A null vehicle throws without inspection or changing the fleet") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("City bike", "F006", 7, true, true)));
    REQUIRE(inspection.getCallCount() == 1);

    CHECK_THROWS_AS(fleet.addVehicle(nullptr), std::invalid_argument);
    CHECK(fleet.getVehicleCount() == 1);
    CHECK(inspection.getCallCount() == 1);
}

TEST_CASE("A duplicate inventory number is rejected without inspection") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("First bike", "F007", 7, true, true)));
    REQUIRE(fleet.getVehicleCount() == 1);
    REQUIRE(inspection.getCallCount() == 1);

    auto duplicate = std::make_unique<Bicycle>("Second bike", "F007", 7, true, true);

    CHECK_FALSE(fleet.addVehicle(std::move(duplicate)));
    CHECK(duplicate == nullptr);
    CHECK(fleet.getVehicleCount() == 1);
    CHECK(inspection.getCallCount() == 1);
}

TEST_CASE("Different string inventory numbers can both be accepted") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("First bike", "001", 7, true, true)));
    CHECK(fleet.addVehicle(
        std::make_unique<Bicycle>("Second bike", "1", 7, true, true)));
    CHECK(fleet.getVehicleCount() == 2);
    CHECK(inspection.getCallCount() == 2);
}

TEST_CASE("An inspection rejection does not reserve the inventory number") {
    StubInspectionService inspection(false);
    FleetService fleet(inspection);

    REQUIRE_FALSE(fleet.addVehicle(
        std::make_unique<Bicycle>("Rejected bike", "F008", 7, true, true)));
    REQUIRE(fleet.getVehicleCount() == 0);
    REQUIRE(inspection.getCallCount() == 1);

    inspection.setDecision(true);

    CHECK(fleet.addVehicle(
        std::make_unique<Bicycle>("Another bike", "F008", 7, true, true)));
    CHECK(fleet.getVehicleCount() == 1);
    CHECK(inspection.getCallCount() == 2);
}