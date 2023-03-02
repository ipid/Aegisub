#pragma once

#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/font.h>

class GridWarningPanel : public wxPanel {
public:
    GridWarningPanel(wxWindow* parent);
    void UpdateWarningStatus(int collisionCount);

private:
    wxStaticText* mWarningText;
};
