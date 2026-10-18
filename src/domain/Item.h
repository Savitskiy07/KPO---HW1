#pragma once

#include "InventoryItem.h"

#include <string>

class Item : public InventoryItem {
public:
    const std::string& getName() const override;
    const std::string& getInventoryNumber() const override;

protected:
    Item(std::string name, std::string inventoryNumber);

private:
    std::string name_;
    std::string inventoryNumber_;
};