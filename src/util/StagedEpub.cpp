#include "StagedEpub.h"

#include <HalStorage.h>
#include <Logging.h>

#include <cstdint>

std::string stagedEpubPath(const std::string& dest) { return dest + ".new"; }

StagedEpubResult installStagedEpub(const std::string& staged, const std::string& dest) {
  uint8_t magic[4] = {0};
  {
    HalFile check;
    if (Storage.openFileForRead("OPDS", staged.c_str(), check)) {
      check.read(magic, sizeof(magic));
    }
    if (check.isOpen()) check.close();  // close before any remove() on the same path
  }
  if (!(magic[0] == 'P' && magic[1] == 'K' && magic[2] == 3 && magic[3] == 4)) {
    LOG_ERR("OPDS", "Downloaded file is not an EPUB (magic %02x%02x%02x%02x)", magic[0], magic[1], magic[2], magic[3]);
    Storage.remove(staged.c_str());
    return StagedEpubResult::NotEpub;
  }
  if (!Storage.replaceFile(staged.c_str(), dest.c_str())) {
    LOG_ERR("OPDS", "Replace failed: %s", dest.c_str());
    // replaceFile removes the original before the rename: if it is gone, the
    // staged copy is the only one left, so keep it.
    if (!Storage.exists(dest.c_str())) return StagedEpubResult::ReplaceFailedLostOld;
    Storage.remove(staged.c_str());
    return StagedEpubResult::ReplaceFailedKeptOld;
  }
  return StagedEpubResult::Installed;
}
