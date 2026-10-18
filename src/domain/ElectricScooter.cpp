#include "ElectricScooter.h"

#include <cmath>
#include <stdexcept>
#include <utility>

ElectricScooter::ElectricScooter(std::string name, std::string inventoryNumber,
                               int beginnerEase, bool brakesWorking,
                               bool frameUndamaged, double dailyEnergyConsumption)
    : Vehicle(std::move(name), std::move(inventoryNumber), beginnerEase,
              brakesWorking, frameUndamaged),
      dailyEnergyConsumption_(dailyEnergyConsumption) {
    if (!std::isfinite(dailyEnergyConsumption) || dailyEnergyConsumption < 0.0) {
        throw std::invalid_argument("Daily energy must be finite and non-negative");
    }
}

double ElectricScooter::getDailyEnergyConsumption() const {
    return dailyEnergyConsumption_;
}