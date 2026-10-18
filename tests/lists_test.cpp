#include "doctest.h"
#include "domain/Bicycle.h"
#include "domain/ChargingStation.h"
#include "domain/ElectricScooter.h"
#include "domain/Helmet.h"
#include "services/FleetService.h"

#include <cstddef>
#include <memory>

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

TEST_CASE("An empty fleet returns two empty lists") {
    const StubInspectionService inspection(true);
    const FleetService fleet(inspection);

    CHECK(fleet.getBeginnerVehicles().empty());
    CHECK(fleet.getInventory().empty());
    CHECK(inspection.getCallCount() == 0);
}

TEST_CASE("Beginner vehicles use the threshold and vehicle insertion order") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("Excluded bike", "B005", 5, true, true)));
    REQUIRE(fleet.addVehicle(std::make_unique<ElectricScooter>(
        "Z scooter", "E006", 6, true, true, 1.25)));
    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("A bike", "B010", 10, true, true)));

    const auto entries = fleet.getBeginnerVehicles();

    REQUIRE(entries.size() == 2);
    CHECK(entries[0].name == "Z scooter");
    CHECK(entries[0].inventoryNumber == "E006");
    CHECK(entries[1].name == "A bike");
    CHECK(entries[1].inventoryNumber == "B010");
}

TEST_CASE("Items never appear in the beginner list") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addItem(std::make_unique<Helmet>("Helmet", "H001")));
    REQUIRE(fleet.addItem(
        std::make_unique<ChargingStation>("Station", "C001", 2.4)));

    CHECK(fleet.getBeginnerVehicles().empty());
    CHECK(fleet.getInventory().size() == 2);
}

TEST_CASE("Inventory lists vehicles first and preserves order within each group") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addItem(std::make_unique<Helmet>("Z helmet", "H002")));
    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("Z bike", "B002", 5, true, true)));
    REQUIRE(fleet.addItem(
        std::make_unique<ChargingStation>("A station", "C002", 2.4)));
    REQUIRE(fleet.addVehicle(std::make_unique<ElectricScooter>(
        "A scooter", "E002", 6, true, true, 1.25)));

    const auto entries = fleet.getInventory();

    REQUIRE(entries.size() == 4);
    CHECK(entries[0].name == "Z bike");
    CHECK(entries[0].inventoryNumber == "B002");
    CHECK(entries[1].name == "A scooter");
    CHECK(entries[1].inventoryNumber == "E002");
    CHECK(entries[2].name == "Z helmet");
    CHECK(entries[2].inventoryNumber == "H002");
    CHECK(entries[3].name == "A station");
    CHECK(entries[3].inventoryNumber == "C002");
}

TEST_CASE("Rejected vehicles and duplicates are absent from both lists") {
    StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("Accepted bike", "B003", 6, true, true)));
    REQUIRE(fleet.addItem(std::make_unique<Helmet>("Accepted helmet", "H003")));

    inspection.setDecision(false);
    REQUIRE_FALSE(fleet.addVehicle(
        std::make_unique<Bicycle>("Rejected bike", "B004", 10, true, true)));

    inspection.setDecision(true);
    REQUIRE_FALSE(fleet.addVehicle(std::make_unique<ElectricScooter>(
        "Duplicate scooter", "B003", 10, true, true, 1.25)));
    REQUIRE_FALSE(fleet.addItem(std::make_unique<Helmet>("Duplicate helmet", "H003")));

    const auto beginners = fleet.getBeginnerVehicles();
    const auto inventory = fleet.getInventory();

    REQUIRE(beginners.size() == 1);
    CHECK(beginners[0].name == "Accepted bike");
    CHECK(beginners[0].inventoryNumber == "B003");
    REQUIRE(inventory.size() == 2);
    CHECK(inventory[0].name == "Accepted bike");
    CHECK(inventory[0].inventoryNumber == "B003");
    CHECK(inventory[1].name == "Accepted helmet");
    CHECK(inventory[1].inventoryNumber == "H003");
    CHECK(inspection.getCallCount() == 2);
}

TEST_CASE("Reading lists neither repeats inspection nor changes fleet counts") {
    StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("Bike", "B020", 6, true, true)));
    REQUIRE(fleet.addItem(std::make_unique<Helmet>("Helmet", "H020")));
    REQUIRE(inspection.getCallCount() == 1);
    REQUIRE(fleet.getVehicleCount() == 1);
    REQUIRE(fleet.getItemCount() == 1);
    REQUIRE(fleet.getTotalCount() == 2);
    inspection.setDecision(false);

    const FleetService& view = fleet;
    CHECK(view.getBeginnerVehicles().size() == 1);
    CHECK(view.getInventory().size() == 2);
    CHECK(inspection.getCallCount() == 1);
    CHECK(view.getVehicleCount() == 1);
    CHECK(view.getItemCount() == 1);
    CHECK(view.getTotalCount() == 2);
}

TEST_CASE("Changing snapshots does not change stored names or numbers") {
    const StubInspectionService inspection(true);
    FleetService fleet(inspection);

    REQUIRE(fleet.addVehicle(
        std::make_unique<Bicycle>("Original bike", "B030", 6, true, true)));
    REQUIRE(fleet.addItem(std::make_unique<Helmet>("Original helmet", "H030")));

    auto beginners = fleet.getBeginnerVehicles();
    auto inventory = fleet.getInventory();
    REQUIRE(beginners.size() == 1);
    REQUIRE(inventory.size() == 2);

    beginners[0].name = "Changed beginner";
    beginners[0].inventoryNumber = "CHANGED-B";
    inventory[0].name = "Changed vehicle";
    inventory[0].inventoryNumber = "CHANGED-V";
    inventory[1].name = "Changed item";
    inventory[1].inventoryNumber = "CHANGED-I";

    const auto freshBeginners = fleet.getBeginnerVehicles();
    const auto freshInventory = fleet.getInventory();

    REQUIRE(freshBeginners.size() == 1);
    CHECK(freshBeginners[0].name == "Original bike");
    CHECK(freshBeginners[0].inventoryNumber == "B030");
    REQUIRE(freshInventory.size() == 2);
    CHECK(freshInventory[0].name == "Original bike");
    CHECK(freshInventory[0].inventoryNumber == "B030");
    CHECK(freshInventory[1].name == "Original helmet");
    CHECK(freshInventory[1].inventoryNumber == "H030");
    CHECK(fleet.getTotalCount() == 2);
}