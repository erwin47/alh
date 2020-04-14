#include "shaftsframe.h"

#include <wx/intl.h>
#include <wx/msgdlg.h>
#include <wx/string.h>

const long ShaftsFrame::ID_TEXTCTRL1 = wxNewId();
const long ShaftsFrame::ID_TEXTCTRL2 = wxNewId();
const long ShaftsFrame::ID_CHOICE1 = wxNewId();
const long ShaftsFrame::ID_CHOICE2 = wxNewId();
const long ShaftsFrame::ID_BUTTON1 = wxNewId();
const long ShaftsFrame::ID_STATICTEXT2 = wxNewId();
const long ShaftsFrame::ID_STATICTEXT1 = wxNewId();
const long ShaftsFrame::ID_STATICTEXT4 = wxNewId();
const long ShaftsFrame::ID_STATICTEXT3 = wxNewId();
const long ShaftsFrame::ID_STATICTEXT5 = wxNewId();
const long ShaftsFrame::ID_STATICTEXT6 = wxNewId();
const long ShaftsFrame::ID_STATICTEXT_HEX1 = wxNewId();
const long ShaftsFrame::ID_STATICTEXT_HEX2 = wxNewId();

BEGIN_EVENT_TABLE(ShaftsFrame,wxFrame)
    EVT_CLOSE  (              ShaftsFrame::OnCloseWindow)
END_EVENT_TABLE()

