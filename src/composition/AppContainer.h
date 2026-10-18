#pragma once

#include "services/FleetService.h"
#include "services/InspectionService.h"

#include <memory>

class AppContainer {
public:
    AppContainer();
    explicit AppContainer(std::unique_ptr<InspectionService> inspection);

    AppContainer(const AppContainer&) = delete;
    AppContainer& operator=(const AppContainer&) = delete;
    AppContainer(AppContainer&&) = delete;
    AppContainer& operator=(AppContainer&&) = delete;

    FleetService& getFleetService() noexcept;

private:
    std::unique_ptr<InspectionService> inspection_;
    FleetService fleet_;
};