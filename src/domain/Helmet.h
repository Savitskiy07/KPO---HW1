#pragma once

#include "Item.h"

#include <string>
#include <utility>

class Helmet : public Item {
public:
    Helmet(std::string name, std::string inventoryNumber)
        : Item(std::move(name), std::move(inventoryNumber)) {}
};