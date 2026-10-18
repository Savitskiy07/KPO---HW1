#pragma once

#include "InventoryItem.h"

#include <string>

class Vehicle : public InventoryItem {
public:
    const std::string& getName() const override;
    const std::string& getInventoryNumber() const override;
    int getBeginnerEase() const;
    bool hasWorkingBrakes() const;
    bool hasUndamagedFrame() const;

protected:
    Vehicle(std::string name, std::string inventoryNumber, int beginnerEase,
            bool brakesWorking, bool frameUndamaged);

private:
    std::string name_;
    std::string inventoryNumber_;
    int beginnerEase_;
    bool brakesWorking_;
    bool frameUndamaged_;
};