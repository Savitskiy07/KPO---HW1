#pragma once

#include <string>

class InventoryItem {
public:
    virtual ~InventoryItem() = default;

    virtual const std::string& getName() const = 0;
    virtual const std::string& getInventoryNumber() const = 0;
};