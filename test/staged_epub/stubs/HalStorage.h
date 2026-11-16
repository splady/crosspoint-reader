#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>

// In-memory SD card: just the calls installStagedEpub() makes.
namespace fake {
inline std::map<std::string, std::string> files;
inline bool failRename = false;
inline std::string failRemove;  // remove() of this path fails
}  // namespace fake

class HalFile {
 public:
  const std::string* bytes = nullptr;
  size_t pos = 0;

  bool isOpen() const { return bytes != nullptr; }
  bool close() {
    bytes = nullptr;
    return true;
  }
  int read(void* out, size_t size) {
    if (!bytes) return -1;
    size = std::min(size, bytes->size() - pos);
    std::memcpy(out, bytes->data() + pos, size);
    pos += size;
    return static_cast<int>(size);
  }
};

class HalStorage {
 public:
  static HalStorage& getInstance() {
    static HalStorage storage;
    return storage;
  }
  bool openFileForRead(const char*, const char* path, HalFile& file) {
    const auto it = fake::files.find(path);
    if (it == fake::files.end()) return false;
    file.bytes = &it->second;
    file.pos = 0;
    return true;
  }
  bool exists(const char* path) const { return fake::files.count(path) != 0; }
  bool remove(const char* path) { return fake::failRemove != path && fake::files.erase(path) != 0; }
  // As on the device: FAT rename does not replace, so the original goes first.
  bool replaceFile(const char* tmpPath, const char* path) {
    if (exists(path) && !remove(path)) return false;
    if (fake::failRename || !exists(tmpPath)) return false;
    fake::files[path] = std::move(fake::files[tmpPath]);
    fake::files.erase(tmpPath);
    return true;
  }
};

#define Storage HalStorage::getInstance()
