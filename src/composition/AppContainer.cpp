#include "AppContainer.h"
#include "services/ServiceCenter.h"

#include <stdexcept>
#include <utility>

namespace {

    const InspectionService& checkedInspection(
        const std::unique_ptr<InspectionService>& inspection) {
        if (!inspection) {
            throw std::invalid_argument("Inspection service must not be null");
        }
        return *inspection;
    }

} // namespace

AppContainer::AppContainer()
    : AppContainer(std::make_unique<ServiceCenter>()) {}

AppContainer::AppContainer(std::unique_ptr<InspectionService> inspection)
    : inspection_(std::move(inspection)),
      fleet_(checkedInspection(inspection_)) {}

FleetService& AppContainer::getFleetService() noexcept {
    return fleet_;
}