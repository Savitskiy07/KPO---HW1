#pragma once

#include "EnergyConsumer.h"
#include "Item.h"

#include <string>

class ChargingStation : public Item, public EnergyConsumer {
public:
    ChargingStation(std::string name, std::string inventoryNumber,
                    double dailyEnergyConsumption);

    double getDailyEnergyConsumption() const override;

private:
    double dailyEnergyConsumption_;
};