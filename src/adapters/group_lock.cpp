#include "pos/adapters/files.hpp"

#include <cerrno>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace pos {
namespace fs = std::filesystem;

GroupLock::GroupLock(const fs::path& directory) {
    fs::create_directories(directory);
    const auto file = directory / "journal.lock";
#ifdef _WIN32
    handle_ = CreateFileW(file.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                          OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle_ == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Папка группы занята или недоступна для записи.");
    }
#else
    fd_ = ::open(file.c_str(), O_RDWR | O_CREAT, 0600);
    if (fd_ == -1) {
        throw std::system_error(errno, std::generic_category(), "Не удалось открыть блокировку");
    }
    if (::flock(fd_, LOCK_EX | LOCK_NB) != 0) {
        ::close(fd_);
        throw std::runtime_error("Папка группы занята или недоступна для блокировки.");
    }
#endif
}

GroupLock::~GroupLock() {
#ifdef _WIN32
    CloseHandle(handle_);
#else
    ::close(fd_);
#endif
}

} // namespace pos
