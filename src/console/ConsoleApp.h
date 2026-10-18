#pragma once

#include "services/FleetService.h"

#include <iosfwd>
#include <string>
#include <vector>

class ConsoleApp {
public:
    ConsoleApp(FleetService& fleet, std::istream& input, std::ostream& output);

    void run();

private:
    bool readLine(const std::string& prompt, std::string& value);
    bool readText(const std::string& prompt, std::string& value);
    bool readInteger(const std::string& prompt, int minimum, int maximum,
                     int& value);
    bool readBoolean(const std::string& prompt, bool& value);
    bool readEnergy(double& value);
    bool addVehicle();
    bool addItem();
    void showSummary();
    void showEntries(const std::vector<InventoryEntry>& entries,
                     const std::string& title, const std::string& emptyMessage);

    FleetService& fleet_;
    std::istream& input_;
    std::ostream& output_;
};