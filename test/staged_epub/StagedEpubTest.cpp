#include <gtest/gtest.h>

#include <string>

#include "HalStorage.h"
#include "StagedEpub.h"

namespace {

const std::string kBook = "/Books/Author - Title.epub";
const std::string kStaged = stagedEpubPath(kBook);
const std::string kOldEpub = std::string("PK\x03\x04", 4) + "old book";
const std::string kNewEpub = std::string("PK\x03\x04", 4) + "new book";

class StagedEpubTest : public ::testing::Test {
 protected:
  void SetUp() override {
    fake::files.clear();
    fake::failRename = false;
    fake::failRemove.clear();
  }
};

TEST_F(StagedEpubTest, EpubReplacesTheExistingBook) {
  fake::files[kBook] = kOldEpub;
  fake::files[kStaged] = kNewEpub;
  EXPECT_EQ(installStagedEpub(kStaged, kBook), StagedEpubResult::Installed);
  EXPECT_EQ(fake::files.at(kBook), kNewEpub);
  EXPECT_FALSE(Storage.exists(kStaged.c_str()));
}

TEST_F(StagedEpubTest, EpubInstallsWhenThereIsNoCopyYet) {
  fake::files[kStaged] = kNewEpub;
  EXPECT_EQ(installStagedEpub(kStaged, kBook), StagedEpubResult::Installed);
  EXPECT_EQ(fake::files.at(kBook), kNewEpub);
  EXPECT_FALSE(Storage.exists(kStaged.c_str()));
}

// A server answering 200 with an HTML error page must not cost the reader the book.
TEST_F(StagedEpubTest, HtmlResponseKeepsTheExistingBook) {
  fake::files[kBook] = kOldEpub;
  fake::files[kStaged] = "<!doctype html><title>Sign in</title>";
  EXPECT_EQ(installStagedEpub(kStaged, kBook), StagedEpubResult::NotEpub);
  EXPECT_EQ(fake::files.at(kBook), kOldEpub);
  EXPECT_FALSE(Storage.exists(kStaged.c_str()));
}

TEST_F(StagedEpubTest, ShortOrMissingStagedFileIsNotAnEpub) {
  fake::files[kBook] = kOldEpub;
  fake::files[kStaged] = "PK";
  EXPECT_EQ(installStagedEpub(kStaged, kBook), StagedEpubResult::NotEpub);
  EXPECT_EQ(fake::files.at(kBook), kOldEpub);
  EXPECT_FALSE(Storage.exists(kStaged.c_str()));

  EXPECT_EQ(installStagedEpub(kStaged, kBook), StagedEpubResult::NotEpub);
  EXPECT_EQ(fake::files.at(kBook), kOldEpub);
}

// replaceFile removes the original before renaming: when the rename then fails,
// the staged file is the only copy left and must be kept.
TEST_F(StagedEpubTest, FailedSwapKeepsTheOnlyCopy) {
  fake::files[kBook] = kOldEpub;
  fake::files[kStaged] = kNewEpub;
  fake::failRename = true;
  EXPECT_EQ(installStagedEpub(kStaged, kBook), StagedEpubResult::ReplaceFailedLostOld);
  EXPECT_FALSE(Storage.exists(kBook.c_str()));
  EXPECT_EQ(fake::files.at(kStaged), kNewEpub);
}

// The original could not be removed: it is still the book, so the staged file goes.
TEST_F(StagedEpubTest, FailedRemoveKeepsTheExistingBook) {
  fake::files[kBook] = kOldEpub;
  fake::files[kStaged] = kNewEpub;
  fake::failRemove = kBook;
  EXPECT_EQ(installStagedEpub(kStaged, kBook), StagedEpubResult::ReplaceFailedKeptOld);
  EXPECT_EQ(fake::files.at(kBook), kOldEpub);
  EXPECT_FALSE(Storage.exists(kStaged.c_str()));
}

TEST_F(StagedEpubTest, StagedPathSitsBesideTheBook) { EXPECT_EQ(stagedEpubPath(kBook), kBook + ".new"); }

}  // namespace
