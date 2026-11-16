add_executable(StagedEpubTest
  StagedEpubTest.cpp
  ${REPO_ROOT}/src/util/StagedEpub.cpp
)

target_include_directories(StagedEpubTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs
  ${REPO_ROOT}/src/util
)

target_link_libraries(StagedEpubTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(StagedEpubTest)
