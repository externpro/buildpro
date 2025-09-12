#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

#include <H5Cpp.h>
#include <gtest/gtest.h>

using namespace H5;

class HDF5Test : public ::testing::Test
{
protected:
  static void SetUpTestSuite()
  {
    // Set up any test suite level resources
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
  }

  void SetUp() override
  {
    // Create a unique filename for each test to avoid conflicts
    filename_ = "test_hdf5_" + std::to_string(std::rand()) + ".h5";
  }

  void TearDown() override
  {
    // Clean up test files
    remove(filename_.c_str());
  }

  std::string filename_;
};

TEST_F(HDF5Test, CreateFile)
{
  // Test creating a new HDF5 file
  {
    H5File file(filename_, H5F_ACC_TRUNC);
    EXPECT_TRUE(file.getId() > 0) << "Failed to create HDF5 file";
  }

  // Verify the file exists
  std::ifstream f(filename_.c_str());
  EXPECT_TRUE(f.good()) << "HDF5 file was not created";
}

TEST_F(HDF5Test, CreateGroup)
{
  H5File file(filename_, H5F_ACC_TRUNC);

  // Create a group
  Group group = file.createGroup("/test_group");
  EXPECT_TRUE(group.getId() > 0) << "Failed to create group";

  // Check if group exists in the file
  EXPECT_TRUE(H5Lexists(file.getId(), "/test_group", H5P_DEFAULT) > 0)
    << "Group does not exist in the file";
}

TEST_F(HDF5Test, WriteAndReadDataset)
{
  H5File file(filename_, H5F_ACC_TRUNC);

  // Create a simple dataset
  const int ROWS = 3;
  const int COLS = 4;
  int data[ROWS][COLS] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}};

  // Define the dataspace
  hsize_t dims[2] = {ROWS, COLS};
  DataSpace dataspace(2, dims);

  // Create the dataset
  DataSet dataset =
    file.createDataSet("/test_dataset", PredType::NATIVE_INT, dataspace);

  // Write data to the dataset
  dataset.write(data, PredType::NATIVE_INT);

  // Read the data back
  int read_data[ROWS][COLS];
  dataset.read(read_data, PredType::NATIVE_INT);

  // Verify the data
  for (int i = 0; i < ROWS; ++i)
  {
    for (int j = 0; j < COLS; ++j)
    {
      EXPECT_EQ(data[i][j], read_data[i][j])
        << "Data mismatch at (" << i << ", " << j << ")";
    }
  }
}

TEST_F(HDF5Test, WriteAndReadAttribute)
{
  H5File file(filename_, H5F_ACC_TRUNC);

  // Create a group
  Group group = file.createGroup("/test_group");

  // Create an attribute
  const std::string attr_value = "Test attribute value";
  DataSpace attr_dataspace(H5S_SCALAR);
  StrType strdatatype(PredType::C_S1, attr_value.length() + 1);

  Attribute attribute =
    group.createAttribute("test_attribute", strdatatype, attr_dataspace);

  // Write the attribute
  attribute.write(strdatatype, attr_value);

  // Read the attribute back
  std::string read_value(attr_value.length() + 1, '\0');
  attribute.read(strdatatype, read_value);
  read_value.resize(attr_value.length());

  // Verify the attribute value
  EXPECT_EQ(attr_value, read_value) << "Attribute value mismatch";
}
