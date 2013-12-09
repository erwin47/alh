#ifndef SHAFTSFRAME_H
#define SHAFTSFRAME_H

//(*Headers(ShaftsFrame)
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/frame.h>
//*)

#include "cstr.h"
#include "collection.h"
#include "cfgfile.h"
#include "files.h"
#include "atlaparser.h"
#include "consts.h"
#include "consts_ah.h"
#include "hash.h"

#include "ahapp.h"
#include "ahframe.h"

class ShaftsFrame: public CAhFrame
{
	public:

		ShaftsFrame(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~ShaftsFrame();

        wxString strHex1, strHex2;
        void UpdateSelect(wxChoice * choice, CLand * pLand); // Fills the select with the structures from the land

        virtual void    Init(int layout, const char * szConfigSection);
        virtual void    Done(BOOL SetClosedFlag);

        static const char * GetConfigSection(int layout);

		//(*Declarations(ShaftsFrame)
		wxButton* ButtonConnectShaft;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticTextHex1;
		wxStaticText* StaticTextHex2;
		wxChoice* ChoiceStructure1;
		wxChoice* ChoiceStructure2;
		wxTextCtrl* TextCtrlHex1;
		wxTextCtrl* TextCtrlHex2;
		//*)

	protected:

		//(*Identifiers(ShaftsFrame)
		static const long ID_TEXTCTRL1;
		static const long ID_TEXTCTRL2;
		static const long ID_CHOICE1;
		static const long ID_CHOICE2;
		static const long ID_BUTTON1;
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_STATICTEXT3;
		static const long ID_STATICTEXT4;
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT6;
		static const long ID_STATICTEXT_HEX1;
		static const long ID_STATICTEXT_HEX2;
		//*)

	private:

		//(*Handlers(ShaftsFrame)
		void OnButtonConnectShaftClick(wxCommandEvent& event);
		void OnChoiceStructure1Activate(wxCommandEvent& event);
		void OnTextCtrlUpdate(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
