#include "Vehicle.h"

#include <stdexcept>
#include <utility>

Vehicle::Vehicle(std::string name, std::string inventoryNumber, int beginnerEase,
                 bool brakesWorking, bool frameUndamaged)
    : name_(std::move(name)),
      inventoryNumber_(std::move(inventoryNumber)),
      beginnerEase_(beginnerEase),
      brakesWorking_(brakesWorking),
      frameUndamaged_(frameUndamaged) {
    if (beginnerEase < 1 || beginnerEase > 10) {
        throw std::invalid_argument("Beginner ease must be between 1 and 10");
    }
}

const std::string& Vehicle::getName() const {
    return name_;
}

const std::string& Vehicle::getInventoryNumber() const {
    return inventoryNumber_;
}

int Vehicle::getBeginnerEase() const {
    return beginnerEase_;
}

bool Vehicle::hasWorkingBrakes() const {
    return brakesWorking_;
}

bool Vehicle::hasUndamagedFrame() const {
    return frameUndamaged_;
}