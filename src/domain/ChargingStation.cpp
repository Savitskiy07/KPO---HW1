#include "ChargingStation.h"

#include <cmath>
#include <stdexcept>
#include <utility>

ChargingStation::ChargingStation(std::string name, std::string inventoryNumber,
                                 double dailyEnergyConsumption)
    : Item(std::move(name), std::move(inventoryNumber)),
      dailyEnergyConsumption_(dailyEnergyConsumption) {
    if (!std::isfinite(dailyEnergyConsumption) || dailyEnergyConsumption < 0.0) {
        throw std::invalid_argument("Daily energy must be finite and non-negative");
    }
}

double ChargingStation::getDailyEnergyConsumption() const {
    return dailyEnergyConsumption_;
}