#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <git2.h>
#include <git2/sys/commit.h>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

class LibGit2Test : public ::testing::Test
{
protected:
  std::string testRepoPath;
  std::string testFilePath;

  void SetUp() override
  {
    // Initialize libgit2
    git_libgit2_init();

    // Create a temporary directory for the test repository
    char tempDirTemplate[] = "/tmp/libgit2_test_XXXXXX";
    char* tempDir = mkdtemp(tempDirTemplate);
    testRepoPath = tempDir;
    testFilePath = testRepoPath + "/test.txt";

    // Create a test file
    std::ofstream testFile(testFilePath);
    testFile << "Test content";
    testFile.close();
  }

  void TearDown() override
  {
    // Cleanup libgit2
    git_libgit2_shutdown();

    // Remove the test directory
    if (!testRepoPath.empty() && fs::exists(testRepoPath))
    {
      fs::remove_all(testRepoPath);
    }
  }

  void CreateTestRepository()
  {
    git_repository* repo = nullptr;
    git_index* index = nullptr;
    git_oid tree_id;
    int error;

    // Initialize a new repository
    error = git_repository_init(&repo, testRepoPath.c_str(), 0);
    if (error != 0)
    {
      const git_error* e = git_error_last();
      std::cerr << "Failed to initialize repository: "
                << (e ? e->message : "No error") << std::endl;
      FAIL();
      return;
    }

    // Get the index
    error = git_repository_index(&index, repo);
    if (error != 0)
    {
      const git_error* e = git_error_last();
      std::cerr << "Failed to get repository index: "
                << (e ? e->message : "No error") << std::endl;
      git_repository_free(repo);
      FAIL();
      return;
    }

    // Add the test file to the index
    error = git_index_add_bypath(index, "test.txt");
    if (error != 0)
    {
      const git_error* e = git_error_last();
      std::cerr << "Failed to add file to index: "
                << (e ? e->message : "No error") << std::endl;
      git_index_free(index);
      git_repository_free(repo);
      FAIL();
      return;
    }

    // Write the index to disk
    error = git_index_write(index);
    if (error != 0)
    {
      const git_error* e = git_error_last();
      std::cerr << "Failed to write index: " << (e ? e->message : "No error")
                << std::endl;
      git_index_free(index);
      git_repository_free(repo);
      FAIL();
      return;
    }

    // Cleanup
    git_index_free(index);
    git_repository_free(repo);
  }
};

TEST_F(LibGit2Test, LibraryInitialization)
{
  // Test that the library was initialized correctly
  const git_error* last_error = git_error_last();
  EXPECT_EQ(nullptr, last_error)
    << "Unexpected error: " << (last_error ? last_error->message : "No error");

  // Test library version
  int major = 0, minor = 0, rev = 0;
  git_libgit2_version(&major, &minor, &rev);
  std::cout << "Using libgit2 version: " << major << "." << minor << "." << rev
            << std::endl;

  EXPECT_GT(major, 0) << "Invalid major version";
  EXPECT_GE(minor, 0) << "Invalid minor version";
  EXPECT_GE(rev, 0) << "Invalid revision";
}

TEST_F(LibGit2Test, RepositoryInitialization)
{
  git_repository* repo = nullptr;

  // Initialize a new repository
  int error = git_repository_init(&repo, testRepoPath.c_str(), 0);
  ASSERT_EQ(0, error) << "Failed to initialize repository: "
                      << git_error_last()->message;

  // Check if the .git directory was created
  std::string gitDir = testRepoPath + "/.git";
  EXPECT_TRUE(fs::exists(gitDir)) << ".git directory not found";

  // Cleanup
  git_repository_free(repo);
}

TEST_F(LibGit2Test, BasicGitOperations)
{
  // Create a test repository
  CreateTestRepository();

  // Open the repository
  git_repository* repo = nullptr;
  int error = git_repository_open(&repo, testRepoPath.c_str());
  ASSERT_EQ(0, error) << "Failed to open repository: "
                      << (git_error_last() ? git_error_last()->message
                                           : "No error");

  // Verify the repository is not bare
  EXPECT_FALSE(git_repository_is_bare(repo)) << "Repository should not be bare";

  // Get the working directory
  const char* workdir = git_repository_workdir(repo);
  EXPECT_NE(nullptr, workdir) << "Failed to get working directory";

  // Check if the .git directory exists
  std::string gitDir = std::string(workdir) + ".git";
  EXPECT_TRUE(fs::exists(gitDir)) << ".git directory not found";

  // Cleanup
  git_repository_free(repo);
}

TEST_F(LibGit2Test, ObjectDatabase)
{
  // Create a test repository
  CreateTestRepository();

  // Open the repository
  git_repository* repo = nullptr;
  int error = git_repository_open(&repo, testRepoPath.c_str());
  ASSERT_EQ(0, error) << "Failed to open repository: "
                      << (git_error_last() ? git_error_last()->message
                                           : "No error");

  // Get the object database
  git_odb* odb = nullptr;
  error = git_repository_odb(&odb, repo);
  ASSERT_EQ(0, error) << "Failed to get object database: "
                      << (git_error_last() ? git_error_last()->message
                                           : "No error");

  // Check if we can read from the ODB
  git_odb_object* obj = nullptr;
  git_oid oid;

  // Get the HEAD reference (should be empty since we haven't created any
  // commits)
  git_reference* head_ref = nullptr;
  error = git_reference_lookup(&head_ref, repo, "HEAD");

  // It's okay if HEAD doesn't exist yet
  if (error == 0)
  {
    // If HEAD exists, verify it's a valid reference
    EXPECT_NE(nullptr, head_ref) << "HEAD reference is null";
    git_reference_free(head_ref);
  }

  // Cleanup
  git_odb_free(odb);
  git_repository_free(repo);
}
