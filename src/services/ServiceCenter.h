#pragma once

#include "InspectionService.h"
#include "domain/Vehicle.h"

class ServiceCenter : public InspectionService {
public:
    bool inspect(const Vehicle& vehicle) const override {
        return vehicle.hasWorkingBrakes() && vehicle.hasUndamagedFrame();
    }
};