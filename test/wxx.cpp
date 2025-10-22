#include <wx/button.h>
#include <wx/dcmemory.h>
#include <wx/plotctrl/plotctrl.h>
#include <wx/things/toggle.h>
#include <wx/treelistctrl/treelistctrl.h>
#include <wx/wx.h>

#include <gtest/gtest.h>

// Forward declarations
class wxPlotCtrl;

// Namespace for wxWidgets extensions
namespace wxcode
{
  class wxTreeListCtrl;
} // namespace wxcode

// A minimal wxApp for testing purposes
class MinimalTestApp : public wxApp
{
public:
  virtual bool OnInit() override
  {
    // Create a minimal frame, but we don't need to show it for most unit tests
    wxFrame* frame = new wxFrame(
      nullptr, wxID_ANY, "Test Frame", wxDefaultPosition, wxSize(200, 100));
    SetTopWindow(frame);
    return true;
  }
};

// Use this macro to associate your test app with wxWidgets
IMPLEMENT_APP_NO_MAIN(MinimalTestApp)

// A fixture to initialize and clean up wxWidgets for each test
class WxExtensionsFixture : public testing::Test
{
protected:
  void SetUp() override
  {
    // Initialize wxWidgets
    // wxApp::SetInstance can be used if you need a specific app instance,
    // otherwise wxEntryStart handles it.
    int argc = 1;
    char appName[] = "wxx_test";
    char* argv[] = {appName};
    wxEntryStart(argc, argv);
    wxTheApp->OnInit(); // Manually call OnInit for the test app
  }

  void TearDown() override
  {
    // Clean up wxWidgets
    wxTheApp->OnExit();
    wxEntryCleanup();
  }
};

// Example test using the fixture
TEST_F(WxExtensionsFixture, BasicWxWidgetsComponentCreation)
{
  wxWindow* topWindow = wxTheApp->GetTopWindow();
  // Test creation of a wxButton
  wxButton* button = new wxButton(topWindow, wxID_ANY, "Test Button");
  ASSERT_NE(button, nullptr);                   // Ensure button was created
  EXPECT_EQ(button->GetLabel(), "Test Button"); // Check a property
  delete button;
}

// Another example test
TEST_F(WxExtensionsFixture, AnotherWxWidgetsTest)
{
  wxWindow* topWindow = wxTheApp->GetTopWindow();
  wxString testString = "Hello wxWidgets";
  wxStaticText* staticText = new wxStaticText(topWindow, wxID_ANY, testString);
  ASSERT_NE(staticText, nullptr);
  EXPECT_EQ(staticText->GetLabel(), testString);
  delete staticText;
}

// Test wxPlotCtrl basic functionality
TEST_F(WxExtensionsFixture, TestPlotCtrl)
{
  wxWindow* topWindow = wxTheApp->GetTopWindow();
  // Create a plot control
  wxPlotCtrl* plot = new wxPlotCtrl(topWindow, wxID_ANY);
  ASSERT_NE(plot, nullptr);

  // Test basic plot functionality - using GetDrawGrid instead of GetShowGrid
  EXPECT_TRUE(plot->GetDrawGrid());

  // Test axis labels
  plot->SetXAxisLabel("X Axis");
  plot->SetYAxisLabel("Y Axis");

  EXPECT_STREQ("X Axis", plot->GetXAxisLabel().mb_str());
  EXPECT_STREQ("Y Axis", plot->GetYAxisLabel().mb_str());

  // Test zoom functionality
  plot->SetViewRect(wxRect2DDouble(0, 0, 10, 100));
  wxRect2DDouble view = plot->GetViewRect();
  EXPECT_DOUBLE_EQ(0.0, view.m_x);
  EXPECT_DOUBLE_EQ(0.0, view.m_y);
  EXPECT_DOUBLE_EQ(10.0, view.m_width);
  EXPECT_NEAR(100.0, view.m_height, 0.5);
}

// Test wxTreeListCtrl basic functionality
TEST_F(WxExtensionsFixture, TestTreeListCtrl)
{
  wxWindow* topWindow = wxTheApp->GetTopWindow();
  // Create a tree list control with default style
  wxcode::wxTreeListCtrl* treeList = new wxcode::wxTreeListCtrl(
    topWindow, wxID_ANY, wxDefaultPosition, wxSize(400, 300));
  ASSERT_NE(treeList, nullptr);

  // Add columns
  treeList->AddColumn("Item", 200);
  treeList->AddColumn("Type", 100);
  treeList->AddColumn("Size", 100);

  // Add root item
  wxTreeItemId root = treeList->AddRoot("Root");
  wxTreeItemId folder = treeList->AppendItem(root, "Documents");
  treeList->SetItemText(folder, 1, "Folder");
  treeList->SetItemText(folder, 2, "--");

  // Add child items
  wxTreeItemId file1 = treeList->AppendItem(folder, "report.pdf");
  treeList->SetItemText(file1, 1, "PDF Document");
  treeList->SetItemText(file1, 2, "2.4 MB");

  wxTreeItemId file2 = treeList->AppendItem(folder, "budget.xlsx");
  treeList->SetItemText(file2, 1, "Excel Spreadsheet");
  treeList->SetItemText(file2, 2, "1.8 MB");

  // Test item count
  EXPECT_GE(treeList->GetCount(), 3);

  // Expand the folder and test
  treeList->Expand(folder);
  EXPECT_TRUE(treeList->IsExpanded(folder));

  // Test item selection
  treeList->SelectItem(file1);
  EXPECT_TRUE(treeList->IsSelected(file1));

  // Test item text
  EXPECT_STREQ("report.pdf", treeList->GetItemText(file1).mb_str());
  EXPECT_STREQ("PDF Document", treeList->GetItemText(file1, 1).mb_str());

  // Verify item text in the first column
  EXPECT_STREQ("report.pdf", treeList->GetItemText(file1).mb_str());
}

// Test wxThings CustomButton with bitmap and toggle states
TEST_F(WxExtensionsFixture, TestCustomButtonWithBitmap)
{
  wxWindow* topWindow = wxTheApp->GetTopWindow();
  // Create a bitmap for the button
  wxBitmap bmp(32, 32);
  wxMemoryDC dc;
  dc.SelectObject(bmp);
  dc.SetBackground(*wxBLUE_BRUSH);
  dc.Clear();
  dc.SetTextForeground(*wxWHITE);
  dc.DrawLabel("Btn", wxRect(0, 0, 32, 32), wxALIGN_CENTER);
  dc.SelectObject(wxNullBitmap);

  // Create a custom button with the bitmap
  wxCustomButton* button = new wxCustomButton(topWindow,
                                              wxID_ANY,
                                              "Click Me",
                                              wxDefaultPosition,
                                              wxDefaultSize,
                                              wxCUSTBUT_BUTTON);
  ASSERT_NE(button, nullptr);

  // Set the bitmap and test
  button->SetBitmapLabel(bmp);
  EXPECT_TRUE(button->GetBitmapLabel().IsOk());

  // Test toggle functionality
  button->SetValue(true);
  EXPECT_TRUE(button->GetValue());

  button->SetValue(false);
  EXPECT_FALSE(button->GetValue());

  // Test label change
  button->SetLabel("New Label");
  EXPECT_STREQ("New Label", button->GetLabel().mb_str());

  // Test color setting
  button->SetBackgroundColour(*wxRED);
  EXPECT_EQ(button->GetBackgroundColour(), *wxRED);
}
