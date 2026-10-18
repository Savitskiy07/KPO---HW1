#pragma once

class EnergyConsumer {
public:
    virtual ~EnergyConsumer() = default;

    // Daily consumption in kWh; finite and non-negative.
    virtual double getDailyEnergyConsumption() const = 0;
};