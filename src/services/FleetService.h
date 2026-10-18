#pragma once

#include "InspectionService.h"
#include "InventoryEntry.h"
#include "domain/Item.h"
#include "domain/Vehicle.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class FleetService {
public:
    explicit FleetService(const InspectionService& inspection);

    bool addVehicle(std::unique_ptr<Vehicle> vehicle);
    bool addItem(std::unique_ptr<Item> item);
    std::size_t getVehicleCount() const;
    std::size_t getItemCount() const;
    std::size_t getTotalCount() const;
    double getTotalDailyEnergyConsumption() const;
    std::vector<InventoryEntry> getBeginnerVehicles() const;
    std::vector<InventoryEntry> getInventory() const;

private:
    bool isInventoryNumberTaken(const std::string& number) const;

    const InspectionService& inspection_;
    std::vector<std::unique_ptr<Vehicle>> vehicles_;
    std::vector<std::unique_ptr<Item>> items_;
};