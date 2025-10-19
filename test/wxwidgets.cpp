#include <fstream>

#include <wx/datetime.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/string.h>
#include <wx/wx.h>

#include <gtest/gtest.h>

// Test fixture for wxWidgets tests
class WxWidgetsTest : public ::testing::Test
{
protected:
  static void SetUpTestSuite()
  {
    // Initialize wxWidgets once for all tests
    int argc = 0;
    char** argv = nullptr;
    static wxInitializer initializer(argc, argv);
    if (!initializer.IsOk())
    {
      FAIL() << "Failed to initialize wxWidgets";
    }
  }
};

// Test basic wxString functionality
TEST_F(WxWidgetsTest, StringOperations)
{
  // String construction and comparison
  wxString str1 = "Hello";
  wxString str2 = "World";
  wxString combined = str1 + " " + str2;

  EXPECT_STREQ("Hello World", combined.mb_str());
  EXPECT_EQ(11, combined.length());

  // String manipulation
  combined.MakeUpper();
  EXPECT_STREQ("HELLO WORLD", combined.mb_str());

  // String formatting
  wxString formatted = wxString::Format("%s %d", "Test", 123);
  EXPECT_STREQ("Test 123", formatted.mb_str());
}

// Test wxDateTime functionality
TEST_F(WxWidgetsTest, DateTimeOperations)
{
  wxDateTime now = wxDateTime::Now();
  wxDateTime today = wxDateTime::Today();

  // Basic date/time validation
  EXPECT_TRUE(now.IsValid());
  EXPECT_TRUE(today.IsValid());

  // Date comparison
  EXPECT_LE(today, now);

  // Date formatting
  wxString dateStr = now.Format("%Y-%m-%d %H:%M:%S");
  EXPECT_FALSE(dateStr.IsEmpty());
}

// Test basic file operations
TEST_F(WxWidgetsTest, FileOperations)
{
  wxString testFileName = wxFileName::CreateTempFileName("wx_test_");
  const std::string testContent = "wxWidgets file operation test";

  // Write to a file using standard C++ streams
  {
    std::ofstream file(testFileName.ToStdString());
    ASSERT_TRUE(file.is_open()) << "Failed to open file for writing";
    file << testContent;
    file.close();
  }

  // Verify file exists
  EXPECT_TRUE(wxFileExists(testFileName)) << "File was not created";

  // Read from the file using standard C++ streams
  std::string readContent;
  {
    std::ifstream file(testFileName.ToStdString());
    ASSERT_TRUE(file.is_open()) << "Failed to open file for reading";
    std::getline(file, readContent);
  }

  // Clean up
  EXPECT_TRUE(wxRemoveFile(testFileName))
    << "Failed to remove test file: " << testFileName;

  // Verify content
  EXPECT_EQ(testContent, readContent) << "File content mismatch";
}

// Test basic GUI components (without creating windows)
TEST_F(WxWidgetsTest, CoreComponents)
{
  // Test basic geometry classes
  wxPoint point(10, 20);
  wxSize size(100, 200);

  EXPECT_EQ(10, point.x);
  EXPECT_EQ(20, point.y);
  EXPECT_EQ(100, size.GetWidth());
  EXPECT_EQ(200, size.GetHeight());

  // Test color class with explicit RGB values
  wxColour red(255, 0, 0); // Red
  EXPECT_EQ(255, red.Red());
  EXPECT_EQ(0, red.Green());
  EXPECT_EQ(0, red.Blue());

  // Test font class
  wxFont font(
    12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
  EXPECT_TRUE(font.IsOk());
  EXPECT_EQ(12, font.GetPointSize());
}

// Test event system with a simple command event
TEST_F(WxWidgetsTest, EventSystem)
{
  bool eventHandled = false;

  // Create a simple event handler
  wxEvtHandler eventHandler;
  eventHandler.Bind(
    wxEVT_IDLE, [&eventHandled](wxIdleEvent&) { eventHandled = true; });

  // Process an idle event
  wxIdleEvent event;
  eventHandler.ProcessEvent(event);

  EXPECT_TRUE(eventHandled);
}
