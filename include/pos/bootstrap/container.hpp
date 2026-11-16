#pragma once

#include "pos/adapters/console.hpp"
#include "pos/adapters/files.hpp"

namespace pos {

class AppContainer {
public:
    AppContainer(const std::filesystem::path& directory, std::istream& input, std::ostream& output)
        : lock_(directory),
          repository_(directory / "journal.pos"),
          exporter_(directory / "results.csv"),
          app_(repository_, exporter_),
          console_(app_, input, output) {}

    Console& console() { return console_; }

private:
    GroupLock lock_;
    FileRepository repository_;
    CsvExporter exporter_;
    JournalApp app_;
    Console console_;
};

} // namespace pos
