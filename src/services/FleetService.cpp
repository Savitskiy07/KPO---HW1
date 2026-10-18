#include "FleetService.h"
#include "domain/EnergyConsumer.h"

#include <stdexcept>
#include <utility>

FleetService::FleetService(const InspectionService& inspection)
    : inspection_(inspection) {}

bool FleetService::addVehicle(std::unique_ptr<Vehicle> vehicle) {
    if (!vehicle) {
        throw std::invalid_argument("Vehicle must not be null");
    }

    if (isInventoryNumberTaken(vehicle->getInventoryNumber())) {
        return false;
    }

    if (!inspection_.inspect(*vehicle)) {
        return false;
    }

    vehicles_.push_back(std::move(vehicle));
    return true;
}

bool FleetService::addItem(std::unique_ptr<Item> item) {
    if (!item) {
        throw std::invalid_argument("Item must not be null");
    }

    if (isInventoryNumberTaken(item->getInventoryNumber())) {
        return false;
    }

    items_.push_back(std::move(item));
    return true;
}

std::size_t FleetService::getVehicleCount() const {
    return vehicles_.size();
}

std::size_t FleetService::getItemCount() const {
    return items_.size();
}

std::size_t FleetService::getTotalCount() const {
    return getVehicleCount() + getItemCount();
}

double FleetService::getTotalDailyEnergyConsumption() const {
    double total = 0.0;

    for (const auto& vehicle : vehicles_) {
        const auto* consumer = dynamic_cast<const EnergyConsumer*>(vehicle.get());
        if (consumer) {
            total += consumer->getDailyEnergyConsumption();
        }
    }

    for (const auto& item : items_) {
        const auto* consumer = dynamic_cast<const EnergyConsumer*>(item.get());
        if (consumer) {
            total += consumer->getDailyEnergyConsumption();
        }
    }

    return total;
}

std::vector<InventoryEntry> FleetService::getBeginnerVehicles() const {
    std::vector<InventoryEntry> entries;

    for (const auto& vehicle : vehicles_) {
        if (vehicle->getBeginnerEase() >= 6) {
            entries.push_back({vehicle->getName(), vehicle->getInventoryNumber()});
        }
    }

    return entries;
}

std::vector<InventoryEntry> FleetService::getInventory() const {
    std::vector<InventoryEntry> entries;

    for (const auto& vehicle : vehicles_) {
        entries.push_back({vehicle->getName(), vehicle->getInventoryNumber()});
    }

    for (const auto& item : items_) {
        entries.push_back({item->getName(), item->getInventoryNumber()});
    }

    return entries;
}

bool FleetService::isInventoryNumberTaken(const std::string& number) const {
    for (const auto& vehicle : vehicles_) {
        if (vehicle->getInventoryNumber() == number) {
            return true;
        }
    }

    for (const auto& item : items_) {
        if (item->getInventoryNumber() == number) {
            return true;
        }
    }

    return false;
}