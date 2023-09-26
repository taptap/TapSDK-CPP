//
// Created by 甘尧 on 2023/9/25.
//

#include "file.h"
#include "logging.h"
#if HAS_UNIX_FD
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif
#include "ghc/fs_std_select.hpp"

namespace tapsdk {

[[nodiscard]] constexpr const char* AccessModeToStr(FileAccessMode mode) {
    switch (mode) {
        case FileAccessMode::Read:
            return "rb";
        case FileAccessMode::Write:
            return "wb";
        case FileAccessMode::Append:
            return "ab";
        case FileAccessMode::ReadWrite:
            return "r+b";
        case FileAccessMode::ReadAppend:
            return "a+b";
        default:
            return "";
    }
}

bool File::Open(tapsdk::FileAccessMode mode) {
    Close();
    file = std::fopen(path.c_str(), AccessModeToStr(mode));
    errno = 0;
    if (IsOpen()) {
        open_mode = mode;
        return true;
    } else {
        const auto ec = std::error_code{errno, std::generic_category()};
        LOG_ERROR("Failed to close the file at path={}, ec_message={}", path, ec.message());
        return false;
    }
}

bool File::IsOpen() { return file; }

bool File::Flush() {
    if (!IsOpen()) {
        return false;
    }
    errno = 0;
    if (std::fflush(file) == 0) {
        return true;
    } else {
        const auto ec = std::error_code{errno, std::generic_category()};
        LOG_ERROR("Failed to flush the file at path={}, ec_message={}", path, ec.message());
        return false;
    }
}

bool File::Commit() {
    if (!IsOpen()) {
        return false;
    }

    errno = 0;

#ifdef _WIN32
    const auto commit_result = std::fflush(file) == 0 && _commit(fileno(file)) == 0;
#else
    const auto commit_result = std::fflush(file) == 0 && fsync(fileno(file)) == 0;
#endif

    if (!commit_result) {
        const auto ec = std::error_code{errno, std::generic_category()};
        LOG_ERROR("Failed to commit the file at path={}, ec_message={}", path, ec.message());
    }

    return commit_result;
}

bool File::Read(void* dest, size_t offset, size_t size) {
#if HAS_UNIX_FD
    return pread(fileno(file), dest, size, offset) == size;
#else
    ASSERT_MSG(offset <= 2_GB, "seek only support 2G file!");
    std::lock_guard guard(lock);
    if (std::fseek(file, offset, SEEK_CUR) == 0) {
        return std::fread(dest, size, 1, file) == size;
    } else {
        return false;
    }
#endif
}

bool File::Write(void* src, size_t offset, size_t size) {
#if HAS_UNIX_FD
    return pwrite(fileno(file), src, size, offset) == size;
#else
    ASSERT_MSG(offset <= 2_GB, "seek only support 2G file!");
    std::lock_guard guard(lock);
    if (std::fseek(file, offset, SEEK_CUR) == 0) {
        return std::fwrite(src, size, 1, file) == size;
    } else {
        return false;
    }
#endif
}

void* File::Map(size_t offset, size_t size) {
    ASSERT_MSG(IsOpen());
#if HAS_UNIX_FD
    int prot{0};
    if ((open_mode & FileAccessMode::Read) != FileAccessMode::None) {
        prot |= PROT_READ;
    }
    if ((open_mode & FileAccessMode::Write) != FileAccessMode::None) {
        prot |= PROT_WRITE;
    }
    return mmap(nullptr, size, prot, MAP_SHARED | MAP_FIXED, fileno(file), offset);
#else
    return {};
#endif
}

bool File::Close() {
    if (!IsOpen()) {
        return false;
    }
    return std::fclose(file);
}

size_t File::Size() {
    try {
        return fs::file_size(path);
    } catch (...) {
        return 0;
    }
}

bool File::Resize(size_t new_size) {
    if (!IsOpen()) {
        return false;
    }

    errno = 0;

#ifdef _WIN32
    const auto set_size_result = _chsize_s(fileno(file), static_cast<s64>(new_size)) == 0;
#else
    const auto set_size_result = ftruncate(fileno(file), static_cast<s64>(new_size)) == 0;
#endif

    if (!set_size_result) {
        const auto ec = std::error_code{errno, std::generic_category()};
        LOG_ERROR("Failed to resize the file at path={}, size={}, ec_message={}",
                  path,
                  new_size,
                  ec.message());
    }

    return set_size_result;
}

bool Unmap(void* mem, size_t size) {
#if HAS_UNIX_FD
    return munmap(mem, size) == 0;
#else
    return false;
#endif
}

}  // namespace tapsdk
