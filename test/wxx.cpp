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

// Test fixture for wxWidgets extension libraries
class WxExtensionsTest : public ::testing::Test
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

  void SetUp() override
  {
    // Create a test frame and panel
    m_frame = new wxFrame(nullptr, wxID_ANY, "Test Frame");
    m_panel = new wxPanel(m_frame);
  }

  void TearDown() override
  {
    if (m_frame)
    {
      m_frame->Destroy();
      m_frame = nullptr;
    }
  }

  wxFrame* m_frame{nullptr};
  wxPanel* m_panel{nullptr};
};

// Test wxPlotCtrl basic functionality
TEST_F(WxExtensionsTest, TestPlotCtrl)
{
  // Create a plot control
  wxPlotCtrl* plot = new wxPlotCtrl(m_panel, wxID_ANY);
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
  EXPECT_DOUBLE_EQ(100.0, view.m_height);
}

// Test wxTreeListCtrl basic functionality
TEST_F(WxExtensionsTest, TestTreeListCtrl)
{
  // Create a tree list control with default style
  wxcode::wxTreeListCtrl* treeList = new wxcode::wxTreeListCtrl(
    m_panel, wxID_ANY, wxDefaultPosition, wxSize(400, 300));
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
TEST_F(WxExtensionsTest, TestCustomButtonWithBitmap)
{
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
  wxCustomButton* button = new wxCustomButton(m_panel,
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
