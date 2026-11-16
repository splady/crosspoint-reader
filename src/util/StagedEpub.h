#pragma once
#include <string>

enum class StagedEpubResult {
  Installed,             // `dest` now holds the downloaded EPUB
  NotEpub,               // staged file removed; `dest` untouched
  ReplaceFailedKeptOld,  // `dest` untouched; staged file removed
  ReplaceFailedLostOld,  // `dest` was removed but the rename failed; staged file kept as the only copy
};

// Where a download for `dest` is written before installStagedEpub() moves it into place.
std::string stagedEpubPath(const std::string& dest);

// Moves a downloaded file from `staged` over `dest`, but only when it is an EPUB
// (ZIP magic). A server can answer 200 with an HTML error page; checking before
// the swap keeps an existing copy of the book, and its cache, intact.
StagedEpubResult installStagedEpub(const std::string& staged, const std::string& dest);
