#pragma once

#include "Vehicle.h"

#include <string>
#include <utility>

class Bicycle : public Vehicle {
public:
    Bicycle(std::string name, std::string inventoryNumber, int beginnerEase,
            bool brakesWorking, bool frameUndamaged)
        : Vehicle(std::move(name), std::move(inventoryNumber), beginnerEase,
                  brakesWorking, frameUndamaged) {}
};