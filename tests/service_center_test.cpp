#include "doctest.h"
#include "domain/Bicycle.h"
#include "services/ServiceCenter.h"

TEST_CASE("ServiceCenter accepts working brakes and an undamaged frame") {
    const Bicycle bicycle("City bike", "001", 7, true, true);
    const ServiceCenter center;
    const InspectionService& inspection = center;

    CHECK(inspection.inspect(bicycle));
}

TEST_CASE("ServiceCenter rejects broken brakes and an undamaged frame") {
    const Bicycle bicycle("City bike", "002", 7, false, true);
    const ServiceCenter center;

    CHECK_FALSE(center.inspect(bicycle));
}

TEST_CASE("ServiceCenter rejects working brakes and a damaged frame") {
    const Bicycle bicycle("City bike", "003", 7, true, false);
    const ServiceCenter center;

    CHECK_FALSE(center.inspect(bicycle));
}

TEST_CASE("ServiceCenter rejects broken brakes and a damaged frame") {
    const Bicycle bicycle("City bike", "004", 7, false, false);
    const ServiceCenter center;

    CHECK_FALSE(center.inspect(bicycle));
}

TEST_CASE("ServiceCenter accepts a sound bicycle with beginner ease 1") {
    const Bicycle bicycle("City bike", "005", 1, true, true);
    const ServiceCenter center;

    CHECK(center.inspect(bicycle));
}