ShaftsFrame::ShaftsFrame(wxWindow* parent,wxWindowID id) : CAhFrame(parent, "Connect Shafts", wxDEFAULT_FRAME_STYLE)
{
	SetClientSize(wxSize(475,225));
	TextCtrlHex1 = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxPoint(56,24), wxSize(384,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrlHex2 = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxPoint(56,104), wxSize(384,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	ChoiceStructure1 = new wxChoice(this, ID_CHOICE1, wxPoint(56,64), wxSize(384,21), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	ChoiceStructure1->SetSelection( ChoiceStructure1->Append(_("(no building selected)")) );
	ChoiceStructure2 = new wxChoice(this, ID_CHOICE2, wxPoint(56,144), wxSize(384,21), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	ChoiceStructure2->SetSelection( ChoiceStructure2->Append(_("(no building selected)")) );
	ButtonConnectShaft = new wxButton(this, ID_BUTTON1, _("Connect Shaft"), wxPoint(344,168), wxSize(96,23), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Hex 1"), wxPoint(4,24), wxSize(29,16), 0, _T("ID_STATICTEXT1"));
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Hex 2"), wxPoint(4,104), wxSize(29,16), 0, _T("ID_STATICTEXT2"));
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Structure"), wxPoint(4,64), wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Structure"), wxPoint(4,144), wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Enter coordinates"), wxPoint(56,8), wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("eg: mountain (15,33)"), wxPoint(328,8), wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	StaticTextHex1 = new wxStaticText(this, ID_STATICTEXT_HEX1, wxEmptyString, wxPoint(56,48), wxSize(384,13), 0, _T("ID_STATICTEXT7"));
	StaticTextHex2 = new wxStaticText(this, ID_STATICTEXT_HEX2, wxEmptyString, wxPoint(56,128), wxSize(384,13), 0, _T("ID_STATICTEXT8"));

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ShaftsFrame::OnTextCtrlUpdate);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ShaftsFrame::OnTextCtrlUpdate);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShaftsFrame::OnButtonConnectShaftClick);
}

ShaftsFrame::~ShaftsFrame()
{
    gpApp->FrameClosing(this);
    Destroy();
}

const char * ShaftsFrame::GetConfigSection(int layout)
{
    return SZ_SECT_WND_SHAFTS;
}

void ShaftsFrame::Init(int layout, const char * szConfigSection)
{
    szConfigSection    = GetConfigSection(layout);
    CAhFrame::Init(layout, szConfigSection);
}

void ShaftsFrame::Done(BOOL SetClosedFlag)
{
    CAhFrame::Done(SetClosedFlag);
}

void ShaftsFrame::UpdateSelect(wxChoice * choice, CLand * pLand)
{
    CStruct    * pStruct;

    // Fill the given choice with the structures from the given land hex.
    choice->Clear();
    choice->Append(wxT("(No structure selected)"), (void *)-1);
    choice->SetSelection(0);

    if (pLand == NULL) return;

    for (long j=0; j<pLand->Structs.Count(); ++j)
    {
        pStruct = (CStruct*)pLand->Structs.At(j);
        choice->Append(wxString::FromUTF8(pStruct->original_description_.c_str()), (void *)j);
    }
}

void ShaftsFrame::OnTextCtrlUpdate(wxCommandEvent& event)
{
    CLand * pLandHex1 = gpApp->m_pAtlantis->GetLandFlexible(TextCtrlHex1->GetValue());
    if (gpApp->m_pAtlantis->GetLandFlexible(StaticTextHex1->GetLabel()) != pLandHex1)
    {
        if (pLandHex1)
        {
            StaticTextHex1->SetLabel(gpApp->m_pAtlantis->getFullStrLandCoord(pLandHex1));
        }
        else
        {
            StaticTextHex1->SetLabel(wxEmptyString);
        }
        UpdateSelect(ChoiceStructure1, pLandHex1);
    }
    CLand * pLandHex2 = gpApp->m_pAtlantis->GetLandFlexible(TextCtrlHex2->GetValue());
    if (gpApp->m_pAtlantis->GetLandFlexible(StaticTextHex2->GetLabel()) != pLandHex2)
    {
        if (pLandHex2)
        {
            StaticTextHex2->SetLabel(gpApp->m_pAtlantis->getFullStrLandCoord(pLandHex2));
        }
        else
        {
            StaticTextHex2->SetLabel(wxEmptyString);
        }
        UpdateSelect(ChoiceStructure2, pLandHex2);
    }
}

void ShaftsFrame::OnButtonConnectShaftClick(wxCommandEvent& event)
{
    CLand * pLandHex1 = gpApp->m_pAtlantis->GetLandFlexible(TextCtrlHex1->GetValue());
    CLand * pLandHex2 = gpApp->m_pAtlantis->GetLandFlexible(TextCtrlHex2->GetValue());
    int selectedIdx1 = ChoiceStructure1->GetSelection();
    int selectedIdx2 = ChoiceStructure2->GetSelection();

    wxString message;

    if (pLandHex1 == pLandHex2)
    {
        wxMessageBox(wxT("It is not possible to link a hex with itself. "), wxT("Linking failed"), wxOK | wxICON_ERROR);
        return;
    }

    if (pLandHex1 && pLandHex2 && selectedIdx1 > 0)
    {
        int idx = (long) (ChoiceStructure1->GetClientData(selectedIdx1));
        if (idx >= 0)
        {
            bool success = gpApp->m_pAtlantis->LinkShaft(pLandHex1, pLandHex2, idx);
            if (success) message += wxT("Shaft linked in one direction. ");
        }
    }

    if (pLandHex1 && pLandHex2 && selectedIdx2 > 0)
    {
        int idx = (long) (ChoiceStructure2->GetClientData(selectedIdx2));
        if (idx >= 0)
        {
            // Link the way back, from the second hex to the first.
            bool success = gpApp->m_pAtlantis->LinkShaft(pLandHex2, pLandHex1, idx);
            if (success && !message.IsEmpty())
            {
                message = wxT("The shaft has been linked in both directions. ");
            }
            else if (success) message = wxT("Shaft linked in one direction. ");
        }
    }

    if (message.IsEmpty())
    {
        wxMessageBox(wxT("Nothing linked"), wxT("Linking failed"), wxOK | wxICON_QUESTION);
    }
    else
    {
        wxMessageBox(message, wxT("Shaft linked"), wxOK | wxICON_INFORMATION);
    }
}
