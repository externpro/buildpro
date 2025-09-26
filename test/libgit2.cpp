#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>

#include <direct.h>
#include <io.h>
#define ACCESS _access
#define MKDIR(d) _mkdir(d)
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define ACCESS access
#define MKDIR(d) mkdir(d, 0700)
#endif

#include <git2.h>
#include <git2/sys/commit.h>
#include <gtest/gtest.h>

// Helper functions to replace std::filesystem
bool path_exists(const std::string& path)
{
  return ACCESS(path.c_str(), 0) == 0;
}

// Cross-platform function to create a temporary directory
std::string createTempDirectory()
{
  std::string tempDir;

#ifdef _WIN32
  // Get the Windows temporary path
  char tempPath[MAX_PATH];
  if (GetTempPathA(MAX_PATH, tempPath) == 0)
  {
    throw std::runtime_error("Failed to get temporary path");
  }

  // Generate a unique directory name
  char tempDirName[MAX_PATH];
  if (GetTempFileNameA(tempPath, "git", 0, tempDirName) == 0)
  {
    throw std::runtime_error("Failed to generate temporary directory name");
  }

  // Delete the file that GetTempFileName created and make a directory instead
  DeleteFileA(tempDirName);
  if (MKDIR(tempDirName) != 0)
  {
    throw std::runtime_error("Failed to create temporary directory");
  }

  tempDir = tempDirName;
#else
  // Use mkdtemp on Unix systems
  char tempDirTemplate[] = "/tmp/libgit2_test_XXXXXX";
  char* dirName = mkdtemp(tempDirTemplate);
  if (dirName == nullptr)
  {
    throw std::runtime_error("Failed to create temporary directory");
  }
  tempDir = dirName;
#endif

  return tempDir;
}

// Recursively remove a directory and its contents
void remove_directory(const std::string& path)
{
#ifdef _WIN32
  // Windows implementation using FindFirstFile/FindNextFile
  WIN32_FIND_DATAA ffd;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  std::string search_path = path + "\\*";

  // Find the first file in the directory
  hFind = FindFirstFileA(search_path.c_str(), &ffd);

  if (hFind == INVALID_HANDLE_VALUE)
  {
    return;
  }

  // Process all entries in the directory
  do
  {
    // Skip . and .. entries
    if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
    {
      continue;
    }

    std::string full_path = path + "\\" + ffd.cFileName;

    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      // Recursively remove subdirectory
      remove_directory(full_path);
    }
    else
    {
      // Remove file
      DeleteFileA(full_path.c_str());
    }
  } while (FindNextFileA(hFind, &ffd) != 0);

  // Close the find handle
  FindClose(hFind);

  // Remove the directory itself
  RemoveDirectoryA(path.c_str());
#else
  // Unix implementation using dirent.h
  DIR* dir = opendir(path.c_str());
  if (dir == nullptr)
  {
    return;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr)
  {
    // Skip . and .. entries
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
    {
      continue;
    }

    std::string full_path = path + "/" + entry->d_name;
    struct stat statbuf;
    if (stat(full_path.c_str(), &statbuf) == -1)
    {
      continue;
    }

    if (S_ISDIR(statbuf.st_mode))
    {
      // Recursively remove subdirectory
      remove_directory(full_path);
    }
    else
    {
      // Remove file
      unlink(full_path.c_str());
    }
  }

  closedir(dir);
  rmdir(path.c_str());
#endif
}

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
    testRepoPath = createTempDirectory();
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
    if (!testRepoPath.empty() && path_exists(testRepoPath))
    {
      remove_directory(testRepoPath);
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
  EXPECT_TRUE(path_exists(gitDir)) << ".git directory not found";

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
  EXPECT_TRUE(path_exists(gitDir)) << ".git directory not found";

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
