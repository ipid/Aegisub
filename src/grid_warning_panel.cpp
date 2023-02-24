#include "grid_warning_panel.h"
#include <wx/generic/statbmpg.h>

/* XPM */
static const char* const warning_red_16[] = {
    /* columns rows colors chars-per-pixel */
    "16 16 32 1 ",
    "  c None",
    ". c #D83A3A",
    "X c #E34444",
    "o c #EA4949",
    "O c #FB5858",
    "+ c #FD5959",
    "@ c #FF5B5B",
    "# c #FF5D5D",
    "$ c #FE5E5E",
    "% c #FF6161",
    "& c #FF6464",
    "* c #FB6A6A",
    "= c #FA8181",
    "- c #FE8181",
    "; c #FE8787",
    ": c #F78989",
    "> c #F88A8A",
    ", c #FE8F8F",
    "< c #F89494",
    "1 c #F89595",
    "2 c #F99C9C",
    "3 c #F99E9E",
    "4 c #FFB4B4",
    "5 c #FBBBBB",
    "6 c #FEBABA",
    "7 c #FFBABA",
    "8 c #FFC5C5",
    "9 c #FCCACA",
    "0 c #FFCCCC",
    "q c #FFD3D3",
    "w c #FFD6D6",
    "e c white",
    /* pixels */
    "       .o       ",
    "      .X@@      ",
    "      .O@@      ",
    "     .X@@@@     ",
    "     .O=97@@    ",
    "    .X@3ew@@    ",
    "    .O@3eq@@@   ",
    "   .X@@1e0@@@   ",
    "   .O@@1e8@@@@  ",
    "  .X@@@>e7@@@@  ",
    "  .O@@@$;&@@@@@ ",
    " .X@@@@*5,@@@@@@",
    " .O@@@@:e4@@@@@@",
    ".X@@@@@$-%@@@@@@",
    ".O@@@@@@@@@@@   ",
    " @@@@@          "
};


GridWarningPanel::GridWarningPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE) {

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxGenericStaticBitmap* icon = new wxGenericStaticBitmap(this, wxID_ANY, wxIcon(warning_red_16));
    icon->SetScaleMode(wxGenericStaticBitmap::Scale_AspectFit);
    icon->SetSizeHints(wxSize(16, 16), wxSize(16, 16));

    wxStaticText* warningPrompt = new wxStaticText(this, wxID_ANY, L"警告：");

    wxFont warningPromptFont = warningPrompt->GetFont();
    warningPromptFont.SetWeight(wxFONTWEIGHT_BOLD);
    warningPrompt->SetFont(warningPromptFont);

    warningPrompt->SetForegroundColour(0x3A3AD8);

    mWarningText = new wxStaticText(this, wxID_ANY, L"暂无警告");

    constexpr int BORDER = 4;
    sizer->Add(icon, 0, wxALL, BORDER);
    sizer->Add(warningPrompt, 0, wxLEFT | wxUP | wxDOWN, BORDER);
    sizer->Add(mWarningText, 1, wxEXPAND | wxRIGHT | wxUP | wxDOWN, BORDER);

    SetSizer(sizer);
}
