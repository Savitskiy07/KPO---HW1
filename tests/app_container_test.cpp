#include "doctest.h"
#include "composition/AppContainer.h"
#include "domain/Bicycle.h"
#include "domain/Helmet.h"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace {

struct InspectionState {
    bool decision = true;
    std::size_t callCount = 0;
    bool alive = false;
};

class StubInspectionService : public InspectionService {
public:
    explicit StubInspectionService(InspectionState& state) : state_(state) {
        state_.alive = true;
    }

    ~StubInspectionService() override {
        state_.alive = false;
    }

    bool inspect(const Vehicle&) const override {
        ++state_.callCount;
        return state_.decision;
    }

private:
    InspectionState& state_;
};

class TrackedBicycle : public Bicycle {
public:
    TrackedBicycle(const InspectionState& state,
                   bool& destroyedWhileInspectionAlive)
        : Bicycle("Tracked bike", "TRACKED", 7, true, true),
          state_(state),
          destroyedWhileInspectionAlive_(destroyedWhileInspectionAlive) {}

    ~TrackedBicycle() override {
        destroyedWhileInspectionAlive_ = state_.alive;
    }

private:
    const InspectionState& state_;
    bool& destroyedWhileInspectionAlive_;
};

static_assert(!std::is_copy_constructible_v<AppContainer>);
static_assert(!std::is_copy_assignable_v<AppContainer>);
static_assert(!std::is_move_constructible_v<AppContainer>);
static_assert(!std::is_move_assignable_v<AppContainer>);

} // namespace

TEST_CASE("The default container uses ServiceCenter rules") {
    AppContainer container;
    auto& fleet = container.getFleetService();

    CHECK(fleet.getTotalCount() == 0);
    CHECK(fleet.addVehicle(
        std::make_unique<Bicycle>("Sound bike", "A001", 7, true, true)));
    CHECK_FALSE(fleet.addVehicle(
        std::make_unique<Bicycle>("Broken brakes", "A002", 7, false, true)));
    CHECK_FALSE(fleet.addVehicle(
        std::make_unique<Bicycle>("Damaged frame", "A003", 7, true, false)));
    CHECK_FALSE(fleet.addVehicle(
        std::make_unique<Bicycle>("Both faults", "A004", 7, false, false)));
    CHECK(fleet.getVehicleCount() == 1);
}

TEST_CASE("The container injects the supplied inspection and takes ownership") {
    InspectionState state;
    auto inspection = std::make_unique<StubInspectionService>(state);
    AppContainer container(std::move(inspection));
    auto& fleet = container.getFleetService();

    CHECK(inspection == nullptr);
    CHECK(state.alive);
    CHECK(state.callCount == 0);
    CHECK(fleet.addVehicle(
        std::make_unique<Bicycle>("Broken bike", "A010", 7, false, false)));
    CHECK(state.callCount == 1);
    CHECK(fleet.getVehicleCount() == 1);

    state.decision = false;

    CHECK_FALSE(fleet.addVehicle(
        std::make_unique<Bicycle>("Sound bike", "A011", 7, true, true)));
    CHECK(state.callCount == 2);
    CHECK(fleet.getVehicleCount() == 1);
}

TEST_CASE("Repeated requests return the same fleet with its existing state") {
    AppContainer container;
    auto& first = container.getFleetService();

    REQUIRE(first.addItem(std::make_unique<Helmet>("Helmet", "A020")));

    auto& second = container.getFleetService();
    CHECK(&first == &second);
    CHECK(second.getItemCount() == 1);
    CHECK_FALSE(second.addItem(
        std::make_unique<Helmet>("Duplicate helmet", "A020")));
    CHECK(first.getTotalCount() == 1);
}

TEST_CASE("Two containers have independent fleets and inspections") {
    InspectionState accepting;
    InspectionState rejecting;
    rejecting.decision = false;
    AppContainer first(std::make_unique<StubInspectionService>(accepting));
    AppContainer second(std::make_unique<StubInspectionService>(rejecting));
    auto& firstFleet = first.getFleetService();
    auto& secondFleet = second.getFleetService();

    CHECK(&firstFleet != &secondFleet);
    REQUIRE(firstFleet.addVehicle(
        std::make_unique<Bicycle>("First bike", "SAME", 7, true, true)));
    CHECK(secondFleet.getTotalCount() == 0);
    CHECK_FALSE(secondFleet.addVehicle(
        std::make_unique<Bicycle>("Second bike", "SAME", 7, true, true)));
    CHECK(accepting.callCount == 1);
    CHECK(rejecting.callCount == 1);
    CHECK(secondFleet.addItem(std::make_unique<Helmet>("Helmet", "SAME")));
    CHECK(firstFleet.getVehicleCount() == 1);
    CHECK(firstFleet.getItemCount() == 0);
    CHECK(secondFleet.getVehicleCount() == 0);
    CHECK(secondFleet.getItemCount() == 1);
}

TEST_CASE("The container rejects a null inspection") {
    CHECK_THROWS_AS(AppContainer(nullptr), std::invalid_argument);
}

TEST_CASE("The container destroys fleet inventory before the inspection") {
    InspectionState state;
    bool destroyedWhileInspectionAlive = false;

    {
        AppContainer container(std::make_unique<StubInspectionService>(state));
        REQUIRE(container.getFleetService().addVehicle(
            std::make_unique<TrackedBicycle>(state, destroyedWhileInspectionAlive)));
        CHECK(state.alive);
        CHECK_FALSE(destroyedWhileInspectionAlive);
    }

    CHECK(destroyedWhileInspectionAlive);
    CHECK_FALSE(state.alive);
}