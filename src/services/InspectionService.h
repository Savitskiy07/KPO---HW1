#pragma once

class Vehicle;

class InspectionService {
public:
    virtual ~InspectionService() = default;

    virtual bool inspect(const Vehicle& vehicle) const = 0;
};