#include "doctest.h"
#include "domain/Bicycle.h"

#include <stdexcept>

TEST_CASE("Bicycle accepts beginner ease 1 and exposes inventory data") {
    const Bicycle bicycle("City bike", "001", 1, true, true);
    const InventoryItem& item = bicycle;

    CHECK(bicycle.getBeginnerEase() == 1);
    CHECK(item.getName() == "City bike");
    CHECK(item.getInventoryNumber() == "001");
    CHECK(bicycle.hasWorkingBrakes());
    CHECK(bicycle.hasUndamagedFrame());
}

TEST_CASE("Bicycle accepts beginner ease 10 and stores technical condition") {
    const Bicycle bicycle("City bike", "002", 10, false, false);

    CHECK(bicycle.getBeginnerEase() == 10);
    CHECK_FALSE(bicycle.hasWorkingBrakes());
    CHECK_FALSE(bicycle.hasUndamagedFrame());
}

TEST_CASE("Bicycle rejects beginner ease 0") {
    CHECK_THROWS_AS(Bicycle("City bike", "003", 0, true, true),
                    std::invalid_argument);
}

TEST_CASE("Bicycle rejects beginner ease 11") {
    CHECK_THROWS_AS(Bicycle("City bike", "004", 11, true, true),
                    std::invalid_argument);
}