#pragma once

#include "EnergyConsumer.h"
#include "Vehicle.h"

#include <string>

class ElectricScooter : public Vehicle, public EnergyConsumer {
public:
    ElectricScooter(std::string name, std::string inventoryNumber, int beginnerEase,
                    bool brakesWorking, bool frameUndamaged,
                    double dailyEnergyConsumption);

    double getDailyEnergyConsumption() const override;

private:
    double dailyEnergyConsumption_;
};