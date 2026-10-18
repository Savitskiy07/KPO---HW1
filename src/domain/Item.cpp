#include "Item.h"

#include <utility>

Item::Item(std::string name, std::string inventoryNumber)
    : name_(std::move(name)), inventoryNumber_(std::move(inventoryNumber)) {}

const std::string& Item::getName() const {
    return name_;
}

const std::string& Item::getInventoryNumber() const {
    return inventoryNumber_;
}