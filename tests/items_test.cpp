#include "doctest.h"
#include "domain/Bicycle.h"
#include "domain/Helmet.h"
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

    std::size_t getCallCount() const {
        return callCount_;
    }

private:
    bool decision_;
    mutable std::size_t callCount_ = 0;
};

} // namespace

TEST_CASE("A new fleet has no items or total inventory") {
    const StubInspectionService inspection(true);
    const FleetService fleet(inspection);

    CHECK(fleet.getVehicleCount() == 0);
    CHECK(fleet.getItemCount() == 0);
    CHECK(fleet.getTotalCount() == 0);
}

TEST_CASE("Helmet exposes its name and number through InventoryItem") {
    const Helmet helmet("City helmet", "001");
    const InventoryItem& item = helmet;

    CHECK(item.getName() == "City helmet");
    CHECK(item.getInventoryNumber() == "001");
}

TEST_CASE("Adding a helmet changes item and total counts only") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);
    auto helmet = std::make_unique<Helmet>("City helmet", "H001");

    CHECK(fleet.addItem(std::move(helmet)));
    CHECK(helmet == nullptr);
    CHECK(fleet.getItemCount() == 1);
    CHECK(fleet.getVehicleCount() == 0);
    CHECK(fleet.getTotalCount() == 1);
}

TEST_CASE("Adding an item does not call inspection") {
    const StubInspectionService inspection(false);
    FleetService fleet(inspection);

    CHECK(fleet.addItem(std::make_unique<Helmet>("City helmet", "H002")));
    CHECK(fleet.getItemCount() == 1);
    CHECK(inspection.getCallCount() == 0);
}

TEST_CASE("A second item with the same number is rejected") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addItem(std::make_unique<Helmet>("First helmet", "H003")));
    auto duplicate = std::make_unique<Helmet>("Second helmet", "H003");

    CHECK_FALSE(fleet.addItem(std::move(duplicate)));
    CHECK(duplicate == nullptr);
    CHECK(fleet.getItemCount() == 1);
    CHECK(fleet.getVehicleCount() == 0);
    CHECK(fleet.getTotalCount() == 1);
    CHECK(inspection.getCallCount() == 0);
}

TEST_CASE("An accepted vehicle prevents an item with the same number") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("City bike", "SHARED1", 7, true, true)));
    REQUIRE(inspection.getCallCount() == 1);

    CHECK_FALSE(fleet.addItem(std::make_unique<Helmet>("City helmet", "SHARED1")));
    CHECK(fleet.getVehicleCount() == 1);
    CHECK(fleet.getItemCount() == 0);
    CHECK(fleet.getTotalCount() == 1);
    CHECK(inspection.getCallCount() == 1);
}

TEST_CASE("An accepted item prevents a vehicle without calling inspection") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addItem(std::make_unique<Helmet>("City helmet", "SHARED2")));
    REQUIRE(inspection.getCallCount() == 0);

    CHECK_FALSE(fleet.addVehicle(
        std::make_unique<Bicycle>("City bike", "SHARED2", 7, true, true)));
    CHECK(fleet.getVehicleCount() == 0);
    CHECK(fleet.getItemCount() == 1);
    CHECK(fleet.getTotalCount() == 1);
    CHECK(inspection.getCallCount() == 0);
}

TEST_CASE("A vehicle and an item with different string numbers coexist") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("City bike", "001", 7, true, true)));
    CHECK(fleet.addItem(std::make_unique<Helmet>("City helmet", "1")));
    CHECK(fleet.getVehicleCount() == 1);
    CHECK(fleet.getItemCount() == 1);
    CHECK(fleet.getTotalCount() == 2);
    CHECK(inspection.getCallCount() == 1);
}

TEST_CASE("An inspection rejection leaves the number available for an item") {
    const StubInspectionService inspection(false);
    FleetService fleet(inspection);

    REQUIRE_FALSE(fleet.addVehicle(
        std::make_unique<Bicycle>("City bike", "SHARED3", 7, true, true)));
    REQUIRE(inspection.getCallCount() == 1);

    CHECK(fleet.addItem(std::make_unique<Helmet>("City helmet", "SHARED3")));
    CHECK(fleet.getVehicleCount() == 0);
    CHECK(fleet.getItemCount() == 1);
    CHECK(fleet.getTotalCount() == 1);
    CHECK(inspection.getCallCount() == 1);
}

TEST_CASE("A null item throws without changing inventory or calling inspection") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("City bike", "V001", 7, true, true)));
    REQUIRE(fleet.addItem(std::make_unique<Helmet>("City helmet", "H004")));
    REQUIRE(inspection.getCallCount() == 1);

    CHECK_THROWS_AS(fleet.addItem(nullptr), std::invalid_argument);
    CHECK(fleet.getVehicleCount() == 1);
    CHECK(fleet.getItemCount() == 1);
    CHECK(fleet.getTotalCount() == 2);
    CHECK(inspection.getCallCount() == 1);
}