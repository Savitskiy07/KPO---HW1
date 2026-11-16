#pragma once

#include "pos/application/journal_app.hpp"

#include <filesystem>

namespace pos {

class GroupLock {
public:
    explicit GroupLock(const std::filesystem::path& directory);
    ~GroupLock();
    GroupLock(const GroupLock&) = delete;
    GroupLock& operator=(const GroupLock&) = delete;

private:
#ifdef _WIN32
    void* handle_;
#else
    int fd_;
#endif
};

class FileRepository final : public JournalRepository {
public:
    explicit FileRepository(std::filesystem::path file) : file_(std::move(file)) {}
    std::optional<Journal> load() const override;
    void save(const Journal& journal) override;

private:
    std::filesystem::path file_;
};

class CsvExporter final : public ResultExporter {
public:
    explicit CsvExporter(std::filesystem::path file) : file_(std::move(file)) {}
    std::string write(const std::vector<StudentResult>& rows) override;

private:
    std::filesystem::path file_;
};

} // namespace pos
