/*
 * This source file is part of the Atlantis Little Helper program.
 * Copyright (C) 2001 Maxim Shariy.
 *
 * Atlantis Little Helper is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atlantis Little Helper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atlantis Little Helper; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "stdhdr.h"

#include "wx/splitter.h"
#include "wx/listctrl.h"
#include <sstream>
#include <algorithm>
//#include "wx/resource.h"

#include "files.h"
#include "consts.h"
#include "consts_ah.h"
#include "objs.h"
#include "hash.h"

#include "ahapp.h"
#include "ahframe.h"
#include "mapframe.h"
#include "unitframe.h"
#include "unitframefltr.h"
#include "msgframe.h"
#include "editsframe.h"
#include "editpane.h"
#include "mappane.h"
#include "listpane.h"
#include "shaftsframe.h"
#include "unitpane.h"
#include "utildlgs.h"
#include "unitfilterdlg.h"
#include "unitpanefltr.h"
#include "listcoledit.h"
#include "optionsdlg.h"
#include "flagsdlg.h"
#include "data_control.h"
#include "ah_control.h"

#ifdef __WXMAC_OSX__
#include <unistd.h>
#include <sys/stat.h>
#include <sys/param.h>
#endif

CAhApp * gpApp = NULL; // Our own the one and only pointer

IMPLEMENT_APP(CAhApp);

static CGameDataHelper ThisGameDataHelper;

//=========================================================================
#ifndef _WIN32
#include <execinfo.h>
#include <unistd.h>

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}
#endif


CAhApp::CAhApp() : m_HexDescrSrc    (128),
                   m_UnitDescrSrc   (128),
                   m_ItemWeights    ( 32),
                   m_OrderHash      (  3),
                   m_TradeItemsHash (  2),
                   m_MenHash        (  2),
                   m_MaxSkillHash   (  6),
                   m_MagicSkillsHash(  6)
{
#ifndef _WIN32
    signal(SIGSEGV, handler);   // install our handler  
#endif
    m_FirstLoad         = TRUE;
    orders_changed_flag_  = false;
    m_CommentsChanged   = FALSE;
    m_UpgradeLandFlags  = FALSE;
    m_DiscardChanges    = FALSE;
    //m_SelUnitIdx        = -1;
    m_layout            = 0;
    m_DisableErrs       = FALSE;
    m_pAccel            = NULL;

    memset(m_Frames, 0, sizeof(m_Frames));
    memset(m_Panes , 0, sizeof(m_Panes ));
    memset(m_Fonts , 0, sizeof(m_Fonts ));

    m_FontDescr[FONT_EDIT_DESCR]  = "Descriptions";
    m_FontDescr[FONT_EDIT_ORDER]  = "Orders & comments";
    m_FontDescr[FONT_MAP_COORD ]  = "Map coordinates";
    m_FontDescr[FONT_MAP_TEXT  ]  = "Map text";
    m_FontDescr[FONT_UNIT_LIST ]  = "Unit list";
    m_FontDescr[FONT_EDIT_HDR  ]  = "Edit pane header";
    m_FontDescr[FONT_VIEW_DLG  ]  = "View dialogs";
    m_FontDescr[FONT_ERR_DLG   ]  = "Messages and Errors";




}

CAhApp::~CAhApp()
{
}

//-------------------------------------------------------------------------
int CAhApp::getLayout() const
{
    return m_layout;
}


bool CAhApp::OnInit()
{
    int               i;
    const char      * p;
    const char      * szName;
    const char      * szValue;
    CStrStr         * pSS;
    CStr              S(32), S2;
    int               sectidx;
    CStrStrColl2      Coll;


    gpApp = this;

    state_sections_.insert(SZ_SECT_DEF_ORDERS       );
    state_sections_.insert(SZ_SECT_ORDERS           );
    state_sections_.insert(SZ_SECT_REPORTS          );
    state_sections_.insert(SZ_SECT_LAND_FLAGS       );
    state_sections_.insert(SZ_SECT_LAND_VISITED     );
    state_sections_.insert(SZ_SECT_SKILLS           );
    state_sections_.insert(SZ_SECT_ITEMS            );
    state_sections_.insert(SZ_SECT_OBJECTS          );
    state_sections_.insert(SZ_SECT_PASSWORDS        );
    state_sections_.insert(SZ_SECT_UNIT_TRACKING    );
    state_sections_.insert(SZ_SECT_FOLDERS          );
    state_sections_.insert(SZ_SECT_DO_NOT_SHOW_THESE);
    state_sections_.insert(SZ_SECT_TROPIC_ZONE      );
    state_sections_.insert(SZ_SECT_PLANE_SIZE       );
    state_sections_.insert(SZ_SECT_UNIT_FLAGS       );


    config_[CONFIG_FILE_CONFIG].load(SZ_CONFIG_FILE);
/* test of config
    config_[CONFIG_FILE_CONFIG].save("test.txt");
    config::Config test_cfg;
    test_cfg.load("test.txt");
    if (test_cfg == config_[CONFIG_FILE_CONFIG] && config_[CONFIG_FILE_CONFIG] == test_cfg) 
    {
      int i = 5;
    }
*/
    config_[CONFIG_FILE_STATE].load(SZ_CONFIG_STATE_FILE);

    //UpgradeConfigFiles();

    m_layout = atol(GetConfig(SZ_SECT_COMMON, SZ_KEY_LAYOUT));
    if (m_layout<0)
        m_layout = 0;
    if (m_layout>=AH_LAYOUT_COUNT)
        m_layout = AH_LAYOUT_COUNT-1;

    m_Brightness_Delta = atol(GetConfig(SZ_SECT_COMMON, SZ_KEY_BRIGHT_DELTA));


    for (i=0; i<FONT_COUNT; i++)
    {
        S.Empty();
        S << (long)i;
        szValue = GetConfig(SZ_SECT_FONTS_2, S.GetData());
        m_Fonts[i] = NewFontFromStr(szValue);
    }

    if (0==stricmp(SZ_EOL_MS, GetConfig(SZ_SECT_COMMON, SZ_KEY_EOL)))
        EOL_FILE = EOL_MS;
    else
        EOL_FILE = EOL_UNIX;


    // Load unit property groups
    m_UnitPropertyGroups.m_bDuplicates=TRUE;
    sectidx = GetSectionFirst(SZ_SECT_UNITPROP_GROUPS, szName, szValue);
    while (sectidx >= 0)
    {
        while (szValue && *szValue)
        {
            szValue = S.GetToken(szValue, ',');
            pSS     = new CStrStr(szName, S.GetData());
            if (Coll.Insert(pSS))
                m_UnitPropertyGroups.Insert(pSS);
            else
                delete pSS;
        }
        sectidx = GetSectionNext(sectidx, SZ_SECT_UNITPROP_GROUPS, szName, szValue);
    }
    CUnit::m_PropertyGroupsColl = &m_UnitPropertyGroups;


    // Property group name must not be an alias!
    szName = "";
    for (i=0; i<Coll.Count(); i++)
    {
        pSS = (CStrStr*)Coll.At(i);
        if (0!=stricmp(szName, pSS->m_key))
        {
            szName = pSS->m_key;
            p = ResolveAlias(szName);
            if (0!=stricmp(szName, p))
            {
                S = "Group name \"";
                S << szName << "\" can be resolved as alias for \"" << p << "\"!\r\n";
                ShowError(S.GetData(), S.GetLength(), TRUE);
            }
        }
    }
    Coll.DeleteAll();


    InitMoveModes();

    //m_Attitudes.FreeAll();
    SetAttitudeForFaction(0, ATT_NEUTRAL);

    // Load order hash
    m_OrderHash.Insert("advance"    ,     (void*)O_ADVANCE    );
    m_OrderHash.Insert("assassinate",     (void*)O_ASSASSINATE);
    m_OrderHash.Insert("attack"     ,     (void*)O_ATTACK     );
    m_OrderHash.Insert("autotax"    ,     (void*)O_AUTOTAX    );
    m_OrderHash.Insert("build"      ,     (void*)O_BUILD      );
    m_OrderHash.Insert("buy"        ,     (void*)O_BUY        );
    m_OrderHash.Insert("claim"      ,     (void*)O_CLAIM      );
    m_OrderHash.Insert("end"        ,     (void*)O_ENDFORM    );
    m_OrderHash.Insert("endturn"    ,     (void*)O_ENDTURN    );
    m_OrderHash.Insert("enter"      ,     (void*)O_ENTER      );
    m_OrderHash.Insert("form"       ,     (void*)O_FORM       );
    m_OrderHash.Insert("give"       ,     (void*)O_GIVE       );
    m_OrderHash.Insert("giveif"     ,     (void*)O_GIVEIF     );
    m_OrderHash.Insert("take"       ,     (void*)O_TAKE       );
    m_OrderHash.Insert("send"       ,     (void*)O_SEND       );
    m_OrderHash.Insert("withdraw"   ,     (void*)O_WITHDRAW   );
    m_OrderHash.Insert("leave"      ,     (void*)O_LEAVE      );
    m_OrderHash.Insert("move"       ,     (void*)O_MOVE       );
    m_OrderHash.Insert("produce"    ,     (void*)O_PRODUCE    );
    m_OrderHash.Insert("promote"    ,     (void*)O_PROMOTE    );
    m_OrderHash.Insert("sail"       ,     (void*)O_SAIL       );
    m_OrderHash.Insert("sell"       ,     (void*)O_SELL       );
    m_OrderHash.Insert("steal"      ,     (void*)O_STEAL      );
    m_OrderHash.Insert("study"      ,     (void*)O_STUDY      );
    m_OrderHash.Insert("teach"      ,     (void*)O_TEACH      );
    m_OrderHash.Insert("transport"  ,     (void*)O_TRANSPORT  );
    m_OrderHash.Insert("turn"       ,     (void*)O_TURN       );

    m_OrderHash.Insert("pillage"    ,     (void*)O_PILLAGE    );
    m_OrderHash.Insert("tax"        ,     (void*)O_TAX        );
    m_OrderHash.Insert("entertain"  ,     (void*)O_ENTERTAIN  );
    m_OrderHash.Insert("work"       ,     (void*)O_WORK       );

    m_OrderHash.Insert("guard"      ,     (void*)O_GUARD      );
    m_OrderHash.Insert("avoid"      ,     (void*)O_AVOID      );
    m_OrderHash.Insert("behind"     ,     (void*)O_BEHIND     );
    m_OrderHash.Insert("reveal"     ,     (void*)O_REVEAL     );
    m_OrderHash.Insert("hold"       ,     (void*)O_HOLD       );
    m_OrderHash.Insert("noaid"      ,     (void*)O_NOAID      );
    m_OrderHash.Insert("consume"    ,     (void*)O_CONSUME    );
    m_OrderHash.Insert("nocross"    ,     (void*)O_NOCROSS    );
    m_OrderHash.Insert("spoils"     ,     (void*)O_SPOILS     );

    m_OrderHash.Insert("recruit"    ,     (void*)O_RECRUIT    );
    m_OrderHash.Insert("share"      ,     (void*)O_SHARE      );

    m_OrderHash.Insert("template"   ,     (void*)O_TEMPLATE   );
    m_OrderHash.Insert("endtemplate",     (void*)O_ENDTEMPLATE);
    m_OrderHash.Insert("all"        ,     (void*)O_ALL        );
    m_OrderHash.Insert("endall"     ,     (void*)O_ENDALL     );

    m_OrderHash.Insert("type"       ,     (void*)O_TYPE       );
    m_OrderHash.Insert("label"      ,     (void*)O_LABEL      );
    m_OrderHash.Insert("name"       ,     (void*)O_NAME       );






    p = SkipSpaces(GetConfig(SZ_SECT_COMMON, SZ_KEY_VALID_ORDERS));
    while (p && *p)
    {
        const void * data;
        p = SkipSpaces(S.GetToken(p, ','));
        if (!S.IsEmpty() && !m_OrderHash.Locate(S.GetData(), data))
            m_OrderHash.Insert(S.GetData(), (void*)-1);
    }
//    m_OrderHash.Dbg_Print();

    // Load trade items hash
    p = SkipSpaces(GetConfig(SZ_SECT_UNITPROP_GROUPS,  PRP_TRADE_ITEMS));
    while (p && *p)
    {
        const void * data;
        p = SkipSpaces(S.GetToken(p, ','));
        if (!S.IsEmpty() && !m_TradeItemsHash.Locate(S.GetData(), data))
            m_TradeItemsHash.Insert(S.GetData(), (void*)-1);
    }

    // All the men hash
    p = SkipSpaces(GetConfig(SZ_SECT_UNITPROP_GROUPS,  PRP_MEN));
    while (p && *p)
    {
        const void * data;
        p = SkipSpaces(S.GetToken(p, ','));
        if (!S.IsEmpty() && !m_MenHash.Locate(S.GetData(), data))
            m_MenHash.Insert(S.GetData(), (void*)-1);
    }

    // Magic skills hash
    p = SkipSpaces(GetConfig(SZ_SECT_UNITPROP_GROUPS,  PRP_MAG_SKILLS));
    while (p && *p)
    {
        const void * data;
        int x;
        p = SkipSpaces(S.GetToken(p, ','));
        x = S.FindSubStrR(PRP_SKILL_POSTFIX);
        if (x>=0)
            S.DelSubStr(x, S.GetLength()-x+1);

        if (!S.IsEmpty() && !m_MagicSkillsHash.Locate(S.GetData(), data))
            m_MagicSkillsHash.Insert(S.GetData(), (void*)-1);
    }

    m_pAtlantis = new CAtlaParser(&ThisGameDataHelper);
    m_Reports.Insert(m_pAtlantis);

    // Read list of year/month for report
    i = GetSectionFirst(SZ_SECT_REPORTS, szName, szValue);
    while (i>=0)
    {
        m_ReportDates.Insert((void*)atol(szName));
        i = GetSectionNext (i, SZ_SECT_REPORTS, szName, szValue);
    }



    LoadTerrainCostConfig();

    StdRedirectInit();

    CreateAccelerator();

    OpenMapFrame();

    if ((AH_LAYOUT_3_WIN==m_layout || AH_LAYOUT_2_WIN==m_layout) &&
        atol(GetConfig(CUnitFrame::GetConfigSection(m_layout), SZ_KEY_OPEN)) )
        OpenUnitFrame();

    if ((AH_LAYOUT_3_WIN==m_layout) &&
        (atol(GetConfig(CEditsFrame::GetConfigSection(m_layout), SZ_KEY_OPEN))) )
        OpenEditsFrame();

    SetTopWindow(m_Frames[AH_FRAME_MAP]);
    m_Frames[AH_FRAME_MAP]->SetFocus();


    if (argc>1)
        for (i=1; i<argc; i++)
            LoadReport(wxString(argv[i]).mb_str(), i>1);
    else
        if (atol(GetConfig(SZ_SECT_COMMON, SZ_KEY_LOAD_REP)) && (m_ReportDates.Count() > 0) )
        {
            S.Empty();
            S << (long)m_ReportDates.At(m_ReportDates.Count()-1);
            S2 = GetConfig(SZ_SECT_REPORTS, S.GetData());
            const char * p = S2.GetData();
            BOOL         join = FALSE;
            while (p && *p)
            {
                p = S.GetToken(p, ',');
                LoadReport(S.GetData(), join);
                join = TRUE;
            }
        }

    if (atol(GetConfig(CUnitFrameFltr::GetConfigSection(m_layout), SZ_KEY_OPEN)) )
        OpenUnitFrameFltr(FALSE);

    return TRUE;
}

//-------------------------------------------------------------------------

int CAhApp::OnExit()
{
    int  i;
    CStr S;
    CStr Name;

    CUnit::ResetCustomFlagNames();

    for (i=0; i<FONT_COUNT; i++)
    {
        FontToStr(m_Fonts[i], S);
        Name.Empty();
        Name << (long)i;
        SetConfig(SZ_SECT_FONTS_2, Name.GetData(), S.GetData());
    }

    if (!m_DiscardChanges)
    {
        config_[CONFIG_FILE_CONFIG].save(SZ_CONFIG_FILE);
        config_[CONFIG_FILE_STATE].save(SZ_CONFIG_STATE_FILE);

        if (ERR_OK==m_pAtlantis->m_ParseErr)
            SaveHistory(SZ_HISTORY_FILE);
    }

    m_TradeItemsHash.FreeAll();
    m_MenHash.FreeAll();
    m_MaxSkillHash.FreeAll();
    m_MagicSkillsHash.FreeAll();

    m_Reports.FreeAll();

    for (i=0; i<FONT_COUNT; i++)
        delete m_Fonts[i];


    if (m_pAccel)
        delete m_pAccel;

    m_MoveModes.FreeAll();
    m_ItemWeights.FreeAll();
    m_OrderHash.FreeAll();
    m_Attitudes.FreeAll();

    StdRedirectDone();

    gpApp = NULL;
    return 0;
}

//-------------------------------------------------------------------------

void CAhApp::CreateAccelerator()
{
    static wxAcceleratorEntry entries[13];
    entries[0].Set(wxACCEL_CTRL,  (int)'S',     menu_SaveOrders);
    entries[1].Set(wxACCEL_CTRL,  (int)'N',     accel_NextUnit );
    entries[2].Set(wxACCEL_CTRL,  (int)'P',     accel_PrevUnit );
    entries[3].Set(wxACCEL_CTRL,  (int)'U',     accel_UnitList );
    entries[4].Set(wxACCEL_CTRL,  (int)'O',     accel_Orders   );
    entries[5].Set(wxACCEL_CTRL,  (int)'F',     accel_CreateNewUnit );
    entries[6].Set(wxACCEL_CTRL,  (int)'R',     accel_ReceiveOrder );
    entries[7].Set(wxACCEL_CTRL,  (int)'G',     accel_FilterByItem );
    entries[8].Set(wxACCEL_CTRL,  (int)'E',     accel_ShowLandEconomy );
    entries[9].Set(wxACCEL_CTRL,  (int)'W',     accel_ShowLandWarehouse );
    entries[10].Set(wxACCEL_ALT,  (int)'W',     accel_ShowLandEnemyWarehouse );
    entries[11].Set(wxACCEL_CTRL,  (int)'A',     accel_ShowLandAutoOrders );
    entries[12].Set(wxACCEL_CTRL, (int)'M',     accel_ShowLandMovePhases );

    m_pAccel = new wxAcceleratorTable(13, entries);
}

//-------------------------------------------------------------------------

void CAhApp::Redraw()
{
    int i;

    for (i=0; i<AH_PANE_COUNT; i++)
        if (m_Panes[i])
            m_Panes[i]->Refresh(FALSE);
}


//-------------------------------------------------------------------------

void CAhApp::ApplyFonts()
{

    if (m_Panes[AH_PANE_MAP          ]) ((CMapPane *)m_Panes[AH_PANE_MAP          ])->ApplyFonts();
    if (m_Panes[AH_PANE_MAP_DESCR    ]) ((CEditPane*)m_Panes[AH_PANE_MAP_DESCR    ])->ApplyFonts();
    if (m_Panes[AH_PANE_UNITS_HEX    ]) ((CUnitPane*)m_Panes[AH_PANE_UNITS_HEX    ])->ApplyFonts();
    if (m_Panes[AH_PANE_UNITS_FILTER ]) ((CUnitPane*)m_Panes[AH_PANE_UNITS_FILTER ])->ApplyFonts();
    if (m_Panes[AH_PANE_UNIT_DESCR   ]) ((CEditPane*)m_Panes[AH_PANE_UNIT_DESCR   ])->ApplyFonts();
    if (m_Panes[AH_PANE_UNIT_COMMANDS]) ((CUnitOrderEditPane*)m_Panes[AH_PANE_UNIT_COMMANDS])->ApplyFonts();
    if (m_Panes[AH_PANE_UNIT_COMMENTS]) ((CEditPane*)m_Panes[AH_PANE_UNIT_COMMENTS])->ApplyFonts();
    if (m_Panes[AH_PANE_MSG          ]) ((CEditPane*)m_Panes[AH_PANE_MSG          ])->ApplyFonts();

}

//-------------------------------------------------------------------------

void CAhApp::ApplyColors()
{
    if (m_Panes[AH_PANE_MAP          ]) ((CMapPane *)m_Panes[AH_PANE_MAP          ])->ApplyColors();
}

//-------------------------------------------------------------------------

void CAhApp::ApplyIcons()
{
    if (m_Panes[AH_PANE_MAP          ]) ((CMapPane *)m_Panes[AH_PANE_MAP          ])->ApplyIcons();
}

//-------------------------------------------------------------------------

void CAhApp::OpenOptionsDlg()
{
    int rc;

    COptionsDialog *dialog = new COptionsDialog(m_Frames[AH_FRAME_MAP]);
    {
        dialog->Init();
        rc = dialog->ShowModal();
        if (wxID_OK==rc)
        {
        }
        dialog->Done();
    }
    //dialog->Close(TRUE);
}

//-------------------------------------------------------------------------

void CAhApp::OpenMapFrame()
{
    if (!m_Frames[AH_FRAME_MAP])
    {
        m_Frames[AH_FRAME_MAP] = new CMapFrame(NULL, m_layout);
        m_Frames[AH_FRAME_MAP]->Init(m_layout, NULL);
        m_Frames[AH_FRAME_MAP]->Show(TRUE);
    }
    else
        m_Frames[AH_FRAME_MAP]->Raise();
}

//-------------------------------------------------------------------------

void CAhApp::OpenUnitFrame()
{
    if (!m_Frames[AH_FRAME_UNITS])
    {
        m_Frames[AH_FRAME_UNITS] = new CUnitFrame(m_Frames[AH_FRAME_MAP]);
        m_Frames[AH_FRAME_UNITS]->Init(m_layout, NULL);
        m_Frames[AH_FRAME_UNITS]->Show(TRUE);
    }
    else
        m_Frames[AH_FRAME_UNITS]->Raise();
}

//--------------------------------------------------------------------------

void CAhApp::ShowShaftConnectGUI()
{
    if (!m_Frames[AH_FRAME_SHAFTS])
    {
        m_Frames[AH_FRAME_SHAFTS] = new ShaftsFrame(m_Frames[AH_FRAME_MAP]);
        m_Frames[AH_FRAME_SHAFTS]->Init(m_layout, NULL);
        m_Frames[AH_FRAME_SHAFTS]->Show(TRUE);
    }
    else
        m_Frames[AH_FRAME_SHAFTS]->Raise();
}

//-------------------------------------------------------------------------

void CAhApp::OpenUnitFrameFltr(BOOL PopUpSettings)
{
    if (!m_Frames[AH_FRAME_UNITS_FLTR])
    {
        m_Frames[AH_FRAME_UNITS_FLTR] = new CUnitFrameFltr(m_Frames[AH_FRAME_MAP]);
        m_Frames[AH_FRAME_UNITS_FLTR]->Init(m_layout, NULL);
        m_Frames[AH_FRAME_UNITS_FLTR]->Show(TRUE);

        CUnitPaneFltr   * pUnitPaneF = (CUnitPaneFltr*)m_Panes [AH_PANE_UNITS_FILTER];
        wxCommandEvent    event;

        if (pUnitPaneF) 
        {
            if  (PopUpSettings)
                pUnitPaneF->OnPopupMenuFilter(event);
            else
                pUnitPaneF->Update(NULL);
        }
    }
    else
        m_Frames[AH_FRAME_UNITS_FLTR]->Raise();


}

//-------------------------------------------------------------------------

void CAhApp::OpenMsgFrame()
{
    if (!m_Frames[AH_FRAME_MSG])
    {
        m_Frames[AH_FRAME_MSG] = new CMsgFrame(m_Frames[AH_FRAME_MAP]);
        m_Frames[AH_FRAME_MSG]->Init(m_layout, NULL);
        m_MsgSrc.Empty();
        m_Frames[AH_FRAME_MSG]->Show(TRUE);
    }
}

//-------------------------------------------------------------------------

void CAhApp::OpenEditsFrame()
{
    if (!m_Frames[AH_FRAME_EDITS])
    {
        m_Frames[AH_FRAME_EDITS] = new CEditsFrame(m_Frames[AH_FRAME_MAP]);
        m_Frames[AH_FRAME_EDITS]->Init(m_layout, NULL);
        m_Frames[AH_FRAME_EDITS]->Show(TRUE);
    }
    else
        m_Frames[AH_FRAME_EDITS]->Raise();
}

//-------------------------------------------------------------------------
/*
void CAhApp::UpgradeConfigFiles()
{
    CStr         Section;
    const char * szNextSection;
    const char * szName;
    const char * szValue;
    BOOL         Ok = TRUE;
    int          fileno, idx, i;
    CStr         ConfigKey;

    // move the sections
    Ok = m_Config[CONFIG_FILE_CONFIG].GetNextSection("", szNextSection);
    while (Ok)
    {
        Section = szNextSection;
        fileno    = GetConfigFileNo(Section.GetData());

        if (CONFIG_FILE_CONFIG != fileno)
        {
            // move to the appropriate file
            idx = m_Config[CONFIG_FILE_CONFIG].GetFirstInSection(Section.GetData(), szName, szValue);
            while (idx>=0)
            {
                m_Config[fileno].SetByName(Section.GetData(), szName, szValue);
                idx = m_Config[CONFIG_FILE_CONFIG].GetNextInSection(idx, Section.GetData(), szName, szValue);
            }
            m_Config[CONFIG_FILE_CONFIG].RemoveSection(Section.GetData());

            // and it means land flags has to be moved, too
            m_UpgradeLandFlags = TRUE;
        }

        Ok = m_Config[CONFIG_FILE_CONFIG].GetNextSection(Section.GetData(), szNextSection);
    }

    // unit lists columns
    szValue = m_Config[CONFIG_FILE_CONFIG].GetByName(SZ_SECT_LIST_COL_CURRENT, SZ_KEY_LIS_COL_UNITS_HEX);
    if (!szValue || !*szValue)
    {
        MoveSectionEntries(CONFIG_FILE_CONFIG, SZ_SECT_UNITLIST_HDR     , SZ_SECT_LIST_COL_UNIT_DEF     );
        MoveSectionEntries(CONFIG_FILE_CONFIG, SZ_SECT_UNITLIST_HDR_FLTR, SZ_SECT_LIST_COL_UNIT_FLTR_DEF);

        SetConfig(SZ_SECT_LIST_COL_CURRENT  , SZ_KEY_LIS_COL_UNITS_HEX  ,     SZ_SECT_LIST_COL_UNIT_DEF);
        SetConfig(SZ_SECT_LIST_COL_CURRENT  , SZ_KEY_LIS_COL_UNITS_FILTER,    SZ_SECT_LIST_COL_UNIT_FLTR_DEF);
    }

    // unit filter
    Section.Empty();
    Section  << SZ_SECT_UNIT_FILTER << "Default";
    Ok = FALSE;
    for (i=0; i<UNIT_SIMPLE_FLTR_COUNT; i++)
    {
        ConfigKey.Format("%s%d", SZ_KEY_UNIT_FLTR_PROPERTY, i);
        szValue = SkipSpaces(gpApp->GetConfig(SZ_SECT_WND_UNITS_FLTR, ConfigKey.GetData()));
        if (szValue && *szValue)
        {
            gpApp->SetConfig(Section.GetData(),      ConfigKey.GetData(), szValue);
            gpApp->SetConfig(SZ_SECT_WND_UNITS_FLTR, ConfigKey.GetData(), "");
            Ok = TRUE;
        }

        ConfigKey.Format("%s%d", SZ_KEY_UNIT_FLTR_COMPARE , i);
        szValue = gpApp->GetConfig(SZ_SECT_WND_UNITS_FLTR, ConfigKey.GetData());
        if (szValue && *szValue)
        {
            gpApp->SetConfig(Section.GetData(),      ConfigKey.GetData(), szValue);
            gpApp->SetConfig(SZ_SECT_WND_UNITS_FLTR, ConfigKey.GetData(), "");
            Ok = TRUE;
        }

        ConfigKey.Format("%s%d", SZ_KEY_UNIT_FLTR_VALUE   , i);
        szValue = gpApp->GetConfig(SZ_SECT_WND_UNITS_FLTR, ConfigKey.GetData());
        if (szValue && *szValue)
        {
            gpApp->SetConfig(Section.GetData(),      ConfigKey.GetData(), szValue);
            gpApp->SetConfig(SZ_SECT_WND_UNITS_FLTR, ConfigKey.GetData(), "");
            Ok = TRUE;
        }
    }
    if (Ok)
        gpApp->SetConfig(SZ_SECT_WND_UNITS_FLTR, SZ_KEY_FLTR_SET, Section.GetData());

    // Arcadia III roads
    szValue = m_Config[CONFIG_FILE_CONFIG].GetByName(SZ_SECT_COLORS,  SZ_KEY_MAP_ROAD_OLD);
    if (szValue && *szValue)
    {
        m_Config[CONFIG_FILE_CONFIG].SetByName(SZ_SECT_COLORS, SZ_KEY_MAP_ROAD    , szValue);
        m_Config[CONFIG_FILE_CONFIG].SetByName(SZ_SECT_COLORS, SZ_KEY_MAP_ROAD_OLD, "");
    }
    szValue = m_Config[CONFIG_FILE_CONFIG].GetByName(SZ_SECT_COLORS,  SZ_KEY_MAP_ROAD_BAD_OLD);
    if (szValue && *szValue)
    {
        m_Config[CONFIG_FILE_CONFIG].SetByName(SZ_SECT_COLORS, SZ_KEY_MAP_ROAD_BAD    , szValue);
        m_Config[CONFIG_FILE_CONFIG].SetByName(SZ_SECT_COLORS, SZ_KEY_MAP_ROAD_BAD_OLD, "");
    }
}
*/
//-------------------------------------------------------------------------
/*
void CAhApp::MoveSectionEntries(int fileno, const char * src, const char * dest)
{
    const char * szName;
    const char * szValue;
    CBufColl     Names, Values;
    int          idx;

    idx = m_Config[fileno].GetFirstInSection(src, szName, szValue);
    while (idx>=0)
    {
        Names.Insert(strdup(szName));
        Values.Insert(strdup(szValue));
        idx = m_Config[fileno].GetNextInSection(idx, src, szName, szValue);
    }
    m_Config[fileno].RemoveSection(src);

    for (idx=0; idx<Values.Count(); idx++)
    {
        szName = (const char *)Names.At(idx);
        szValue= (const char *)Values.At(idx);
        m_Config[fileno].SetByName(dest, szName?szName:"", szValue?szValue:"");
    }

    Names.FreeAll();
    Values.FreeAll();
}*/

//-------------------------------------------------------------------------
/*
void CAhApp::UpgradeConfigByFactionId()
{
    int          fileno, idx;
    CStr         S, Section, Key;
    const char * szName;
    const char * szValue;

    if (m_pAtlantis->m_CrntFactionId > 0)
    {
        // Upgrade order files
        ComposeConfigOrdersSection(Section, m_pAtlantis->m_CrntFactionId);
        fileno  = GetConfigFileNo(SZ_SECT_ORDERS);

        config_[fileno][Section] = 

        idx     = m_Config[fileno].GetFirstInSection(SZ_SECT_ORDERS, szName, szValue);
        while (idx>=0)
        {
            m_Config[fileno].SetByName(Section.GetData(), szName, szValue);
            idx = m_Config[fileno].GetNextInSection(idx, SZ_SECT_ORDERS, szName, szValue);
        }
        m_Config[fileno].RemoveSection(SZ_SECT_ORDERS);

        // Upgrade passwords
        S = GetConfig(SZ_SECT_COMMON, SZ_KEY_PWD_OLD);
        S.TrimRight(TRIM_ALL);
        if (!S.IsEmpty())
        {
            Key.Empty();
            Key << (long)m_pAtlantis->m_CrntFactionId;
            SetConfig(SZ_SECT_PASSWORDS, Key.GetData() , S.GetData() );
            SetConfig(SZ_SECT_COMMON   , SZ_KEY_PWD_OLD, (const char *)NULL);
        }
    }
}*/

//-------------------------------------------------------------------------

void CAhApp::ComposeConfigOrdersSection(CStr & Sect, int FactionId)
{
    Sect = SZ_SECT_ORDERS;
    Sect << "_" << (long)FactionId;
}

//-------------------------------------------------------------------------

int CAhApp::GetConfigFileNo(const char * szSection)
{
    if (state_sections_.find(szSection) != state_sections_.end() ||
        0==strnicmp(SZ_SECT_ORDERS, szSection, sizeof(SZ_SECT_ORDERS)-1) ) // orders section is composite starting from 2.1.6
        return CONFIG_FILE_STATE;
    else
        return CONFIG_FILE_CONFIG;
}

//-------------------------------------------------------------------------

const char * CAhApp::GetConfig(const char* section, const char* param_name)
{
    
    int          i;
    int          fileno = GetConfigFileNo(section);

    const char* ret = config_[fileno].get(section, param_name, "");
    if (ret == NULL)
    {
        for (i=0; i<DefaultConfigSize; i++) 
        {
            if ( (0==stricmp(section, DefaultConfig[i].szSection)) &&
                 (0==stricmp(param_name,    DefaultConfig[i].szName))  ) 
            {
                config_[fileno].set(section, param_name, DefaultConfig[i].szValue);
                return DefaultConfig[i].szValue;
            }
        }
        ret = "";
    }
    return ret;
}

//-------------------------------------------------------------------------

void CAhApp::SetConfig(const char* section, const char* param, const char* value)
{
    int  fileno = GetConfigFileNo(section);
    config_[fileno].set(section, param, value);
}

//-------------------------------------------------------------------------

void CAhApp::SetConfig(const char * section, const char * param, long value)
{
    int    fileno = GetConfigFileNo(section);
    config_[fileno].set(section, param, value);
}


//-------------------------------------------------------------------------

int  CAhApp::GetSectionFirst(const char * szSection, const char *& szName, const char *& szValue)
{
    int fileno = GetConfigFileNo(szSection);

    if (config_[fileno].pair_begin(szSection) == config_[fileno].pair_end(szSection))
    {
        for (int i=0; i<DefaultConfigSize; ++i)
            if (0==stricmp(szSection, DefaultConfig[i].szSection))
                config_[fileno].set(szSection, DefaultConfig[i].szName, DefaultConfig[i].szValue);
    }

    auto pb = config_[fileno].pair_begin(szSection);
    if (pb == config_[fileno].pair_end(szSection))
        return -1;

    szName = pb->first.c_str();
    szValue = pb->second.c_str();
    return 0;
}

//-------------------------------------------------------------------------

int  CAhApp::GetSectionNext (int idx, const char * szSection, const char *& szName, const char *& szValue)
{
    int   fileno = GetConfigFileNo(szSection);
    auto it = config_[fileno].pair_begin(szSection);
    std::advance(it, ++idx);
    if (it != config_[fileno].pair_end(szSection))
    {
        szName = it->first.c_str();
        szValue = it->second.c_str();
        return idx;
    }
    return -1;        
}

//-------------------------------------------------------------------------

void  CAhApp::RemoveSection(const char * szSection)
{
    int fileno = GetConfigFileNo(szSection);
    config_[fileno].delete_section(szSection);
}

//-------------------------------------------------------------------------

const char * CAhApp::GetNextSectionName(int fileno, const char * szStart)
{
    if (fileno!=CONFIG_FILE_STATE && fileno!=CONFIG_FILE_CONFIG)
        return nullptr;

    if (szStart == nullptr)
        return config_[fileno].section_begin()->first.c_str();

    auto it = std::lower_bound(config_[fileno].section_begin(), 
                               config_[fileno].section_end(), 
                               szStart,
                               [&](std::pair<std::string, std::map<std::string, std::string>> it1, //config::Config::sect_iterator& it1, 
                                   std::string phrase) {  //config::Config::sect_iterator& it2) {
        return it1.first < phrase;
    });

    if (it == config_[fileno].section_end() || it->first != szStart)
        return nullptr;

    ++it;
    if (it != config_[fileno].section_end())
        return it->first.c_str();

    return nullptr;
}

//-------------------------------------------------------------------------

void CAhApp::ForgetFrame(int no, BOOL frameclosed)
{
    int i;

    if (m_Frames[no])
    {
        m_Frames[no]->Done(frameclosed);

        for (i=0; i<AH_PANE_COUNT; i++)
            if (m_Frames[no]->m_Panes[i])
                m_Panes[i] = NULL;

        if (no == AH_FRAME_MSG)
        {
            m_Frames[no]->Destroy();
        }
        m_Frames[no] = NULL;
        
    }
}


//-------------------------------------------------------------------------

void CAhApp::FrameClosing(CAhFrame * pFrame)
{
    int  no;

    if (pFrame)
        for (no=0; no<AH_FRAME_COUNT; no++)
            if (m_Frames[no] == pFrame)
            {
                if (AH_FRAME_MAP==no)
                {
                    // shutdown in progress
                    for (no=0; no<AH_FRAME_COUNT; no++)
                        ForgetFrame(no, FALSE);
                }
                else
                    ForgetFrame(no, TRUE);
                break;
            }
}

//-------------------------------------------------------------------------

void CAhApp::ShowError(const char * msg, int msglen, BOOL ignore_disabled)
{
    CEditPane * p;

    if (m_DisableErrs && !ignore_disabled)
       return;

    OpenMsgFrame();
    m_MsgSrc.AddStr(msg, msglen);

    p = (CEditPane*)m_Panes[AH_PANE_MSG];
    if (p)
        p->SetSource(&m_MsgSrc, NULL);
}

//-------------------------------------------------------------------------

const char * CAhApp::ResolveAlias(const char * alias)
{//TODO: split to AliasSkills and AliasGeneral(for the rest weird aliases if they really exist)
    static std::unordered_map<std::string, std::string> local_cache;
    if (local_cache.find(alias) != local_cache.end()) {
        std::string res = local_cache[alias];
        if (res.empty())
            return alias;
        return local_cache[alias].c_str();
    }

    int          fileno = GetConfigFileNo(SZ_SECT_ALIAS);
    const char * p = config_[fileno].get_case_insensitive(SZ_SECT_ALIAS, alias, alias);


    //const char * p = SkipSpaces(GetConfig(SZ_SECT_ALIAS, alias));
    if (p && *p) {
        local_cache[alias] = p;
        return p;
    }
    local_cache[alias] = "";
    return alias;
}

bool CAhApp::ResolveAliasItems(const std::string& phrase, std::string& codename, std::string& name, std::string& name_plural)
{
    struct Aliases {
      std::string code_;
      std::string name_;
      std::string name_plural_;
    };

    static std::unordered_map<std::string, std::shared_ptr<Aliases>> cache;
    if (cache.find(phrase) != cache.end()) {
        if (cache[phrase]->code_.empty())
            return false;
        codename = cache[phrase]->code_;
        name = cache[phrase]->name_;
        name_plural = cache[phrase]->name_plural_;
        return true;
    }

    const char* pkey;
    const char* pvalue;
    std::string current_codename, current_name, current_name_plural, value;
    int sectidx = gpApp->GetSectionFirst(SZ_SECT_ALIAS_ITEMS, pkey, pvalue);
    while (sectidx >= 0)
    {
        if (pkey == NULL || pvalue == NULL)
        {
            sectidx = gpApp->GetSectionNext(sectidx, SZ_SECT_ALIAS_ITEMS, pkey, pvalue);
            continue;
        }
        current_codename = pkey;
        value = pvalue;

        size_t pos = value.find(",");
        if (pos == std::string::npos)
        {
            sectidx = gpApp->GetSectionNext(sectidx, SZ_SECT_ALIAS_ITEMS, pkey, pvalue);
            continue;
        }
        current_name = value.substr(0, pos);
        current_name_plural = value.substr(pos+1);

        if (current_name.empty())
            current_name = current_name_plural;
        if (current_name_plural.empty())
            current_name_plural = current_name;

        if (stricmp(phrase.c_str(), current_codename.c_str()) == 0 || 
            stricmp(phrase.c_str(), current_name.c_str()) == 0 ||
            stricmp(phrase.c_str(), current_name_plural.c_str()) == 0)
        {
            codename = current_codename;
            name = current_name;
            name_plural = current_name_plural;
            cache[phrase] = std::make_shared<Aliases>();
            cache[phrase]->code_ = codename;
            cache[phrase]->name_ = name;
            cache[phrase]->name_plural_ = name_plural;
            return true;
        }
        sectidx = gpApp->GetSectionNext(sectidx, SZ_SECT_ALIAS_ITEMS, pkey, pvalue);
    }
    cache[phrase] = std::make_shared<Aliases>();
    cache[phrase]->code_ = "";
    return false;
}

void CAhApp::SetAliasItems(const std::string& codename, const std::string& long_name, const std::string& long_name_plural)
{
    std::string compose = long_name + "," + long_name_plural;
    gpApp->SetConfig(SZ_SECT_ALIAS_ITEMS, codename.c_str(), compose.c_str());
}

//-------------------------------------------------------------------------
/*
long CAhApp::GetStudyCost(const char * skill)
{
    long        n;
    const char *p;

    p = ResolveAlias(skill);
    n = atol(GetConfig(SZ_SECT_STUDY_COST, p));

    int fileno = GetConfigFileNo(SZ_SECT_STUDY_COST);
    const char* ret = config_[fileno].get_case_insensitive<long>(SZ_SECT_STUDY_COST, skill, 0);

    return n;
}*/

//-------------------------------------------------------------------------

BOOL CAhApp::GetItemWeights(const char * item, int *& weights, const char **& movenames, int & movecount )
{
    ItemWeights   Dummy;
    ItemWeights * pWeights;
    int           i;
    const char  * p;
    BOOL          Ok = TRUE;
    BOOL          Update = FALSE;


    Dummy.name = (char *)item;

    if (m_ItemWeights.Search(&Dummy, i))
        pWeights = (ItemWeights *)m_ItemWeights.At(i);
    else
    {
        CStr S;

        p = SkipSpaces(GetConfig(SZ_SECT_WEIGHT_MOVE, item));


        Ok = (p && *p);
        pWeights          = new ItemWeights;
        pWeights->name    = strdup(item);
        pWeights->weights = (int*)malloc(m_MoveModes.Count()*sizeof(int));


        for (i=0; i<m_MoveModes.Count(); i++)
        {
            p = SkipSpaces(S.GetToken(p, ','));
            if (i==4 && S.IsEmpty())
            {
                // Update swimming for 2.3.2
                int x;
                for (x=0; x<DefaultConfigSize; x++)
                    if ( (0==stricmp(SZ_SECT_WEIGHT_MOVE, DefaultConfig[x].szSection)) &&
                         (0==stricmp(item               , DefaultConfig[x].szName))  )
                    {
                        const char * q = DefaultConfig[x].szValue;
                        int          m;
                        for (m=0; m<=i; m++)
                            q = SkipSpaces(S.GetToken(q, ','));
                        Update = TRUE;
                        break;
                    }
            }
            pWeights->weights[i] = atoi(S.GetData());
        }
        if (Update && !IsASkillRelatedProperty(item))
        {
            // Update swimming for 2.3.2
            S.Empty();
            for (i=0; i<m_MoveModes.Count(); i++)
            {
                if (i>0)
                    S << ',';
                S << (long)pWeights->weights[i];
            }
            SetConfig(SZ_SECT_WEIGHT_MOVE, item, S.GetData());
        }
        m_ItemWeights.Insert(pWeights);
    }

    weights   = pWeights->weights;
    movenames = (const char **)m_MoveModes.GetItems();
    movecount = m_MoveModes.Count();

    if (!Ok)
    {
        CStr S;
        bool skipit;
        if (!IsASkillRelatedProperty(item))
        {
            for (i=0; i<STD_UNIT_PROPS_COUNT; i++)
                if (0==stricmp(item, STD_UNIT_PROPS[i]))
                {
                    skipit = TRUE;
                    break;
                }
            if (!skipit)
            {
                S.Empty();
                S << "Warning! Weight and capacities for " << item <<
                     " are unknown and assumed to be zero. Movement modes can not be calculated correct. Update your " <<
                     SZ_CONFIG_FILE << " file!" <<EOL_SCR;
                ShowError(S.GetData(), S.GetLength(), TRUE);
            }
        }
    }

    return Ok;
}


//-------------------------------------------------------------------------

void CAhApp::GetMoveNames(const char **& movenames)
{
    movenames = (const char **)m_MoveModes.GetItems();
}

//-------------------------------------------------------------------------

BOOL CAhApp::GetOrderId(const char * order, long & id)
{
    const void * data = NULL;
    BOOL  Ok;

    Ok = m_OrderHash.Locate(order, data);
    id = (long)data;

    return Ok;
}

//-------------------------------------------------------------------------

BOOL CAhApp::IsTradeItem(const char * item)
{
    const void * data = NULL;

    return m_TradeItemsHash.Locate(item, data);
}

//-------------------------------------------------------------------------

BOOL CAhApp::IsMan(const char * item)
{
    const void * data = NULL;

    return m_MenHash.Locate(item, data);
}

//-------------------------------------------------------------------------

BOOL CAhApp::IsMagicSkill(const char * skill)
{
    const void * data = NULL;

    return m_MagicSkillsHash.Locate(skill, data);
}

//-------------------------------------------------------------------------

const char * CAhApp::GetWeatherLine(BOOL IsCurrent, BOOL IsGood, int Zone)
{
    const char * szKey = NULL;

    if (IsCurrent)
        if (IsGood)
            if (0==Zone) //Tropic
                szKey = SZ_KEY_WEATHER_CUR_GOOD_TROPIC;
            else
                szKey = SZ_KEY_WEATHER_CUR_GOOD_MEDIUM;
        else
            if (0==Zone) //Tropic
                szKey = SZ_KEY_WEATHER_CUR_BAD_TROPIC;
            else
                szKey = SZ_KEY_WEATHER_CUR_BAD_MEDIUM;
    else
        if (IsGood)
            if (0==Zone) //Tropic
                szKey = SZ_KEY_WEATHER_NEXT_GOOD_TROPIC;
            else
                szKey = SZ_KEY_WEATHER_NEXT_GOOD_MEDIUM;
        else
            if (0==Zone) //Tropic
                szKey = SZ_KEY_WEATHER_NEXT_BAD_TROPIC;
            else
                szKey = SZ_KEY_WEATHER_NEXT_BAD_MEDIUM;

    return GetConfig(SZ_SECT_WEATHER, szKey);
}

//-------------------------------------------------------------------------

std::shared_ptr<TProdDetails> CAhApp::GetProdDetails(const char* item)
{
    static std::map<std::string, std::shared_ptr<TProdDetails>> known_item_details;
    auto search = known_item_details.find(item);
    if (search != known_item_details.end()) {
        return search->second;
    }

    std::shared_ptr<TProdDetails> ret = std::make_shared<TProdDetails>();
    game_control::NameAndAmount skill_val = game_control::get_game_config_val<game_control::NameAndAmount>(SZ_SECT_PROD_SKILL, item);
    ret->skill_name_ = skill_val.name_;
    ret->skill_level_ = skill_val.amount_;

    std::vector<game_control::NameAndAmount> resources = game_control::get_game_config<game_control::NameAndAmount>(SZ_SECT_PROD_RESOURCE, item);
    ret->req_resources_.resize(resources.size());
    std::transform(resources.begin(), resources.end(), ret->req_resources_.begin(), 
        [](const game_control::NameAndAmount& name_and_amount) {
            return std::pair<std::string, double>(name_and_amount.name_, name_and_amount.amount_);
    });

    ret->per_month_ = game_control::get_game_config_val<double>(SZ_SECT_PROD_MONTHS, item);
    game_control::NameAndAmount tool_val = game_control::get_game_config_val<game_control::NameAndAmount>(SZ_SECT_PROD_TOOL, item);
    ret->tool_name_ = tool_val.name_;
    ret->tool_plus_ = tool_val.amount_;

    known_item_details[item] = ret;
    return ret;
}

//-------------------------------------------------------------------------

BOOL CAhApp::CanSeeAdvResources(const char * skillname, const char * terrain, CLongColl & Levels, CBufColl & Resources)
{
    CStr         ProdSkillLine;
    CStr         ProdLandLine;
    BOOL         Ok = FALSE;
    const char * p1, * p2, *p;
    CStr         Prod1, Prod2, S1;
    long         level;

    Levels.FreeAll();
    Resources.FreeAll();

    ProdSkillLine = GetConfig(SZ_SECT_RESOURCE_SKILL,  skillname);
    ProdSkillLine.TrimRight(TRIM_ALL);

    ProdLandLine = GetConfig(SZ_SECT_RESOURCE_LAND,  terrain);
    ProdLandLine.TrimRight(TRIM_ALL);

    if (!ProdSkillLine.IsEmpty() && !ProdLandLine.IsEmpty())
    {
        p1 = SkipSpaces(S1.GetToken(ProdSkillLine.GetData(), ',', TRIM_ALL));
        while (!S1.IsEmpty())
        {
            p  = SkipSpaces(Prod1.GetToken(S1.GetData(), ' ', TRIM_ALL));
            level = p ? atol(p) : 0;

            p2 = SkipSpaces(Prod2.GetToken(ProdLandLine.GetData(), ',', TRIM_ALL));
            while (!Prod2.IsEmpty())
            {
                if (0==stricmp(Prod1.GetData(), Prod2.GetData()))
                {
                    Ok = TRUE;
                    Levels.Insert((void*)level);
                    Resources.Insert(strdup(Prod1.GetData()));
                    break;
                }
                p2 = SkipSpaces(Prod2.GetToken(p2, ',', TRIM_ALL));
            }

            p1 = SkipSpaces(S1.GetToken(p1, ',', TRIM_ALL));
        }
    }

    return Ok;
}


//-------------------------------------------------------------------------

int64_t CAhApp::GetAttitudeForFaction(int id)
{
    int player_id = atol( GetConfig(SZ_SECT_ATTITUDES, SZ_ATT_PLAYER_ID));
    if(id == player_id) return ATT_FRIEND2;
    int attitude = ATT_UNDECLARED;
    CAttitude * policy;
    for(int i=m_Attitudes.Count()-1; i>=0; i--)
    {
        policy = (CAttitude *) m_Attitudes.At(i);
        if(policy->FactionId == id) attitude=policy->Stance;
    }
    if(attitude == ATT_UNDECLARED)
    {
        // check for default attitude
        for(int i=m_Attitudes.Count()-1; i>=0; i--)
        {
            policy = (CAttitude *) m_Attitudes.At(i);
            if(policy->FactionId == 0) attitude=policy->Stance;
        }
    }
    return attitude;
}

//-------------------------------------------------------------------------
void CAhApp::SetAttitudeForFaction(int id, int attitude)
{
    int att_idx = -1;
    CAttitude * policy;
    if((attitude < ATT_FRIEND1) || (attitude >= ATT_UNDECLARED)) return;
    for(int i=m_Attitudes.Count()-1; i>=0; i--)
    {
        policy = (CAttitude *) m_Attitudes.At(i);
        if(policy && (policy->FactionId == id)) att_idx=i;
    }
    if(att_idx < 0)
    {   // new attitude declaration
        policy = new CAttitude;
        policy->FactionId = id;
        policy->SetStance(attitude);
        m_Attitudes.Insert(policy);
    }
    else
    {   // change existing declaration
        policy = (CAttitude *) m_Attitudes.At(att_idx);
        policy->SetStance(attitude);
    }
}

//-------------------------------------------------------------------------

void CAhApp::GetShortFactName(CStr & S, int FactionId)
{
#define MAX_F_NAME 8
    int           i;
    char          ch;
    CFaction    * pFaction;
//    CBaseObject   Dummy;
//    int           idx;

    S.Empty();
//    Dummy.Id = FactionId;
//    if (m_pAtlantis->m_Factions.Search(&Dummy, idx))
//    {
//        pFaction = (CFaction*)m_pAtlantis->m_Factions.At(idx);
//        S = pFaction->Name;
//    }
//    else
//        S << (long)FactionId;
    pFaction = m_pAtlantis->GetFaction(FactionId);
    if (pFaction)
        S = pFaction->Name;
    else
        S << (long)FactionId;

    if (0==stricmp(S.GetData(), "faction"))
    {
        S.Empty();
        S << "F_" << (long)FactionId << "_";
    }

    S.ToLower();
    for (i=S.GetLength()-1; i>=0; i--)
    {
        ch = S.GetData()[i];
        if ( (ch < 'a' || ch > 'z') && (ch < '0' || ch > '9') )
            S.DelCh(i);
    }
    if (S.GetLength() > MAX_F_NAME)
        S.DelSubStr(MAX_F_NAME, S.GetLength()-MAX_F_NAME);
    S.TrimRight(TRIM_ALL);

}

//-------------------------------------------------------------------------

void CAhApp::SetMapFrameTitle()
{
    CMapFrame   * pMapFrame  = (CMapFrame *)m_Frames[AH_FRAME_MAP];
    CMapPane    * pMapPane   = (CMapPane  * )m_Panes[AH_PANE_MAP];
    CPlane      * pPlane     = NULL;

    CStr          S;

    S = m_sTitle;

    if (pMapPane)
    {
        pPlane = (CPlane*)m_pAtlantis->m_Planes.At(pMapPane->m_SelPlane);

        S << " (" << pMapPane->m_SelHexX << "," << pMapPane->m_SelHexY;
        if (pPlane && 0!=stricmp(DEFAULT_PLANE, pPlane->Name.GetData()))
        {
            S << "," << pPlane->Name;
        }
        S << ")";
    }

    if (orders_changed())
        S << " [modified]";
    if (pMapFrame)
        pMapFrame->SetTitle(wxString::FromAscii(S.GetData()));
}

//-------------------------------------------------------------------------

void CAhApp::orders_changed(bool changed)
{
    orders_changed_flag_ = changed;

    SetMapFrameTitle();
}


//-------------------------------------------------------------------------

int CAhApp::SaveOrders(BOOL UsingExistingName)
{
    CStr S, FName, Section;
    int  i, id, err=ERR_OK;

    for (i=0; i<m_pAtlantis->m_OurFactions.Count(); i++)
    {
        id = (long)m_pAtlantis->m_OurFactions.At(i);
        if (UsingExistingName)
        {
            ComposeConfigOrdersSection(Section, id);
            S.Empty();
            S << (long)m_pAtlantis->m_YearMon;
            FName = GetConfig(Section.GetData(), S.GetData());
            FName.TrimRight(TRIM_ALL);
        }
        err = SaveOrders(FName.GetData(), id);
        if (ERR_OK!=err)
            break;
    }

    if (ERR_OK==err)
        orders_changed(false);

    return err;
}

//-------------------------------------------------------------------------

int  CAhApp::SaveOrders(const char * FNameOut, int FactionId)
{

    int         err;
    char        buf[64];
    CStr        FName;
    CStr        Dir;
    CStr        S, Section, Prompt, Key;
    CFaction  * pFaction;

    FName = FNameOut;
    FName.TrimRight(TRIM_ALL);

    ComposeConfigOrdersSection(Section, FactionId);
    if (FName.IsEmpty())
    {
        S.Format("%d", m_pAtlantis->m_YearMon);
        FName = GetConfig(Section.GetData(), S.GetData());
        FName.TrimRight(TRIM_ALL);

        if (FName.IsEmpty())
        {
            GetShortFactName(S, FactionId);
            if (S.IsEmpty())
                S << (long)FactionId;
            FName.Format("%s%04d.ord", S.GetData(), m_pAtlantis->m_YearMon);
        }
        pFaction = m_pAtlantis->GetFaction(FactionId);

        Prompt = "Save orders for ";
        if (pFaction)
            Prompt << pFaction->Name.GetData() << " ";
        else
            Prompt << "Faction ";
        Prompt << (long)FactionId;

        Dir = GetConfig(SZ_SECT_FOLDERS, SZ_KEY_FOLDER_ORDERS);
        Dir.TrimRight(TRIM_ALL);
        if (Dir.IsEmpty())
            Dir = ".";

        CStr File;
        wxString CurrentDir = wxGetCwd();
        //MakePathFull(CurrentDir.mb_str(), FName);
        GetFileFromPath(FName.GetData(), File);

        MakePathFull(CurrentDir.mb_str(), Dir);
        wxFileDialog dialog((CMapFrame*)m_Frames[AH_FRAME_MAP],
                            wxString::FromAscii(Prompt.GetData()),
                            wxString::FromAscii(Dir.GetData()),
                            wxString::FromAscii(File.GetData()),
                            wxT(SZ_ORD_FILES),
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
        err = dialog.ShowModal();
        wxSetWorkingDirectory(CurrentDir);

        if (wxID_OK == err)
        {
            FName = dialog.GetPath().mb_str();
            MakePathRelative(CurrentDir.mb_str(), FName);
            GetDirFromPath(FName.GetData(), Dir);
            SetConfig(SZ_SECT_FOLDERS, SZ_KEY_FOLDER_ORDERS, Dir.GetData() );
        }
        else
            return ERR_CANCEL;

        FName.TrimRight(TRIM_ALL);
    }
    if (FName.IsEmpty())
        return ERR_FNAME;

    Key.Empty();
    Key << (long)FactionId;

    err = m_pAtlantis->SaveOrders(FName.GetData(),
                                  GetConfig(SZ_SECT_PASSWORDS, Key.GetData()),
                                  (BOOL)atol(GetConfig(SZ_SECT_COMMON, SZ_KEY_DECORATE_ORDERS)),
                                  FactionId
                                 );
    if (ERR_OK==err)
    {
        sprintf(buf, "%ld", m_pAtlantis->m_YearMon);
        SetConfig(Section.GetData(), buf, FName.GetData());
    }

    // Save config, too
    config_[CONFIG_FILE_CONFIG].save(SZ_CONFIG_FILE);
    config_[CONFIG_FILE_STATE ].save(SZ_CONFIG_STATE_FILE);

    if (ERR_OK==m_pAtlantis->m_ParseErr)
        SaveHistory(SZ_HISTORY_FILE);

    return err;
}

//-------------------------------------------------------------------------

void CAhApp::RedrawTracks(std::vector<long>& points) {
    //currently drawing tracks is sticked to CUnit object, which is a design problem, but
    //it requires to rewrite & redesign all the painting system, which is complicated.
    //so it's a trick: we create fake unit with filled fields to write a custom track.
    if (points.size() < 2)
        return;

    std::shared_ptr<CUnit> unit = std::make_shared<CUnit>();
    auto it = points.begin();
    unit->LandId = *it;
    ++it;
    unit->movements_.insert(unit->movements_.begin(), it, points.end());
    unit->movement_stop_ = points[points.size()-1];

    CMapPane    * pMapPane  = (CMapPane* )m_Panes[AH_PANE_MAP];
    if (!pMapPane)
        return;

    CPlane* pPlane = (CPlane*)m_pAtlantis->m_Planes.At(pMapPane->m_SelPlane);
    pMapPane->RedrawTracksForUnit(pPlane, unit.get(), NULL, TRUE);
}

void CAhApp::RedrawTracks()
{
    CUnit       * pUnit = GetSelectedUnit();
    CPlane      * pPlane;
    CMapPane    * pMapPane  = (CMapPane* )m_Panes[AH_PANE_MAP];

    if (!pMapPane)
        return;

    pPlane   = (CPlane*)m_pAtlantis->m_Planes.At(pMapPane->m_SelPlane);
    pMapPane->RedrawTracksForUnit(pPlane, pUnit, NULL, TRUE);
}

//-------------------------------------------------------------------------

CUnit * CAhApp::GetSelectedUnit()
{
    CUnit       * pUnit = NULL;
    CUnitPane   * pUnitPane = (CUnitPane*)m_Panes[AH_PANE_UNITS_HEX];

    if (pUnitPane)
        pUnit = pUnitPane->GetSelectedUnit();// m_pUnits->At(m_SelUnitIdx);

    return pUnit;
}

//-------------------------------------------------------------------------

int  CAhApp::LoadOrders  (const char * FNameIn)
{
    int           err;
    CStr          S(32), FName, Sect;
    int           factid;
//    CMapPane    * pMapPane = (CMapPane* )m_Panes[AH_PANE_MAP];


    FName = FNameIn;  // FNameIn can be coming from config, so do not use it directly!
    err = m_pAtlantis->LoadOrders(FName.GetData(), factid);
    if (ERR_OK==err)
    {
        S.Empty();
        S << (long)m_pAtlantis->m_YearMon;
        ComposeConfigOrdersSection(Sect, factid);
        SetConfig(Sect.GetData(), S.GetData(), FName.GetData());

//        if (pMapPane)
//            pMapPane->Refresh(FALSE, NULL);
//            pMapPane->CleanCities(); //pMapPane->Refresh(FALSE, NULL); // to remove pointers to land wich could be replaced by joining orders

        OnMapSelectionChange();
        RedrawTracks();
    }
    return err;
}

//-------------------------------------------------------------------------

void EncodeConfigLine(CStr & dest, const char * src)
{
    dest.Empty();
    while (src && *src)
    {
        switch (*src)
        {
        case '\r':  break;
        case '\n':  dest.AddStr("\\n", 2);
                    break;
        default  :  dest.AddCh(*src);
        }
        src++;
    }
}

//-------------------------------------------------------------------------

void DecodeConfigLine(CStr & dest, const char * src)
{
    BOOL          Esc;

    dest.Empty();
    Esc = FALSE;
    while (src && *src)
    {
        if ('\\' == *src)


            Esc = TRUE;
        else
        {
            if (Esc)
            {
                switch(*src)
                {
                case 'n':
                    dest << EOL_SCR;
                    break;
                default:
                    dest.AddCh('\\');
                    dest.AddCh(*src);
                }
            }
            else
                dest.AddCh(*src);
            Esc = FALSE;
        }

        src++;
    }
}

//-------------------------------------------------------------------------

void CAhApp::LoadComments()
{
    char          buf[32];
    CStr          S;

    for (CUnit* unit : m_pAtlantis->units_)
    {
        sprintf(buf, "%ld", unit->Id);

        DecodeConfigLine(unit->DefOrders, GetConfig(SZ_SECT_DEF_ORDERS, buf));

        unit->DefOrders.TrimRight(TRIM_ALL);
        unit->ExtractCommentsFromDefOrders();
    }
    m_CommentsChanged = FALSE;
}

//-------------------------------------------------------------------------

void CAhApp::SaveComments()
{
    char          buf[32];
    CStr          S;
    const char  * p;

    for (CUnit* pUnit : m_pAtlantis->units_)
    {
        S.Empty();
        pUnit->DefOrders.TrimRight(TRIM_ALL);
        if (pUnit->DefOrders.GetLength() > 0)
        {
            EncodeConfigLine(S, pUnit->DefOrders.GetData());
            p = S.GetData();
        }
        else
            p = NULL;
        sprintf(buf, "%ld", pUnit->Id);
        SetConfig(SZ_SECT_DEF_ORDERS, buf, p);
    }
    m_CommentsChanged = FALSE;
}


//-------------------------------------------------------------------------

void CAhApp::LoadUnitFlags()
{
    int           x;
    char          buf[32];
    CStr          S;

    for (CUnit* pUnit : m_pAtlantis->units_)
    {
        sprintf(buf, "%ld", pUnit->Id);

        x = atol(GetConfig(SZ_SECT_UNIT_FLAGS, buf));
        if (x & UNIT_CUSTOM_FLAG_MASK)
        {
            pUnit->Flags    |= (x & UNIT_CUSTOM_FLAG_MASK);
            pUnit->FlagsOrg |= (x & UNIT_CUSTOM_FLAG_MASK);
            pUnit->FlagsLast = ~pUnit->Flags;
        }
    }
}

//-------------------------------------------------------------------------

void CAhApp::SaveUnitFlags()
{
    char          buf[32];
    CStr          S;

    for (CUnit* pUnit : m_pAtlantis->units_)
    {
        sprintf(buf, "%ld", pUnit->Id);

        S.Empty();
        if (pUnit->Flags & UNIT_CUSTOM_FLAG_MASK)
            S << (long)(pUnit->Flags & UNIT_CUSTOM_FLAG_MASK);
        SetConfig(SZ_SECT_UNIT_FLAGS, buf, S.GetData());
    }
}

//-------------------------------------------------------------------------

void CAhApp::SetAllLandUnitFlags()
{
    CUnitPane  * pUnitPane = (CUnitPane*)m_Panes[AH_PANE_UNITS_HEX];
    CPlane     * pPlane;
    CLand      * pLand;
    int          i, n, f, x;
    int          rc;

    CUnitFlagsDlg dlg(m_Frames[AH_FRAME_MAP], eAll, 0);

    rc = dlg.ShowModal();

    if ((ID_BTN_SET_ALL_LAND==rc || ID_BTN_RMV_ALL_LAND==rc) && dlg.m_LandFlags>0)
    {
        for (n=0; n<m_pAtlantis->m_Planes.Count(); n++)
        {
            pPlane = (CPlane*)m_pAtlantis->m_Planes.At(n);
            for (i=0; i<pPlane->Lands.Count(); i++)
            {
                pLand = (CLand*)pPlane->Lands.At(i);
                x     = 1;
                for (f=0; f<LAND_FLAG_COUNT; f++)
                {
                    if (dlg.m_LandFlags & x)
                    {
                        if (ID_BTN_RMV_ALL_LAND==rc)
                        {
                            // clear flag
                            pLand->FlagText[f].Empty();
                        }
                        else
                        {
                            // set flag
                            if (pLand->FlagText[f].IsEmpty())
                                pLand->FlagText[f] = LandFlagLabel[f];
                        }
                        pLand->Flags |= LAND_HAS_FLAGS;
                    }
                    x <<= 1;
                }
            }
        }

        if (m_Panes[AH_PANE_MAP])
            (m_Panes[AH_PANE_MAP])->Refresh(FALSE);
    }

    if ( (ID_BTN_SET_ALL_UNIT==rc || ID_BTN_RMV_ALL_UNIT==rc) && dlg.m_UnitFlags>0 )
    {
        for (CUnit* pUnit : m_pAtlantis->units_)
        {
            if (ID_BTN_SET_ALL_UNIT==rc)
            {
                pUnit->Flags    |= (dlg.m_UnitFlags & UNIT_CUSTOM_FLAG_MASK);
                pUnit->FlagsOrg |= (dlg.m_UnitFlags & UNIT_CUSTOM_FLAG_MASK);
            }
            else
            {
                pUnit->Flags    &= ~(dlg.m_UnitFlags & UNIT_CUSTOM_FLAG_MASK);
                pUnit->FlagsOrg &= ~(dlg.m_UnitFlags & UNIT_CUSTOM_FLAG_MASK);
            }

            pUnit->FlagsLast = ~pUnit->Flags;
        }
        if (pUnitPane)
            pUnitPane->Update(pUnitPane->m_pCurLand);
    }
}

//-------------------------------------------------------------------------

void CAhApp::SaveLandFlags()
{
    int          i, n, f;
    CPlane     * pPlane;
    CLand      * pLand;
    CStr         sName;
    CStr         sData;
    long         ym_last;
    long         ym_first;
    const char * p;

    for (n=0; n<m_pAtlantis->m_Planes.Count(); n++)
    {
        pPlane = (CPlane*)m_pAtlantis->m_Planes.At(n);
        for (i=0; i<pPlane->Lands.Count(); i++)
        {
            pLand = (CLand*)pPlane->Lands.At(i);
            sData.Empty();
            for (f=0; f<LAND_FLAG_COUNT; f++)
            {
                pLand->FlagText[f].TrimRight(TRIM_ALL);
                if (!pLand->FlagText[f].IsEmpty())
                {
                    if (!sData.IsEmpty())
                        sData << "\\n";
                    sData << (long)f << ":" << pLand->FlagText[f];
                }
            }
            m_pAtlantis->ComposeLandStrCoord(pLand, sName);


            if (!sData.IsEmpty() || (pLand->Flags & LAND_HAS_FLAGS)) // allow to remove flags
                SetConfig(SZ_SECT_LAND_FLAGS, sName.GetData(), sData.GetData());

            if (pLand->Flags&LAND_IS_CURRENT) //LAND_UNITS)
            {
                //ym = atol(GetConfig(SZ_SECT_LAND_VISITED, sName.GetData()));
                p        = sData.GetToken(GetConfig(SZ_SECT_LAND_VISITED, sName.GetData()), ',');
                ym_last  = atol(sData.GetData());
                if (sData.IsEmpty())
                    ym_first = m_pAtlantis->m_YearMon;
                else
                {
                    p        = sData.GetToken(SkipSpaces(p), ',');
                    ym_first = atol(sData.GetData());
                }
                if (ym_last < m_pAtlantis->m_YearMon)
                {
                    sData.Empty();
                    sData << m_pAtlantis->m_YearMon << "," << ym_first;
                    SetConfig(SZ_SECT_LAND_VISITED, sName.GetData(), sData.GetData());
                }
            }
        }
    }

//    m_LandFlagsChanged = FALSE;
}

//-------------------------------------------------------------------------

void CAhApp::LoadLandFlags()
{
    int               sectidx, n;
    const char      * szName;
    const char      * szValue;
    const char      * p;
    const char      * line;
    CStr              sData, sLine, sN;
    CLand           * pLand;

    sectidx = GetSectionFirst(SZ_SECT_LAND_FLAGS, szName, szValue);
    while (sectidx >= 0)
    {
        pLand   = m_pAtlantis->GetLand(szName);
        if (pLand)
        {
            DecodeConfigLine(sData, szValue);

            line = sData.GetData();
            while (line && *line)
            {
                line = sLine.GetToken(line, '\n');
                p    = sLine.GetData();
                p    = sN.GetToken(p, ':');
                if (p)
                    n = atoi(sN.GetData());
                else
                {
                    p = sN.GetData();
                    n = 0;
                }
                if (n<0 || n>=LAND_FLAG_COUNT)
                    n = 0;
                pLand->FlagText[n] = p;
                pLand->Flags |= LAND_HAS_FLAGS;
            }

        }
        sectidx = GetSectionNext(sectidx, SZ_SECT_LAND_FLAGS, szName, szValue);
    }


}


//-------------------------------------------------------------------------

bool CAhApp::terrain_type_water(CLand* land)
{
    static std::unordered_map<std::string, bool> cache;
    if (cache.find(land->TerrainType.GetData()) != cache.end())
        return cache[land->TerrainType.GetData()];
    
    auto vec = game_control::get_game_config<std::string>(SZ_SECT_COMMON, SZ_KEY_WATER_TERRAINS);
    for (auto& type : vec)
    {
        if (stricmp(land->TerrainType.GetData(), type.c_str()) == 0) 
        {
            cache.insert({land->TerrainType.GetData(), true});
            return true;
        }
    }
    cache.insert({land->TerrainType.GetData(), false});
    return false;
}

void CAhApp::UpdateEdgeStructs()
{
    int          i, n, k;
    int          d, adj_dir;
    int          x, y, z;
    int          adj_index;
    CPlane     * pPlane;
    CLand      * pLand, * adj_land;
    CStruct    * pEdge;

    for (n=0; n<m_pAtlantis->m_Planes.Count(); n++)
    {
        pPlane = (CPlane*)m_pAtlantis->m_Planes.At(n);
        for (i=0; i<pPlane->Lands.Count(); i++)
        {
            pLand = (CLand*)pPlane->Lands.At(i);
            if(!pLand) continue;
            // set the Water-Type flag
            if(terrain_type_water(pLand))
            {
                pLand->Flags |= LAND_IS_WATER;
            }
            for(d=0; d<6; d++)
            {
                adj_dir = (d%6)-3;
                if(adj_dir < 0) adj_dir += 6;
                LandIdToCoord(pLand->Id,x,y,z);
                m_pAtlantis->ExtrapolateLandCoord(x,y,z,d);

                CBaseObject Dummy;
                Dummy.Id = LandCoordToId(x, y, z);
                if (pPlane->Lands.Search(&Dummy, adj_index))
                {
                    adj_land = (CLand *) pPlane->Lands.At(adj_index);
                    if(adj_land)
                    {
                        if((pLand->Flags&LAND_IS_CURRENT) && !(adj_land->Flags&LAND_IS_CURRENT))
                        {  // set the corresponding Edge Structure in adjacent region
                            adj_land->RemoveEdgeStructs(adj_dir);
                            for(k=pLand->EdgeStructs.Count()-1; k>=0; k--)
                            {
                                pEdge = (CStruct*) pLand->EdgeStructs.At(k);
                                if((pEdge != NULL) && (pEdge->Location == d))                          
                                    adj_land->AddNewEdgeStruct(pEdge->type_.c_str(), adj_dir);
                            }
                        }
                        // set CoastBits
                        if(terrain_type_water(adj_land) && !land_control::is_water(pLand))
                            adj_land->CoastBits |= ExitFlags[adj_dir];
                        else if(!terrain_type_water(adj_land) && land_control::is_water(pLand))
                        {
                            pLand->CoastBits |= ExitFlags[d];
                        }
                    }
                }
            }
        }
    }
}


//-------------------------------------------------------------------------

void CAhApp::WriteMagesCSV()
{
    CStr FName;
    CStr S;


//    GetShortFactName(S);
//    FName.Format("%s_%s%04d.csv", S.GetData(), "mages", m_pAtlantis->m_YearMon);
    FName.Format("%s%04d.csv", "mages", m_pAtlantis->m_YearMon);

    CExportMagesCSVDlg Dlg(m_Frames[AH_FRAME_MAP], FName.GetData());
    if (wxID_OK == Dlg.ShowModal())
        m_pAtlantis->WriteMagesCSV(Dlg.m_pFileName->GetValue().mb_str(),
                                   0==SafeCmp(Dlg.m_pOrientation->GetValue().mb_str(), SZ_VERTICAL),
                                   Dlg.m_pSeparator->GetValue().mb_str(),
                                   Dlg.m_nFormat
                                  );
}

//-------------------------------------------------------------------------

void CAhApp::CheckTaxDetails  (CLand  * pLand, CTaxProdDetailsCollByFaction & TaxDetails)
{
    int               x;
    CUnit           * pUnit;
    EValueType        type;
    long              men;
    CStr              sCoord;
    CTaxProdDetails * pDetail;
    CTaxProdDetails   Dummy;
    int               idx;
    CTaxProdDetailsCollByFaction Factions;
    wxString          OneLine;

    for (x=0; x<pLand->Units.Count(); x++)
    {
        pUnit = (CUnit*)pLand->Units.At(x);
        if (pUnit->Flags & UNIT_FLAG_TAXING)
        {
            Dummy.FactionId = pUnit->FactionId;
            if (TaxDetails.Search(&Dummy, idx))
                pDetail = (CTaxProdDetails*)TaxDetails.At(idx);
            else
            {
                pDetail = new CTaxProdDetails;
                pDetail->FactionId = pUnit->FactionId;
                TaxDetails.Insert(pDetail);
            }
            if (Factions.Insert(pDetail))
            {
                pDetail->amount = pLand->current_state_.tax_.amount_;
                pDetail->HexCount++;
            }
            if (pUnit->GetProperty(PRP_MEN, type, (const void *&)men, eNormal) && eLong==type)
                pDetail->amount -= men*atol(GetConfig(SZ_SECT_COMMON, SZ_KEY_TAX_PER_TAXER));
        }
    }


    // Output
    for (int iFac=0; iFac<Factions.Count(); iFac++)
    {
        pDetail = (CTaxProdDetails*)Factions.At(iFac);
        OneLine.Empty();

        m_pAtlantis->ComposeLandStrCoord(pLand, sCoord);
        OneLine << wxString::FromUTF8(pLand->TerrainType.GetData()) << wxT(" (") << wxString::FromUTF8(sCoord.GetData()) << wxT(") ");
        if (!pLand->CityName.IsEmpty())
            OneLine << wxString::FromUTF8(pLand->CityName.GetData()) << wxT(" ");

        wxCoord x, y;
        wxClientDC myDC(m_Frames[AH_FRAME_MAP]);
        do
        {
            OneLine.Append(wxT(" "));
            myDC.GetTextExtent(OneLine.GetData(), &x, &y, NULL, NULL, (m_Fonts[FONT_ERR_DLG]));
        }
        while (x < 245);

        if (pDetail->amount > 0)
            OneLine << wxT("is undertaxed by ") << pDetail->amount << wxT(" silv (") << 100 * (pLand->current_state_.tax_.amount_ - pDetail->amount) / (pLand->current_state_.tax_.amount_+1) << wxT("% of $") << pLand->current_state_.tax_.amount_ << wxT(").") << wxString::FromUTF8(EOL_SCR);
        else if (pDetail->amount<0)
            OneLine << wxT("is overtaxed  by ") << (-pDetail->amount) << wxT(" silv (") << 100 * (pLand->current_state_.tax_.amount_ - pDetail->amount) / (pLand->current_state_.tax_.amount_+1) << wxT("% of $") << pLand->current_state_.tax_.amount_ << wxT(").") << wxString::FromUTF8(EOL_SCR);
        else
            OneLine << wxString::FromUTF8(EOL_SCR);

        pDetail->Details << OneLine.ToUTF8();
    }

    Factions.DeleteAll();
}

//-------------------------------------------------------------------------
bool CAhApp::GetTradeActivityDescription(CLand* land, std::map<int, std::vector<std::string>>& report)
{
    bool ret_val(false);
    //resources
    if (land->current_state_.produced_items_.size() > 0)
    {
        std::sort(land->current_state_.resources_.begin(), 
                  land->current_state_.resources_.end(), 
                  [](const CItem& it1, const CItem& it2){
            return it2.amount_ < it1.amount_;
        });

        std::set<long> involved_factions;
        land_control::perform_on_each_unit(land, [&](CUnit* unit) {
            auto ret = orders::control::retrieve_orders_by_type(orders::Type::O_PRODUCE, unit->orders_);
            if (ret.size() == 1)
            {
                std::string item;
                long amount;
                if (orders::parser::specific::parse_produce(ret[0], item, amount))
                    involved_factions.insert(unit->FactionId);
            }
        });

        for (const auto& resource : land->current_state_.resources_)
        {
            std::string reg_line = "    " + resource.code_name_ + " " + std::to_string(resource.amount_);

            auto produce_it = std::find_if(land->current_state_.produced_items_.begin(), 
                                           land->current_state_.produced_items_.end(), 
                                            [&](const std::pair<std::string, std::pair<long,long>>& item) {
                                        return resource.code_name_ == item.first;
                            });

            for (auto& factionId : involved_factions)
            {
                if (produce_it != land->current_state_.produced_items_.end())
                {
                    report[factionId].push_back(reg_line + " (requested: " + std::to_string(produce_it->second.second) + ")");
                }
                else 
                {
                    report[factionId].push_back(reg_line);
                }
            }
        }

        //products
        for (const auto& product : land->current_state_.produced_items_)
        {
            auto resource_it = std::find_if(land->current_state_.resources_.begin(), 
                                            land->current_state_.resources_.end(), 
                                            [&](const CItem& item) {
                                        return item.code_name_ == product.first;
                            });

            for (auto& factionId : involved_factions)
            {
                if (resource_it == land->current_state_.resources_.end())
                {
                    report[factionId].push_back("    produce: " + product.first + " " + std::to_string(product.second.second));
                }
            }
        }
        ret_val = true;
    }

    // compose building statistics
    std::vector<unit_control::UnitError> errors;
    std::vector<land_control::ActionUnit> build_orders;
    land_control::get_land_builders(land, build_orders, errors);
    for (auto& action : build_orders)
    {
        std::string temp = unit_control::compose_unit_name(action.unit_);
        report[action.unit_->FactionId].push_back("    " + temp + " " + action.description_);
        ret_val = true;
    }
    for (auto& error : errors)
    {
        std::string temp = unit_control::compose_unit_name(error.unit_);
        report[error.unit_->FactionId].push_back("    " + temp + " " + error.type_ + ": " + error.message_);
    }
    //trade items
    //TODO
    return ret_val;
}

//-------------------------------------------------------------------------

void CAhApp::CheckTaxTrade()
{
    //CStr                sTax(64);
    //CStr                sTrade(64);
    //CStr                Report(64), S(64);
    //CStr                Details(256);
//    long                tax = 0;
//    long                trade = 0;
    int                 n, i;
    CLand             * pLand;
    CPlane            * pPlane;


    //TODO: add possibility to make it for ANY chosen faction, currently most of the stats related to current faction

    struct FactionStats 
    {
        FactionStats() : trade_details_(), stats_() {};
        //long trade_regions_amount_;
        //long tax_regions_amount_;
        std::string trade_details_;
        //std::vector<std::string> stats_order_;
        std::map<std::string, std::map<std::string, long>> stats_;

        void add_stat(const std::string& category, const std::string& name, long val)
        {
            if (category.empty()) 
                stats_["General"][name] += val;
            else
                stats_[category][name] += val;
        }
    };
    std::map<long, FactionStats> output_stats;
    std::stringstream tax_description;

    for (n=0; n<m_pAtlantis->m_Planes.Count(); n++)
    {
        pPlane = (CPlane*)m_pAtlantis->m_Planes.At(n);
        for (i=0; i<pPlane->Lands.Count(); i++)
        {
            pLand = (CLand*)pPlane->Lands.At(i);

            //get tax amount
            std::set<long> scoreboard;//<factionId>, one for region
            land_control::perform_on_each_unit(pLand, [&](CUnit* unit) {

                if (scoreboard.find(unit->FactionId) != scoreboard.end())
                    return;

                scoreboard.insert(unit->FactionId);//once for each faction

                output_stats[unit->FactionId].add_stat("", "Taxing men", pLand->current_state_.tax_.requesters_amount_);
                output_stats[unit->FactionId].add_stat("", "Working men", pLand->current_state_.work_.requesters_amount_);
                output_stats[unit->FactionId].add_stat("", "Entertaining men", pLand->current_state_.entertain_.requesters_amount_);

                if (pLand->current_state_.tax_.requesters_amount_ > 0)
                    output_stats[unit->FactionId].add_stat("", "Taxing regions", 1);

                if (unit_control::of_player(unit))
                {
                    output_stats[unit->FactionId].add_stat("Economy", "01. Initial SILV", pLand->current_state_.economy_.initial_amount_);
                    output_stats[unit->FactionId].add_stat("Economy", "02. Claim", pLand->current_state_.economy_.claim_income_);
                    output_stats[unit->FactionId].add_stat("Economy", "03. Tax/Pillage", pLand->current_state_.economy_.tax_income_);
                    output_stats[unit->FactionId].add_stat("Economy", "04. Sell", pLand->current_state_.economy_.sell_income_);
                    output_stats[unit->FactionId].add_stat("Economy", "05. Buy", pLand->current_state_.economy_.buy_expenses_);
                    output_stats[unit->FactionId].add_stat("Economy", "06. Moving in", pLand->current_state_.economy_.moving_in_);
                    output_stats[unit->FactionId].add_stat("Economy", "07. Moving out", pLand->current_state_.economy_.moving_out_);
                    output_stats[unit->FactionId].add_stat("Economy", "08. Study", pLand->current_state_.economy_.study_expenses_);
                    output_stats[unit->FactionId].add_stat("Economy", "09. Work/Entertain", pLand->current_state_.economy_.work_income_);
                    output_stats[unit->FactionId].add_stat("Economy", "10. Maintenance", pLand->current_state_.economy_.maintenance_);
                    output_stats[unit->FactionId].add_stat("Economy", "11. Balance", pLand->current_state_.economy_.initial_amount_ +
                                                                                  pLand->current_state_.economy_.claim_income_ + 
                                                                                  pLand->current_state_.economy_.tax_income_ + 
                                                                                  pLand->current_state_.economy_.sell_income_ - 
                                                                                  pLand->current_state_.economy_.buy_expenses_ + 
                                                                                  pLand->current_state_.economy_.moving_in_ - 
                                                                                  pLand->current_state_.economy_.moving_out_ - 
                                                                                  pLand->current_state_.economy_.study_expenses_ + 
                                                                                  pLand->current_state_.economy_.work_income_ - 
                                                                                  pLand->current_state_.economy_.maintenance_);

                    //pLand->current_state_.produced_items_

                    std::vector<land_control::Trader> buyers;
                    std::vector<unit_control::UnitError> errors;
                    land_control::get_land_buys(pLand, buyers, errors);
                    for (const auto& buyer : buyers)
                    {
                        if (unit_control::of_player(buyer.unit_))
                        {
                            output_stats[unit->FactionId].add_stat("Buy", buyer.item_name_, buyer.items_amount_);
                            
                            if (item_control::is_men(buyer.item_name_))
                                output_stats[unit->FactionId].add_stat("Buy", "men", buyer.items_amount_);
                            if (item_control::is_trade(buyer.item_name_))
                                output_stats[unit->FactionId].add_stat("Buy", "trade items", buyer.items_amount_);
                        }
                    }

                    std::vector<land_control::ProduceItem> producers;
                    land_control::get_land_producers(pLand, producers, errors);
                    for (const auto& producer : producers)
                    {
                        long req_amount(0);
                        for (const auto& unit_pair : producer.units_)
                            if (unit_control::of_player(unit_pair.first))
                                req_amount += unit_pair.second;

                        if (producer.is_craft_) 
                            output_stats[unit->FactionId].add_stat("Craft", producer.item_name_, req_amount);
                        else
                        {
                            long land_amount = land_control::get_resource(pLand->current_state_, producer.item_name_);
                            output_stats[unit->FactionId].add_stat("Produce", producer.item_name_, std::min(land_amount, req_amount));
                        }
                    }                    
                }
            });

            std::map<int, std::vector<std::string>> reg_report;
            if (GetTradeActivityDescription(pLand, reg_report))
            {
                for (auto& fact_rep : reg_report)
                {
                    output_stats[fact_rep.first].add_stat("", "Production/Trade region", 1);
                    output_stats[fact_rep.first].trade_details_.append(land_control::land_full_name(pLand) + EOL_SCR);
                    for (auto& line : fact_rep.second)
                    {
                        output_stats[fact_rep.first].trade_details_.append(line + std::string(EOL_SCR));
                    }
                }
            }
        }
    }

    for (auto& faction_stat : output_stats)
    {
        std::string header = "Faction: " + std::to_string(faction_stat.first) + EOL_SCR;
        std::string trade_detailed = faction_stat.second.trade_details_ + EOL_SCR;

        ShowError(header.c_str(), header.size(), TRUE);

        std::vector<std::string> categories_order = {
            "General",
            "Economy",
            "Buy",
            "Produce",
            "Craft"
        };

        //known ordered categories
        for (auto& category : categories_order)
        {
            std::string temp = "    " + category + EOL_SCR;
            for (auto& stat : faction_stat.second.stats_[category])
            {
                temp += "        " + stat.first + " " + std::to_string(stat.second) + EOL_SCR;
            }
            ShowError(temp.c_str(), temp.size(), TRUE);
            faction_stat.second.stats_.erase(category);
        }

        //the rest
        for (auto& category : faction_stat.second.stats_)
        {
            std::string temp = "    " + category.first + EOL_SCR;
            for (auto& stat : category.second)
            {
                temp += "        " + stat.first + " " + std::to_string(stat.second) + EOL_SCR;
            }
            ShowError(temp.c_str(), temp.size(), TRUE);
        }        
        ShowError(trade_detailed.c_str(), trade_detailed.size(), TRUE);


    }
}

//-------------------------------------------------------------------------

void CAhApp::CheckProduction()
{
    int    n, i, x;
    CLand  * pLand;
    CPlane * pPlane;
    CUnit  * pUnit;
    CStr     Error(64), S(32);

    for (n=0; n<m_pAtlantis->m_Planes.Count(); n++)
    {
        pPlane = (CPlane*)m_pAtlantis->m_Planes.At(n);
        for (i=0; i<pPlane->Lands.Count(); i++)
        {
            pLand = (CLand*)pPlane->Lands.At(i);
            for (x=0; x<pLand->Units.Count(); x++)
            {
                pUnit = (CUnit*)pLand->Units.At(x);
                if (!m_pAtlantis->CheckResourcesForProduction(pUnit, pLand, S))
                    Error << "Unit " << pUnit->Id << " " << S << EOL_SCR;
            }
        }
    }

    S.Empty();
    if (Error.IsEmpty())
        wxMessageBox(wxT("No problem with resources for production detected"));
    else
    {
        S << "The following problems were detected:" << EOL_SCR << EOL_SCR << Error;
        ShowError(S.GetData(), S.GetLength(), TRUE);
    }
}

//--------------------------------------------------------------------------

void CAhApp::CheckSailing()
{
    CStr     Error(64), S(32), sCoord(32);

    for (int n=0; n<m_pAtlantis->m_Planes.Count(); n++)
    {
        CPlane* pPlane = (CPlane*)m_pAtlantis->m_Planes.At(n);
        for (int i=0; i<pPlane->Lands.Count(); i++)
        {
            CLand* land = (CLand*)pPlane->Lands.At(i);
            m_pAtlantis->ComposeLandStrCoord(land, sCoord);

            land_control::perform_on_each_struct(land, [&](CStruct* structure) {
                if (struct_control::flags::is_ship(structure) && 
                    structure->SailingPower > 0)
                {
                    if (structure->occupied_capacity_ > structure->capacity_)
                        Error << land->TerrainType << " (" << sCoord << ") - Ship " << structure->Id << " is overloaded by " << (structure->occupied_capacity_ - structure->capacity_) << "." << EOL_SCR;
                    if (structure->SailingPower < structure->MinSailingPower)
                        Error << land->TerrainType << " (" << sCoord << ") - Ship " << structure->Id << " is underpowered by " << (structure->MinSailingPower - structure->SailingPower) << "." << EOL_SCR;                    
                }
            });
        }
    }

    S.Empty();
    if (Error.IsEmpty())
        wxMessageBox(wxT("No problems with sailing detected"));
    else
    {
        S << "The following problems were detected:" << EOL_SCR << EOL_SCR << Error;
        ShowError(S.GetData(), S.GetLength(), TRUE);
    }
}

//--------------------------------------------------------------------------

#define SET_UNIT_PROP_NAME(_name, _type)                                 \
{                                                                        \
    CStrInt         * pSI, SI;                                           \
    int               k;                                                 \
    if (!m_pAtlantis->m_UnitPropertyNames.Search((void*)_name, k))       \
        m_pAtlantis->m_UnitPropertyNames.Insert(strdup(_name));          \
    SI.m_key = _name;                                                    \
    if (!m_pAtlantis->m_UnitPropertyTypes.Search(&SI, k))                \
    {                                                                    \
        pSI = new CStrInt(_name, _type);                                 \
        m_pAtlantis->m_UnitPropertyTypes.Insert(pSI);                    \
    }                                                                    \
    SI.m_key = NULL;                                                     \
}

void CAhApp::PreLoadReport()
{
    CStr S, FName;

    SaveLandFlags();
    SaveUnitFlags();
    if (m_CommentsChanged)
        SaveComments();
    if (orders_changed())
        SaveOrders(TRUE);

    if (ERR_OK==m_pAtlantis->m_ParseErr)
        SaveHistory(SZ_HISTORY_FILE);


}

//-------------------------------------------------------------------------

void CAhApp::PostLoadReport()
{
    CStr              S;
    CMapFrame       * pMapFrame  = (CMapFrame    *)m_Frames[AH_FRAME_MAP];
    CMapPane        * pMapPane   = (CMapPane     *)m_Panes [AH_PANE_MAP];
    CUnitPaneFltr   * pUnitPaneF = (CUnitPaneFltr*)m_Panes [AH_PANE_UNITS_FILTER];
    CUnitPane       * pUnitPane  = (CUnitPane    *)m_Panes [AH_PANE_UNITS_HEX];
    long              year, mon;
    const char      * szName;
    const char      * szValue;
    CPlane          * pPlane;
    CShortNamedObj  * pItem;
    CFaction          DummyFaction;
    CFaction        * pFaction;
    int               i, n;


    // update edge structures
    UpdateEdgeStructs();

    SaveLandFlags();

    // count number of our men in every hex

    m_pAtlantis->CountMenForTheFaction(m_pAtlantis->m_CrntFactionId);

    if (pMapFrame)
    {
        m_sTitle.Empty();

        for (i=0; i<m_pAtlantis->m_OurFactions.Count(); i++)
        {
            pFaction = m_pAtlantis->GetFaction((long)m_pAtlantis->m_OurFactions.At(i));
            if (pFaction)
            {
                if (!m_sTitle.IsEmpty())
                    m_sTitle << ", ";
                if (m_pAtlantis->m_OurFactions.Count()<3)
                    m_sTitle << pFaction->Name << " ";
                m_sTitle << (long)pFaction->Id;
            }
        }
        year = (long)(m_pAtlantis->m_YearMon/100);
        mon  = m_pAtlantis->m_YearMon % 100 - 1;
        if ( (mon >= 0) && (mon < 12) )
            m_sTitle << ". " << Monthes[mon] << " year " << year;
        SetMapFrameTitle();
    }

    // if loaded for the very first time, center it
    if (GetSectionFirst(SZ_SECT_REPORTS, szName, szValue) < 0)
    {
        wxCommandEvent event(wxEVT_COMMAND_TOOL_CLICKED, tool_centerout);

        if (m_Panes[AH_PANE_MAP])
            ((CMapPane*)m_Panes[AH_PANE_MAP])->OnToolbarCmd(event);
    }



    // stnadard unit and base properties - that is likely to be forgotten when
    // new properties are introduced :((
    SET_UNIT_PROP_NAME(PRP_COMMENTS          , eCharPtr)
    SET_UNIT_PROP_NAME(PRP_ORDERS            , eCharPtr)
    SET_UNIT_PROP_NAME(PRP_FACTION_ID        , eLong   )
    SET_UNIT_PROP_NAME(PRP_FACTION           , eCharPtr)
    SET_UNIT_PROP_NAME(PRP_LAND_ID           , eLong   )
    SET_UNIT_PROP_NAME(PRP_ID                , eLong   )
    SET_UNIT_PROP_NAME(PRP_NAME              , eCharPtr)
    SET_UNIT_PROP_NAME(PRP_FULL_TEXT         , eCharPtr)
    SET_UNIT_PROP_NAME(PRP_TEACHING          , eLong   )
    SET_UNIT_PROP_NAME(PRP_SEQUENCE          , eLong   )
    SET_UNIT_PROP_NAME(PRP_FRIEND_OR_FOE     , eLong   )
    SET_UNIT_PROP_NAME(PRP_WEIGHT            , eLong   )
    SET_UNIT_PROP_NAME(PRP_WEIGHT_WALK       , eLong   )
    SET_UNIT_PROP_NAME(PRP_WEIGHT_RIDE       , eLong   )
    SET_UNIT_PROP_NAME(PRP_WEIGHT_FLY        , eLong   )
    SET_UNIT_PROP_NAME(PRP_WEIGHT_SWIM       , eLong   )
    SET_UNIT_PROP_NAME(PRP_BEST_SKILL        , eLong   )
    SET_UNIT_PROP_NAME(PRP_BEST_SKILL_DAYS   , eLong   )
    SET_UNIT_PROP_NAME(PRP_DESCRIPTION       , eCharPtr)
    SET_UNIT_PROP_NAME(PRP_COMBAT            , eCharPtr)
    SET_UNIT_PROP_NAME(PRP_GUI_COLOR         , eLong   )
    SET_UNIT_PROP_NAME(PRP_MOVEMENT          , eCharPtr)
    SET_UNIT_PROP_NAME(PRP_FLAGS_STANDARD    , eCharPtr)
    SET_UNIT_PROP_NAME(PRP_FLAGS_CUSTOM      , eCharPtr)
    SET_UNIT_PROP_NAME(PRP_FLAGS_CUSTOM_ABBR , eCharPtr)

    LoadTerrainCostConfig();

    // If no orders loaded, no movement will be calculated. Force it.
    if (!m_pAtlantis->m_OrdersLoaded)
    {
        for (CUnit* pUnit : m_pAtlantis->units_)
        {
            pUnit->ResetNormalProperties();
        }

        for (n=0; n<m_pAtlantis->m_Planes.Count(); n++)
        {
            pPlane = (CPlane*)m_pAtlantis->m_Planes.At(n);
            for (i=0; i<pPlane->Lands.Count(); i++)
            {
                CLand* land = (CLand*)pPlane->Lands.At(i);
                land_control::structures::update_struct_weights(land);
                land->SetFlagsFromUnits(); // maybe not needed here...
            }
        }
    }

    // skills
    for (i=0; i<m_pAtlantis->m_Skills.Count(); i++)
    {
        pItem = (CShortNamedObj*)m_pAtlantis->m_Skills.At(i);


        EncodeConfigLine(S, pItem->Description.GetData());
        SetConfig(SZ_SECT_SKILLS, pItem->Name.GetData(), S.GetData());
    }

    // Items
    for (i=0; i<m_pAtlantis->m_Items.Count(); i++)
    {
        pItem = (CShortNamedObj*)m_pAtlantis->m_Items.At(i);

        EncodeConfigLine(S, pItem->Description.GetData());
        SetConfig(SZ_SECT_ITEMS, pItem->Name.GetData(), S.GetData());
    }

    // Objects
    for (i=0; i<m_pAtlantis->m_Objects.Count(); i++)
    {
        pItem = (CShortNamedObj*)m_pAtlantis->m_Objects.At(i);

        EncodeConfigLine(S, pItem->Description.GetData());
        SetConfig(SZ_SECT_OBJECTS, pItem->Name.GetData(), S.GetData());
    }

    if (pMapPane)
        pMapPane->Refresh(FALSE, NULL);


    if (pUnitPane)
        pUnitPane->m_pCurLand = NULL; // force the unit pane to do full update
    OnMapSelectionChange();

    // if there were Hex Events, show them
    if (!m_pAtlantis->m_HexEvents.Description.IsEmpty())
    {
        CBaseColl   Coll;
        Coll.Insert(&m_pAtlantis->m_HexEvents);
        ShowDescriptionList(Coll, "Hex Events");
    }

    if (pUnitPaneF)
        pUnitPaneF->Update(NULL);

    CheckRedirectedOutputFiles();

    if (!m_pAtlantis->m_SecurityEvents.Description.IsEmpty())
        m_pAtlantis->m_SecurityEvents.Description << EOL_SCR << EOL_SCR;
}

//-------------------------------------------------------------------------

int  CAhApp::LoadReport  (const char * FNameIn, BOOL Join)
{
    CStr S, Sect, S2;
    CStr FName;
    int  LoadOrd;
    int  i;
    long n;
    int  err = ERR_FOPEN;

    wxBeginBusyCursor();

    m_DisableErrs = TRUE;

    if (FNameIn && *FNameIn)
    {
        FName = FNameIn;
        FName.TrimRight(TRIM_ALL);

        PreLoadReport();

        if (!m_FirstLoad && !Join)
        {
            m_pAtlantis = new CAtlaParser(&ThisGameDataHelper);
            LoadTerrainCostConfig();
        }

        if (!Join)
        {
            m_pAtlantis->Clear();
            m_pAtlantis->ParseRep(SZ_HISTORY_FILE, FALSE, TRUE);
        }

        // Append unit group property names here so they are available while parsing
        for (i=0; i<m_UnitPropertyGroups.Count(); i++ )
        {
            CStrStr * pSS = (CStrStr*)m_UnitPropertyGroups.At(i);
            SET_UNIT_PROP_NAME(pSS->m_key, eLong)
        }


        err = m_pAtlantis->ParseRep(FName.GetData(), Join, FALSE);
        switch (err)
        {
            case ERR_INV_TURN:
                wxMessageBox(wxT("Wrong turn in the report"), wxT("Error"));
                break;
        }
        orders_changed(false);
        m_CommentsChanged = FALSE;
        if ( ERR_OK==err && m_pAtlantis->m_YearMon != 0 && m_pAtlantis->m_CrntFactionId != 0 )
        {
            m_ReportDates.Insert((void*)m_pAtlantis->m_YearMon);
            //UpgradeConfigByFactionId();

            if (atol(GetConfig(SZ_SECT_COMMON, SZ_KEY_PWD_READ)) && !m_pAtlantis->m_CrntFactionPwd.IsEmpty())
            {
                S.Empty();
                S << (long)m_pAtlantis->m_CrntFactionId;
                SetConfig(SZ_SECT_PASSWORDS, S.GetData(), m_pAtlantis->m_CrntFactionPwd.GetData() );
            }

            LoadOrd = atol(GetConfig(SZ_SECT_COMMON, SZ_KEY_LOAD_ORDER));
            if (LoadOrd)
            {
                S.Empty();
                S << (long)m_pAtlantis->m_YearMon;
                ComposeConfigOrdersSection(Sect, m_pAtlantis->m_CrntFactionId);
                LoadOrders(GetConfig(Sect.GetData(), S.GetData()));
            }
        }

        LoadComments();
        LoadLandFlags();
        LoadUnitFlags();
        PostLoadReport();

        if ( (ERR_OK==err) && (m_pAtlantis->m_YearMon != 0) )
        {
            // doing it after PostLoadReport() since it will check the section
            S.Empty();
            S << (long)m_pAtlantis->m_YearMon;
            if (!Join)
                SetConfig(SZ_SECT_REPORTS, S.GetData(), FName.GetData());
            else
            {
                S2 = GetConfig(SZ_SECT_REPORTS, S.GetData());
                if (!S2.IsEmpty())
                    S2 << ", ";
                S2 << FName;
                SetConfig(SZ_SECT_REPORTS, S.GetData(), S2.GetData());
            }
        }

        if (!m_FirstLoad && !Join)
        {
            if (m_Reports.Search(m_pAtlantis, i))
                m_Reports.AtFree(i);
            m_Reports.Insert(m_pAtlantis);

            n = atol(GetConfig(SZ_SECT_COMMON, SZ_KEY_REP_CACHE_COUNT));
            if (n<=0)
                n = 1;
            if (m_Reports.Count()>n)
            {
                if (i > n/2)
                    n = 0;
                else
                    n = m_Reports.Count()-1;
                if (m_pAtlantis != m_Reports.At(n))
                    m_Reports.AtFree(n);
            }
        }
        m_FirstLoad = FALSE;
    }

    m_DisableErrs = FALSE;

    wxEndBusyCursor();

    return err;
}

//-------------------------------------------------------------------------

int  CAhApp::LoadReport(BOOL Join)
{
    int rc;
    CStr Dir;
    const char * key;

    key = Join ? SZ_KEY_FOLDER_REP_JOIN : SZ_KEY_FOLDER_REP_LOAD;
    Dir = GetConfig(SZ_SECT_FOLDERS, key);
    if (Dir.IsEmpty())
        Dir = ".";

    wxString CurrentDir = wxGetCwd();
    wxFileDialog dialog(m_Frames[AH_FRAME_MAP],
                        wxT("Load Report"),
                        wxString::FromAscii(Dir.GetData()),
                        wxT(""),
                        wxT(SZ_REP_FILES),
                        wxFD_OPEN);
    rc = dialog.ShowModal();
    wxSetWorkingDirectory(CurrentDir);

    if (wxID_OK == rc)
    {
        CStr S;
        S = dialog.GetPath().mb_str();
        MakePathRelative(CurrentDir.mb_str(), S);

        GetDirFromPath(S.GetData(), Dir);
        SetConfig(SZ_SECT_FOLDERS, key, Dir.GetData() );

        return LoadReport(S.GetData(), Join);
    }
    else
        return ERR_CANCEL;

}

//-------------------------------------------------------------------------

void CAhApp::SelectTempUnit(CUnit * pUnit)
{
    CEditPane   * pDescription = (CEditPane*)m_Panes[AH_PANE_UNIT_DESCR   ];
    CUnitOrderEditPane   * pOrders      = (CUnitOrderEditPane*)m_Panes[AH_PANE_UNIT_COMMANDS];
    CEditPane   * pComments    = (CEditPane*)m_Panes[AH_PANE_UNIT_COMMENTS];

    OnUnitHexSelectionChange(); // unselect ?? -- not unselecting
    m_UnitDescrSrc.Empty();

    if (pUnit)
        m_UnitDescrSrc = pUnit->Description;

    if (pDescription)
        pDescription->SetSource(&m_UnitDescrSrc, NULL);
    if (pOrders)
    {
        pOrders->change_representing_unit(NULL);
        pOrders->SetReadOnly(TRUE);
        pOrders->ApplyFonts();
    }
    if (pComments)
    {
        pComments->SetSource(NULL, NULL);
//        pComments->SetReadOnly ( TRUE );
    }
}

//-------------------------------------------------------------------------

void CAhApp::SelectUnit(CUnit * pUnit)
{
    CMapPane    * pMapPane  = (CMapPane* )gpApp->m_Panes[AH_PANE_MAP];
    CUnitPane   * pUnitPane = (CUnitPane*)gpApp->m_Panes[AH_PANE_UNITS_HEX];
    CLand       * pLand;
    CPlane      * pPlane;
    int           nx, ny, nz;
    BOOL          refresh;
    BOOL          NeedSetUnit;

    if (!pUnit || !pMapPane)
        return;
    pLand = m_pAtlantis->GetLand(pUnit->LandId);
    if (!pLand)
        return;

    //pLand->guiUnit = pUnit->Id;

    LandIdToCoord(pLand->Id, nx, ny, nz);
    pPlane   = (CPlane*)m_pAtlantis->m_Planes.At(nz);

    refresh = pMapPane->EnsureLandVisible(nx, ny, nz, FALSE);
    if (refresh)
        pMapPane->Refresh(FALSE);

    NeedSetUnit = (pUnitPane && (pLand==pUnitPane->m_pCurLand));

    pMapPane->SetSelection(nx, ny, pUnit, pPlane, TRUE);

    if (pUnit->Flags & UNIT_FLAG_TEMP)
    {
        pUnitPane->SelectUnit(-1);
        SelectTempUnit(pUnit);  // just redraw description
    }
    else
        if (NeedSetUnit)
            pUnitPane->SelectUnit(pUnit->Id); // otherwise will be already selected
}

//-------------------------------------------------------------------------

void CAhApp::SelectLand(CLand * pLand)
{
    CMapPane    * pMapPane  = (CMapPane* )gpApp->m_Panes[AH_PANE_MAP];
    CUnitPane   * pUnitPane = (CUnitPane*)gpApp->m_Panes[AH_PANE_UNITS_HEX];
    CPlane      * pPlane;
    int           nx, ny, nz;
    BOOL          refresh;

    if (pLand)
    {
        LandIdToCoord(pLand->Id, nx, ny, nz);
        pPlane   = (CPlane*)gpApp->m_pAtlantis->m_Planes.At(nz);

        refresh = pMapPane->EnsureLandVisible(nx, ny, nz, TRUE);
        if (refresh)
            pMapPane->Refresh(FALSE);

	if ((m_layout == AH_LAYOUT_1_WIN_ONE_DESCR) ||
            (!pUnitPane || pLand != pUnitPane->m_pCurLand))
        pMapPane->SetSelection(nx, ny, NULL, pPlane, TRUE);
    }
}

//-------------------------------------------------------------------------

BOOL CAhApp::SelectLand(const char * landcoords) //  "48,52[,somewhere]"
{
    CLand       * pLand     = m_pAtlantis->GetLand(landcoords);

    if (pLand)
    {
        SelectLand(pLand);
        return TRUE;
    }
    else
        return FALSE;
}

//-------------------------------------------------------------------------

void CAhApp::EditPaneDClicked(CEditPane * pPane)
{
    const char  * p;
    CStr          src, S;
    char          ch;
    CUnit       * pUnit;
    CBaseObject   Dummy;
    int           idx;
    long          position;


    if (pPane == m_Panes[AH_PANE_MSG] || pPane == m_Panes[AH_PANE_MAP_DESCR])
    {
        pPane->GetValue(src);
        position = pPane->m_pEditor->GetInsertionPoint();

// There is a bug in win32 GetInsertionPoint() - returned value corresponds to "\r\n" end of lines,
// while actual returned string has "\n" end of lines
#ifdef __WXMSW__
        long x = 0;
        p = src.GetData();
        while (x<position)
        {
            if ('\n' == p[x])
                position--;
            x++;
        }
#endif
        if (position > src.GetLength())
            position = src.GetLength();

        p = src.GetData();
        while (position > 0)

        {
            if (p[position-1]=='\n')
                break;
            position--;
        }

        p = &src.GetData()[position];
        p = SkipSpaces(S.GetToken(p, " \t", ch, TRIM_ALL));
        if (0==stricmp("UNIT", S.GetData()))  // that is an order problem report
        {
            /*long unitId = atol(S.GetData());
            if (m_pAtlantis->units_ids_.find(unitId) != m_pAtlantis->units_ids_.end())
            {
                SelectUnit(m_pAtlantis->units_[units_ids_[unitId]]);
                return;
            }*/

            S.GetToken(p, " \t", ch, TRIM_ALL);
            Dummy.Id = atol(S.GetData());
            if (m_pAtlantis->m_Units.Search(&Dummy, idx))
            {
                pUnit = (CUnit*)m_pAtlantis->m_Units.At(idx);
                SelectUnit(pUnit);
                return;
            }
        }


        p = &src.GetData()[position];
        p = SkipSpaces(S.GetToken(p, "(\n", ch, TRIM_ALL)); // must be an error from the report file
        if ('('==ch)
        {
            S.GetToken(p, ",)\n", ch, TRIM_ALL);
            Dummy.Id = atol(S.GetData());
            if (')'==ch && m_pAtlantis->m_Units.Search(&Dummy, idx))
            {
                pUnit = (CUnit*)m_pAtlantis->m_Units.At(idx);
                SelectUnit(pUnit);
                return;
            }
        }

        // land (5,1,2 <underworld>)
        p = &src.GetData()[position];
        p = SkipSpaces(S.GetToken(p, "(\n", ch, TRIM_ALL));
        if ('('==ch)
        {
            p = SkipSpaces(S.GetToken(p, ")\n", ch, TRIM_ALL));
            if (')' == ch)
            {
                // Try to parse a unit number as well: (5,7) NEW 1 (2883585) Warning
                CStr U;
                CLand * pLand = m_pAtlantis->GetLand(S.GetData());
                p = SkipSpaces(U.GetToken(p, "(\n", ch, TRIM_ALL));
                if ('('==ch)
                {
                    U.GetToken(p, ",)\n", ch, TRIM_ALL);
                    if (')' == ch)
                    {
                        Dummy.Id = atol(U.GetData());
                        if (pLand->Units.Search(&Dummy, idx))
                        {
                            pUnit = (CUnit*)pLand->Units.At(idx);
                            SelectUnit(pUnit);
                            return;
                        }
                    }
                }
                if (SelectLand(S.GetData()))
                    return;
            }
        }
    }
}

//-------------------------------------------------------------------------

void CAhApp::SwitchToYearMon(long YearMon)
{
    char          Dummy[sizeof(CAtlaParser)];
    CAtlaParser * pDummy  = (CAtlaParser *)Dummy;
    int           i;
    CStr          S, S2;

    PreLoadReport();
    if (orders_changed())
        return;
    pDummy->m_YearMon = YearMon;
    if (m_Reports.Search(pDummy, i))
    {
        m_pAtlantis = (CAtlaParser *)m_Reports.At(i);
        PostLoadReport();
    }
    else
    {
        S.Empty();
        S << YearMon;

        S2 = GetConfig(SZ_SECT_REPORTS, S.GetData());
        const char * p = S2.GetData();
        BOOL         join = FALSE;
        while (p && *p)
        {
            p = S.GetToken(p, ',');
            LoadReport(S.GetData(), join);
            join = TRUE;
        }
    }
}

//-------------------------------------------------------------------------

void CAhApp::SwitchToRep(eRepSeq whichrep)
{
    int  i;

    m_DisableErrs = TRUE;

    if (CanSwitchToRep(whichrep, i))
        SwitchToYearMon((long)m_ReportDates.At(i));

    m_DisableErrs = FALSE;
}

//-------------------------------------------------------------------------

BOOL CAhApp::CanSwitchToRep(eRepSeq whichrep, int & RepIdx)
{
    long       ym;
    CStr       sName, sData;
    CLand    * pLand;
    CMapPane * pMapPane;

    RepIdx=-1;

    switch(whichrep)
    {
    case repFirst:
        RepIdx = 0;
        break;

    case repLast:
        if (m_pAtlantis->m_YearMon == (long)m_ReportDates.At(gpApp->m_ReportDates.Count()-1) )
            RepIdx = -1;
        else
            RepIdx = m_ReportDates.Count()-1;
        break;

    case repPrev:
        if (m_ReportDates.Search((void*)m_pAtlantis->m_YearMon, RepIdx) )
            RepIdx--;
        break;

    case repNext:
        if (m_ReportDates.Search((void*)m_pAtlantis->m_YearMon, RepIdx) )
            RepIdx++;
        break;

    case repLastVisited:
        pMapPane = (CMapPane* )m_Panes[AH_PANE_MAP];
        pLand    = m_pAtlantis->GetLand(pMapPane->m_SelHexX, pMapPane->m_SelHexY, pMapPane->m_SelPlane, TRUE);
        m_pAtlantis->ComposeLandStrCoord(pLand, sName);
//        ym       = atol(GetConfig(SZ_SECT_LAND_VISITED, sName.GetData()));
        sData.GetToken(GetConfig(SZ_SECT_LAND_VISITED, sName.GetData()), ',');
        ym = atol(sData.GetData());


        if (ym==m_pAtlantis->m_YearMon || !m_ReportDates.Search((void*)ym, RepIdx))
            RepIdx = -1;
        break;
    }

    return (RepIdx>=0 && RepIdx<m_ReportDates.Count());
}

//-------------------------------------------------------------------------

BOOL CAhApp::GetPrevTurnReport(CAtlaParser *& pPrevTurn)
{
    int idx;

    pPrevTurn = NULL;

    if (CanSwitchToRep(repPrev, idx))
    {
        char          Dummy[sizeof(CAtlaParser)];
        CAtlaParser * pDummy  = (CAtlaParser *)Dummy;
        int           i;
        CStr          S, S2;

        long YearMon = (long)m_ReportDates.At(idx);

        pDummy->m_YearMon = YearMon;
        if (m_Reports.Search(pDummy, i))
        {
            pPrevTurn = (CAtlaParser *)m_Reports.At(i);
        }
        else
        {
            S.Empty();
            S << YearMon;

            S2 = GetConfig(SZ_SECT_REPORTS, S.GetData());
            const char * p = S2.GetData();
            BOOL         join = FALSE;
            m_DisableErrs = TRUE;
            wxBeginBusyCursor();
            pPrevTurn = new CAtlaParser(&ThisGameDataHelper);
            pPrevTurn->ParseRep(SZ_HISTORY_FILE, FALSE, TRUE);
            while (p && *p)
            {
                p = S.GetToken(p, ',');
                //LoadReport(S.GetData(), join);
                pPrevTurn->ParseRep(S.GetData(), join, FALSE);
                join = TRUE;
            }
            wxEndBusyCursor();
            m_DisableErrs = FALSE;
            if (pPrevTurn->m_YearMon == YearMon)
                m_Reports.Insert(pPrevTurn);
            else
            {
                delete pPrevTurn;
                pPrevTurn = NULL;
            }
        }
    }

    return (pPrevTurn != NULL);
}

/*
    CStr S, Sect, S2;
    CStr FName;
    int  LoadOrd;
    int  i;
    long n;
    int  err = ERR_FOPEN;

    wxBeginBusyCursor();

    m_DisableErrs = TRUE;

    if (FNameIn && *FNameIn)
    {
        FName = FNameIn;
        FName.TrimRight(TRIM_ALL);

        PreLoadReport();

        if (!m_FirstLoad && !Join)
            m_pAtlantis = new CAtlaParser(&ThisGameDataHelper);

        if (!Join)
        {
            m_pAtlantis->Clear();
            m_pAtlantis->ParseRep(SZ_HISTORY_FILE, FALSE, TRUE);
        }

        // Append unit group property names here so they are available while parsing
        for (i=0; i<m_UnitPropertyGroups.Count(); i++ )
        {
            CStrStr * pSS = (CStrStr*)m_UnitPropertyGroups.At(i);
            SET_UNIT_PROP_NAME(pSS->m_key, eLong)
        }


        err = m_pAtlantis->ParseRep(FName.GetData(), Join, FALSE);
        switch (err)
        {
            case ERR_INV_TURN:
                wxMessageBox("Wrong turn in the report", "Error");
                break;
        }
        SetOrdersChanged(FALSE);
        m_CommentsChanged = FALSE;
        if ( ERR_OK==err && m_pAtlantis->m_YearMon != 0 && m_pAtlantis->m_CrntFactionId != 0 )
        {
            m_ReportDates.Insert((void*)m_pAtlantis->m_YearMon);
            UpgradeConfigByFactionId();

            if (atol(GetConfig(SZ_SECT_COMMON, SZ_KEY_PWD_READ)) && !m_pAtlantis->m_CrntFactionPwd.IsEmpty())
            {
                S.Empty();
                S << (long)m_pAtlantis->m_CrntFactionId;
                SetConfig(SZ_SECT_PASSWORDS, S.GetData(), m_pAtlantis->m_CrntFactionPwd.GetData() );
            }

            LoadOrd = atol(GetConfig(SZ_SECT_COMMON, SZ_KEY_LOAD_ORDER));
            if (LoadOrd)
            {
                S.Empty();
                S << (long)m_pAtlantis->m_YearMon;
                ComposeConfigOrdersSection(Sect, m_pAtlantis->m_CrntFactionId);
                LoadOrders(GetConfig(Sect.GetData(), S.GetData()));
            }
        }

        LoadComments();
        LoadLandFlags();
        LoadUnitFlags();
        PostLoadReport();

        if ( (ERR_OK==err) && (m_pAtlantis->m_YearMon != 0) )
        {
            // doing it after PostLoadReport() since it will check the section
            S.Empty();
            S << (long)m_pAtlantis->m_YearMon;
            if (!Join)
                SetConfig(SZ_SECT_REPORTS, S.GetData(), FName.GetData());
            else
            {
                S2 = GetConfig(SZ_SECT_REPORTS, S.GetData());
                if (!S2.IsEmpty())
                    S2 << ", ";
                S2 << FName;
                SetConfig(SZ_SECT_REPORTS, S.GetData(), S2.GetData());
            }
        }

        if (!m_FirstLoad && !Join)
        {
            if (m_Reports.Search(m_pAtlantis, i))
                m_Reports.AtFree(i);
            m_Reports.Insert(m_pAtlantis);

            n = atol(GetConfig(SZ_SECT_COMMON, SZ_KEY_REP_CACHE_COUNT));
            if (n<=0)
                n = 1;
            if (m_Reports.Count()>n)
            {
                if (i > n/2)
                    n = 0;
                else
                    n = m_Reports.Count()-1;
                if (m_pAtlantis != m_Reports.At(n))
                    m_Reports.AtFree(n);
            }
        }
        m_FirstLoad = FALSE;
    }

    m_DisableErrs = FALSE;

    wxEndBusyCursor();
*/


//-------------------------------------------------------------------------




std::string CAhApp::land_description_editpane(CLand * pLand)
{
    if (pLand == nullptr)
        return "";

    //load full region description
    std::string reg_description(pLand->Description.GetData(), pLand->Description.GetLength());


    if (pLand->current_state_.tax_.requesters_amount_)
    {
        size_t ins_pos = reg_description.find("-----------------------------");
        while(ins_pos != std::string::npos && ins_pos > 0 && reg_description[ins_pos] != '.')
            --ins_pos;

        long tax_per_man = game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_KEY_TAX_PER_TAXER);
        std::string tax_insertion = " (" + std::to_string(pLand->current_state_.tax_.requesters_amount_) + 
                    "/" + std::to_string((pLand->current_state_.tax_.amount_-1)/tax_per_man +1) + ")";
        reg_description.insert(ins_pos, tax_insertion);
    }

    if (pLand->current_state_.work_.requesters_amount_)
    {
        size_t ins_pos = reg_description.find("Wages: ");
        ins_pos = reg_description.find('.', ins_pos);
        if (std::isdigit(reg_description[ins_pos+1]))//if next symbol is digit, we are in a float wages
            ins_pos = reg_description.find('.', ins_pos+1);

        std::string work_insertion = " " + std::to_string(pLand->current_state_.work_.requesters_amount_) + 
                    "/" + std::to_string(long((pLand->current_state_.work_.amount_-1)/pLand->Wages)+1);
        reg_description.insert(ins_pos, work_insertion);
    }   

    if (pLand->current_state_.entertain_.requesters_amount_)
    {
        size_t ins_pos = reg_description.find("Entertainment available: ");
        if (ins_pos != std::string::npos)
        {
            ins_pos = reg_description.find('.', ins_pos);

            long ente_per_man = game_control::get_game_config_val<long>(SZ_SECT_COMMON, SZ_KEY_ENTERTAINMENT_SILVER);
            std::string entertain_insertion = " (" + std::to_string(pLand->current_state_.entertain_.requesters_amount_) + 
                        "/" + std::to_string(long((pLand->current_state_.entertain_.amount_-1)/ente_per_man)+1) + ")";
            reg_description.insert(ins_pos, entertain_insertion);
        }
    }                       

    if (pLand->current_state_.sold_items_.size() > 0)
    {
        size_t beg_pos = reg_description.find("Wanted: ");
        size_t end_pos = reg_description.find('.', beg_pos);
        for (const auto& pair : pLand->current_state_.sold_items_)
        {
            auto it = std::search(reg_description.begin()+beg_pos, reg_description.begin()+end_pos, 
                      pair.first.begin(), pair.first.end());
            if (it != reg_description.begin()+beg_pos)
            {
                size_t ins_pos = it - reg_description.begin();
                while (ins_pos > beg_pos && !std::isdigit(reg_description[ins_pos]))
                    --ins_pos;
                ++ins_pos;
                
                std::string buy_insertion = " (-"+std::to_string(pair.second)+")";
                reg_description.insert(ins_pos, buy_insertion);
            }
        }
    }

    if (pLand->current_state_.bought_items_.size() > 0)
    {
        size_t beg_pos = reg_description.find("For Sale: ");
        size_t end_pos = reg_description.find('.', beg_pos);
        for (const auto& pair : pLand->current_state_.bought_items_)
        {
            auto it = std::search(reg_description.begin()+beg_pos, reg_description.begin()+end_pos, 
                      pair.first.begin(), pair.first.end());
            if (it != reg_description.begin()+beg_pos)
            {
                size_t ins_pos = it - reg_description.begin();
                while (ins_pos > beg_pos && !std::isdigit(reg_description[ins_pos]))
                    --ins_pos;
                ++ins_pos;
                
                std::string sell_insertion = " (-"+std::to_string(pair.second)+")";
                reg_description.insert(ins_pos, sell_insertion);
            }
        }
    }            

    reg_description.append(EOL_SCR);

    std::stringstream ss;
    m_pAtlantis->compose_products_detailed(pLand, ss);
    reg_description.append(ss.str());

    long final_amount =  pLand->current_state_.economy_.initial_amount_ +
                          pLand->current_state_.economy_.tax_income_ +
                          pLand->current_state_.economy_.claim_income_ +
                          pLand->current_state_.economy_.sell_income_ + 
                          pLand->current_state_.economy_.work_income_ +
                            std::max(pLand->current_state_.economy_.moving_in_, (long)0) -
                          pLand->current_state_.economy_.buy_expenses_ -
                          pLand->current_state_.economy_.maintenance_ -
                            std::max(pLand->current_state_.economy_.moving_out_, (long)0) - 
                          pLand->current_state_.economy_.study_expenses_;

    reg_description.append(EOL_SCR);
    reg_description += "Silver balance: [" + std::to_string(pLand->current_state_.economy_.initial_amount_)
                    + "/" + std::to_string(final_amount) + "]" + EOL_SCR;

    if (pLand->current_state_.structures_.size()>0)
    {
        reg_description.append(EOL_SCR);
        reg_description.append("-----------");
        reg_description.append(EOL_SCR);
    }

    //static building part
    bool exists_static_building = false;
    land_control::perform_on_each_struct(pLand, [&](CStruct* structure) {
        if (!struct_control::flags::is_ship(structure)) {
            // separation between buildings and ships
            reg_description.append(structure->original_description_);
            trim_inplace(reg_description);
            reg_description.append(EOL_SCR);
            exists_static_building = true;
        }
    });

    //separation for ships if static buildings exist
    if (exists_static_building)
        reg_description.append(EOL_SCR);

    // ships part
    land_control::perform_on_each_struct(pLand, [&](CStruct* structure) {
        if (struct_control::flags::is_ship(structure)) {
            reg_description.append(structure->original_description_);
            trim_inplace(reg_description);
            reg_description += " Forecast: Load[" + std::to_string(structure->occupied_capacity_) 
                            + "/" + std::to_string(structure->capacity_) + "], Power[" 
                            + std::to_string(structure->SailingPower) + "/" 
                            + std::to_string(structure->MinSailingPower) + "].";
            reg_description.append(EOL_SCR);
        }
    });
    
    bool FlagsEmpty(true);
    for (long i=0; i<LAND_FLAG_COUNT; i++)
        if (!pLand->FlagText[i].IsEmpty())
        {
            FlagsEmpty = false;
            break;
        }

    if (!FlagsEmpty)
    {
        reg_description.append(EOL_SCR);
        reg_description += "-----------";

        for (long i=0; i<LAND_FLAG_COUNT; i++)
            if (!pLand->FlagText[i].IsEmpty())
            {
                reg_description.append(EOL_SCR); 
                reg_description.append(pLand->FlagText[i].GetData());
            }
              
    }

    //events/exits
    if (!pLand->Events.IsEmpty() &&
          0 != stricmp(SkipSpaces(pLand->Events.GetData()), "none"))
    {
        reg_description.append(EOL_SCR); 
        reg_description.append("Events:");
        reg_description.append(EOL_SCR); 
        reg_description.append(pLand->Events.GetData());
        reg_description.append(EOL_SCR); 
        
    }
    reg_description.append(EOL_SCR); 
    reg_description.append("Exits:"); 
    reg_description.append(EOL_SCR); 
    reg_description.append(pLand->Exits.GetData());

    //errors
    if (pLand->current_state_.run_orders_errors_.size() > 0)
    {
        reg_description.append(EOL_SCR); 
        reg_description.append("Errors:"); 
        reg_description.append(EOL_SCR);
    }
    for (const auto& error : pLand->current_state_.run_orders_errors_)
    {
        reg_description += "    " + unit_control::compose_unit_name(error.unit_) + error.message_;
        reg_description.append(EOL_SCR);
    }
    return reg_description;
}

//-------------------------------------------------------------------------
/*
void CAhApp::UpdateUnitPane(CLand * pLand)
{
    CUnitPane   * pUnitPane = (CUnitPane*)m_Panes[AH_PANE_UNITS_HEX];

    if (pLand != nullptr)
        m_pAtlantis->RunOrders(pLand);

    if (pUnitPane)
        pUnitPane->Updaaaate(pLand);

}*/

//-------------------------------------------------------------------------

void CAhApp::OnMapSelectionChange()
{
    CLand*        land              = NULL;
    CMapPane*     map_pane          = (CMapPane* )m_Panes[AH_PANE_MAP];
    CUnitPane*    unit_pane         = (CUnitPane*)m_Panes[AH_PANE_UNITS_HEX];
    CEditPane*    description_pane  = (CEditPane*)m_Panes[AH_PANE_MAP_DESCR];

    if (map_pane)
        land = m_pAtlantis->GetLand(map_pane->m_SelHexX, map_pane->m_SelHexY, map_pane->m_SelPlane, TRUE);

    if (land != nullptr)
        m_pAtlantis->RunOrders(land);

    if (unit_pane)
    {
        unit_pane->is_filtered_ = false;//selection on map should discard filters
        unit_pane->Update(land);
    }        

    if (description_pane)
    {
        m_HexDescrSrc.Empty();
        std::string description = land_description_editpane(land);  // NULL is Ok!
        m_HexDescrSrc << description.c_str();
        description_pane->SetHeader("Hex description");
        description_pane->SetSource(&m_HexDescrSrc, NULL);
    }    
    SetMapFrameTitle();
}

//-------------------------------------------------------------------------


bool CAhApp::OnUnitHexSelectionChange()
{
    // It can be called as a result of selecting a hex on the map!

    // It will be unit in the current hex!
    BOOL          ReadOnly = TRUE;
    CEditPane   * pDescription;
    CUnitOrderEditPane   * pOrders;
    CEditPane   * pComments;
    CUnit       * pUnit;
    bool return_value = false;//! signal that order was manually modified

    //m_SelUnitIdx = idx;
    pUnit        = GetSelectedUnit(); // depends on m_SelUnitIdx

    pDescription = (CEditPane*)m_Panes[AH_PANE_UNIT_DESCR   ];
    pOrders      = (CUnitOrderEditPane*)m_Panes[AH_PANE_UNIT_COMMANDS];
    pComments    = (CEditPane*)m_Panes[AH_PANE_UNIT_COMMENTS];

    m_UnitDescrSrc.Empty();

    if (pUnit)
    {//
        ReadOnly = !pUnit->IsOurs || pUnit->Id<=0 || 
                   (m_pAtlantis->m_YearMon != (long)m_ReportDates.At(gpApp->m_ReportDates.Count()-1) );
        
        if (pDescription) 
        {//update description
            m_UnitDescrSrc << unit_control::get_initial_description(pUnit).c_str() << EOL_SCR;

            for (const std::string& impact_descr : pUnit->impact_description_)
                m_UnitDescrSrc << ";" << impact_descr.c_str() << "." << EOL_SCR;
            m_UnitDescrSrc << EOL_SCR;

            m_UnitDescrSrc << unit_control::get_actual_description(pUnit).c_str() << EOL_SCR;

            if (!IS_NEW_UNIT(pUnit)) 
            {
                if (!pUnit->Errors.IsEmpty())
                    m_UnitDescrSrc << " ***** Errors:" << EOL_SCR << pUnit->Errors;
                if (!pUnit->Events.IsEmpty())
                    m_UnitDescrSrc << " ----- Events:" << EOL_SCR << pUnit->Events;
            }
            
            pDescription->SetHeader("Unit description");
            pDescription->SetSource(&m_UnitDescrSrc, NULL);
        }
    }

    if (pOrders)
    {//update orders
        CLand* prev_unit_land(NULL);
        CUnit* prev_unit = pOrders->change_representing_unit(pUnit);
        if (prev_unit != NULL && prev_unit->orders_.is_modified_)//add here ORDER_MODIFIED check
        {
            return_value = true;
            prev_unit->orders_.is_modified_ = false;
            prev_unit_land = gpApp->m_pAtlantis->GetLand(prev_unit->LandId);
            if (prev_unit_land)
                gpApp->m_pAtlantis->RunLandOrders(prev_unit_land);
        }
        pOrders->SetReadOnly(ReadOnly);
        pOrders->ApplyFonts();
    }

    if (pComments)
    {
        if (pComments->m_pEditor->IsModified())
        {
            // OnKillFocus event for the editor did not fire up
            pComments->SaveModifications();
        }
        pComments->SetSource(pUnit?&pUnit->DefOrders:NULL, &m_CommentsChanged);
    }
    RedrawTracks();
    return return_value;
}

//-------------------------------------------------------------------------

void CAhApp::LoadOrders()
{
    int rc;
    CStr Dir;

    Dir = GetConfig(SZ_SECT_FOLDERS, SZ_KEY_FOLDER_ORDERS);
    if (Dir.IsEmpty())
        Dir = ".";

    wxString CurrentDir = wxGetCwd();
    wxFileDialog dialog(m_Frames[AH_FRAME_MAP],
                        wxT("Load orders"),
                        wxString::FromAscii(Dir.GetData()),
                        wxT(""),
                        wxT(SZ_ORD_FILES),
                        wxFD_OPEN );
    rc = dialog.ShowModal();
    wxSetWorkingDirectory(CurrentDir);

    if (wxID_OK==rc)
    {
        CStr S;
        S = dialog.GetPath().mb_str();
        MakePathRelative(CurrentDir.mb_str(), S);
        GetDirFromPath(S.GetData(), Dir);
        SetConfig(SZ_SECT_FOLDERS, SZ_KEY_FOLDER_ORDERS, Dir.GetData() );

        CUnitPane * pUnitPane = (CUnitPane*)m_Panes[AH_PANE_UNITS_HEX];
        if (pUnitPane) {
            pUnitPane->Update(NULL);
        }
            

        LoadOrders(S.GetData());
        orders_changed(false);
    }
}

//-------------------------------------------------------------------------

void CAhApp::LoadTerrainCostConfig()
{
    const char * szName, * szValue;
    int sectidx = GetSectionFirst(SZ_SECT_TERRAIN_COST, szName, szValue);
    while (sectidx >= 0)
    {
        if (szValue && *szValue)
        {
            m_pAtlantis->TerrainMovementCost[wxString::FromUTF8(szName)] = atoi(szValue);
        }
        sectidx = GetSectionNext(sectidx, SZ_SECT_TERRAIN_COST, szName, szValue);
    }
}

//-------------------------------------------------------------------------

BOOL CAhApp::CanCloseApp()
{
    SaveLandFlags();
    SaveUnitFlags();
    if (m_CommentsChanged)
        SaveComments();

    return ( m_DiscardChanges || !orders_changed() || ERR_OK==SaveOrders(TRUE));
}

//--------------------------------------------------------------------------

void CAhApp::ShowDescriptionList(CCollection & Items, const char * title) // Collection of CBaseObject
{
    CBaseObject  * pObj;

    if (Items.Count() > 0)
    {
        if (1 == Items.Count())
        {
            pObj = (CBaseObject*)Items.At(0);
            CShowOneDescriptionDlg dlg(gpApp->m_Frames[AH_FRAME_MAP], pObj->Name.GetData(), pObj->Description.GetData());
            dlg.ShowModal();
        }
        else
        {
            CShowDescriptionListDlg dlg(gpApp->m_Frames[AH_FRAME_MAP], title, &Items);
            dlg.ShowModal();
        }
    }

}

//--------------------------------------------------------------------------
/*
void CAhApp::ViewSkills(BOOL ViewAll)
{
    CBaseColl     Skills;
    CBaseObject * pSkill;
    const char  * szName;
    const char  * szValue;
    int           sectidx;

    if (ViewAll)
    {
        sectidx = GetSectionFirst(SZ_SECT_SKILLS, szName, szValue);
        while (sectidx >= 0)
        {
            pSkill              = new CBaseObject;
            pSkill->Name        = szName;
            DecodeConfigLine(pSkill->Description, szValue);
            Skills.Insert(pSkill);

            sectidx = GetSectionNext(sectidx, SZ_SECT_SKILLS, szName, szValue);
        }

        ShowDescriptionList(Skills, "Skills");
        Skills.FreeAll();
    }
    else
        ShowDescriptionList(m_pAtlantis->m_Skills, "Skills");
}
*/
//--------------------------------------------------------------------------

void CAhApp::ViewShortNamedObjects(BOOL ViewAll, const char * szSection, const char * szHeader, CBaseColl & ListNew)
{
    CBaseColl     Items;
    CBaseObject * pItem;
    const char  * szName;
    const char  * szValue;
    int           sectidx;

    if (ViewAll)
    {
        sectidx = GetSectionFirst(szSection, szName, szValue);
        while (sectidx >= 0)
        {
            pItem              = new CBaseObject;
            pItem->Name        = szName;
            DecodeConfigLine(pItem->Description, szValue);
            Items.Insert(pItem);

            sectidx = GetSectionNext(sectidx, szSection, szName, szValue);
        }

        ShowDescriptionList(Items, szHeader);
        Items.FreeAll();
    }
    else
        ShowDescriptionList(ListNew, szHeader);
}

//--------------------------------------------------------------------------

void CAhApp::ViewEvents(BOOL DoEvents)
{
    CBaseColl   Coll;

    if (DoEvents)
    {
        Coll.Insert(&m_pAtlantis->m_Events);
        ShowDescriptionList(Coll, "Events");
    }
    else
    {
//        Coll.Insert(&m_pAtlantis->m_Errors);
//        ShowDescriptionList(Coll, "Errors");
        m_MsgSrc.Empty();
        ShowError(m_pAtlantis->m_Errors.Description.GetData(), m_pAtlantis->m_Errors.Description.GetLength(), TRUE);

    }
    Coll.DeleteAll();
}

//--------------------------------------------------------------------------

void CAhApp::ViewSecurityEvents()
{
/*    CBaseColl   Coll;

    Coll.Insert(&m_pAtlantis->m_SecurityEvents);
    ShowDescriptionList(Coll, "Security Events");

    Coll.DeleteAll();*/

        m_MsgSrc.Empty();
        ShowError(m_pAtlantis->m_SecurityEvents.Description.GetData(), m_pAtlantis->m_SecurityEvents.Description.GetLength(), TRUE);
}

//--------------------------------------------------------------------------

void CAhApp::ViewNewProducts()
{
    ShowDescriptionList(m_pAtlantis->m_NewProducts, "New products");
}

//--------------------------------------------------------------------------

void CAhApp::ViewBattlesAll()
{
    ShowDescriptionList(m_pAtlantis->m_Battles, "Battles");
}

//--------------------------------------------------------------------------

void CAhApp::ViewGates()
{
    ShowDescriptionList(m_pAtlantis->m_Gates, "Gates");
}

//--------------------------------------------------------------------------

void CAhApp::ViewCities()
{
    CBaseObject      * pObj;
    CBaseCollByName    coll;
    int                np,nl;
    CPlane           * pPlane;
    CLand            * pLand;
    //int                x,y,z;
    CStr               sCoord;

    for (np=0; np<m_pAtlantis->m_Planes.Count(); np++)
    {
        pPlane = (CPlane*)m_pAtlantis->m_Planes.At(np);
        for (nl=0; nl<pPlane->Lands.Count(); nl++)
        {
            pLand    = (CLand*)pPlane->Lands.At(nl);
            if (!pLand->CityName.IsEmpty())
            {
                pObj       = new CBaseObject;
                pObj->Name = pLand->CityName;

                //LandIdToCoord(pLand->Id, x, y, z);
                m_pAtlantis->ComposeLandStrCoord(pLand, sCoord);
                pObj->Description << pLand->TerrainType << " (" << sCoord << ") in " << pLand->Name;
                pObj->Description << ", contains " << pLand->CityName << " [" << pLand->CityType << "]";

                if (!coll.Insert(pObj))
                    delete pObj;
            }
        }
    }

    ShowDescriptionList(coll, "Cities");
    coll.FreeAll();
}

//--------------------------------------------------------------------------

void CAhApp::ViewProvinces()
{
    CBaseObject      * pObj;
    CBaseCollByName    coll;
    int                np,nl;
    CPlane           * pPlane;
    CLand            * pLand;
    CStr               sCoord;
    int                loop;

    for (loop=0; loop<2; loop++)
    {
        for (np=0; np<m_pAtlantis->m_Planes.Count(); np++)
        {
            pPlane = (CPlane*)m_pAtlantis->m_Planes.At(np);
            for (nl=0; nl<pPlane->Lands.Count(); nl++)
            {
                pLand      = (CLand*)pPlane->Lands.At(nl);
                if ((pLand->Flags&LAND_VISITED) || 1==loop) // we run it twice, so we pick visited hexes if we can
                {
                    pObj       = new CBaseObject;
                    pObj->Name = pLand->Name;

                    m_pAtlantis->ComposeLandStrCoord(pLand, sCoord);
                    pObj->Description << pLand->TerrainType << " (" << sCoord << ") in " << pLand->Name;

                    if (!coll.Insert(pObj))
                        delete pObj;
                }
            }
        }
    }

    ShowDescriptionList(coll, "Provinces");
    coll.FreeAll();
}

//--------------------------------------------------------------------------

void CAhApp::ViewFactionInfo()
{
    CStr sMoreInfo(32), sInfo(32);
    int                np,nl;
    CPlane           * pPlane;
    CLand            * pLand;
    long               nLandsTotal = 0, nLandsVisited=0;

    sMoreInfo << EOL_SCR << "-------------------------" << EOL_SCR;
    for (np=0; np<m_pAtlantis->m_Planes.Count(); np++)
    {
        pPlane = (CPlane*)m_pAtlantis->m_Planes.At(np);
        for (nl=0; nl<pPlane->Lands.Count(); nl++)
        {
            pLand    = (CLand*)pPlane->Lands.At(nl);
            nLandsTotal++;
            if (pLand->Flags&LAND_VISITED)
                nLandsVisited++;
        }
    }
    sMoreInfo << "Total hexes  : " << nLandsTotal   << EOL_SCR
              << "Visited hexes: " << nLandsVisited << EOL_SCR ;


    sInfo << m_pAtlantis->m_FactionInfo << sMoreInfo;
    CShowOneDescriptionDlg dlg(gpApp->m_Frames[AH_FRAME_MAP],
                               "Faction Info",
                               sInfo.GetData());
    dlg.ShowModal();
}

//--------------------------------------------------------------------------

void CAhApp::ViewFactionOverview_IncrementValue(long FactionId, const char * factionname, CBaseCollById & Factions, const char * propname, long value)
{
    CBaseObject   * pFaction;
    CBaseObject     Dummy;
    int             idx;
    EValueType      type;
    const void    * valuetot;

    Dummy.Id = FactionId;
    if (Factions.Search(&Dummy, idx))
        pFaction = (CBaseObject*)Factions.At(idx);
    else
    {
        pFaction       = new CBaseObject;
        pFaction->Id   = FactionId;
        if (factionname)
            pFaction->Name = factionname;
        Factions.Insert(pFaction);
    }

    if (!pFaction->GetProperty(propname, type, valuetot, eNormal))
        valuetot = (void*)0;

    if (-1==(long)valuetot || 0x7fffffff - (long)value < (long)valuetot )
        valuetot = (void*)(long)-1; // overflow protection
    else
        valuetot = (void*)((long)valuetot + (long)value);
    pFaction->SetProperty(propname, eLong, valuetot, eNormal);
}

//--------------------------------------------------------------------------

void CAhApp::ViewFactionOverview()
{
//m_UnitPropertyNames

    int             unitidx, propidx, nl;
    CUnit         * pUnit;
    CStr            propname;
    CStr            Skill;
    int             skilllen;
    int             maxproplen = 0;
    CStr            Report(128);
    CMapPane      * pMapPane  = (CMapPane* )m_Panes[AH_PANE_MAP];
    BOOL            Selected  = FALSE;
    EValueType      type;
    const void    * value;
    int             idx;
    CBaseObject   * pFaction;
    long            men;

    CBaseColl       Hexes(64);
    CBaseCollById   Factions(16);
    CLand         * pLand;

    if (!pMapPane->HaveSelection())
        ShowMessageBoxSwitchable(wxT("Hint"), wxT("Faction overview can be generated using only selected area on the map"), wxT("FACTION_OVERVIEW"));

    if (pMapPane->HaveSelection() &&
        wxYES == wxMessageBox(wxT("Use only selected hexes?"), wxT("Confirm"), wxYES_NO, NULL))
        Selected = TRUE;

    skilllen    = strlen(PRP_SKILL_POSTFIX);

    // collect data
    pMapPane->GetSelectedOrAllHexes(Hexes, Selected);
    for (nl=0; nl<Hexes.Count(); nl++)
    {
        pLand = (CLand*)Hexes.At(nl);
        for (unitidx=0; unitidx<pLand->Units.Count(); unitidx++)
        {
            pUnit    = (CUnit*)pLand->Units.At(unitidx);
            men      = 0;
            if (pUnit->GetProperty(PRP_MEN, type, value, eOriginal) && (eLong==type) )
                men = (long)value;

            for (propidx=0; propidx<m_pAtlantis->m_UnitPropertyNames.Count(); propidx++)
            {
                propname = (const char *) gpApp->m_pAtlantis->m_UnitPropertyNames.At(propidx);

                // skip 'skill days' property
                if (IsASkillRelatedProperty(propname.GetData()) &&
                     propname.FindSubStrR(PRP_SKILL_POSTFIX) != propname.GetLength()-skilllen)
                    continue;

                // skip some properties which can not be aggegated
                if (0==stricmp(propname.GetData(), PRP_ID        ) ||
                    0==stricmp(propname.GetData(), PRP_FACTION_ID) ||
                    0==stricmp(propname.GetData(), PRP_LAND_ID   ) ||
                    0==stricmp(propname.GetData(), PRP_STRUCT_ID ) ||
                    0==stricmp(propname.GetData(), PRP_TEACHING  ) ||
                    0==stricmp(propname.GetData(), PRP_SKILLS    ) ||
                    0==stricmp(propname.GetData(), PRP_MAG_SKILLS) ||
                    0==stricmp(propname.GetData(), PRP_WEIGHT_WALK) ||
                    0==stricmp(propname.GetData(), PRP_WEIGHT_RIDE) ||
                    0==stricmp(propname.GetData(), PRP_WEIGHT_FLY) ||
                    0==stricmp(propname.GetData(), PRP_WEIGHT_SWIM) ||
                    0==stricmp(propname.GetData(), PRP_BEST_SKILL) ||
                    0==stricmp(propname.GetData(), PRP_BEST_SKILL_DAYS) ||
                    0==stricmp(propname.GetData(), PRP_MAG_SKILLS) ||
                    0==stricmp(propname.GetData(), PRP_GUI_COLOR ) ||
                    0==stricmp(propname.GetData(), PRP_SEQUENCE  ) ||
                    0==stricmp(propname.GetData(), PRP_FRIEND_OR_FOE  )


                   )
                    continue;

                if (pUnit->GetProperty(propname.GetData(), type, value, eOriginal) &&
                    (eLong==type) )
                    do
                    {
                        if (propname.FindSubStrR(PRP_SKILL_POSTFIX) == propname.GetLength()-skilllen)
                        {
                                // it is a skill

                            propname << (long)value;
                            value    = (void*)men;
                        }
                        else
                            if (IsASkillRelatedProperty(propname.GetData()))
                                break;

                        if (propname.GetLength() > maxproplen)
                            maxproplen = propname.GetLength();

                        ViewFactionOverview_IncrementValue(pUnit->FactionId, pUnit->pFaction ? pUnit->pFaction->Name.GetData() : NULL, Factions, propname.GetData(), (long)value);

                    } while (FALSE);

            }

            if (pUnit->Flags & UNIT_FLAG_AVOIDING)
                ViewFactionOverview_IncrementValue(pUnit->FactionId, pUnit->pFaction ? pUnit->pFaction->Name.GetData() : NULL, Factions, "Avoiding", men);
            else
            {
                if (pUnit->Flags & UNIT_FLAG_BEHIND)
                    ViewFactionOverview_IncrementValue(pUnit->FactionId, pUnit->pFaction ? pUnit->pFaction->Name.GetData() : NULL, Factions, "Back Line", men);
                else
                    ViewFactionOverview_IncrementValue(pUnit->FactionId, pUnit->pFaction ? pUnit->pFaction->Name.GetData() : NULL, Factions, "Front Line", men);
            }


            /*
            propidx  = 0;
            propname = pUnit->GetPropertyName(propidx);
            while (!propname.IsEmpty())
            {
                if (pUnit->GetProperty(propname.GetData(), type, value, eOriginal) &&
                    (eLong==type) )
                    do
                    {
                        if (propname.FindSubStrR(PRP_SKILL_POSTFIX) == propname.GetLength()-skilllen)
                        {
                            // it is a skill

                            propname << (long)value;
                            if (!pUnit->GetProperty(PRP_MEN, type, value, eOriginal) &&
                                (eLong==type) )
                                break;
                        }
                        else if (IsASkillRelatedProperty(propname.GetData()) ||
                                 0==stricmp(PRP_SEQUENCE, propname.GetData()) ||
                                 0==stricmp(PRP_STRUCT_ID, propname.GetData()) )
                            break;

                        if (propname.GetLength() > maxproplen)
                            maxproplen = propname.GetLength();

                        Dummy.Id = pUnit->FactionId;
                        if (Factions.Search(&Dummy, idx))
                            pFaction = (CBaseObject*)Factions.At(idx);
                        else
                        {
                            pFaction       = new CBaseObject;
                            pFaction->Id   = pUnit->FactionId;
                            if (pUnit->pFaction)
                                pFaction->Name = pUnit->pFaction->Name;
                            Factions.Insert(pFaction);
                        }

                        if (!pFaction->GetProperty(propname.GetData(), type, valuetot, eNormal))
                            valuetot = (void*)0;

                        valuetot = (void*)((long)valuetot + (long)value);
                        pFaction->SetProperty(propname.GetData(), eLong, valuetot, eNormal);
                    } while (FALSE);

                propname = pUnit->GetPropertyName(++propidx);
            }
            */
        }
    }
    Hexes.DeleteAll();

    // prepare display

    for (idx=0; idx<Factions.Count(); idx++)
    {
        pFaction = (CBaseObject*)Factions.At(idx);
        Report << "Faction " << pFaction->Id << " " << pFaction->Name << EOL_SCR << EOL_SCR;


        propidx  = 0;
        propname = pFaction->GetPropertyName(propidx);
        while (!propname.IsEmpty())
        {
            if (pFaction->GetProperty(propname.GetData(), type, value, eNormal) &&
                (eLong==type) )
            {
                while (propname.GetLength() < maxproplen)
                    propname.AddCh(' ');
                Report << propname << "  " << (long)value << EOL_SCR;
            }

            propname = pFaction->GetPropertyName(++propidx);
        }
        Report << EOL_SCR << "-------------------------------------------"  << EOL_SCR << EOL_SCR;
    }

    //display data

    CShowOneDescriptionDlg dlg(gpApp->m_Frames[AH_FRAME_MAP],
                               "Factions Overview",
                               Report.GetData());
    dlg.ShowModal();
    Factions.FreeAll();
}

//--------------------------------------------------------------------------

void CAhApp::CheckMonthLongOrders()
{
    static const char dup_ord_msg[] = ";--- Duplicate month long orders";
    int                  x;
    CUnit              * pUnit;
    const char         * src;
    const char         * dupord;
    const char         * p;
    char                 ch;
    CStr                 Line;
    CStr                 Ord;
    const char         * order;
    BOOL                 IsNew;
    BOOL                 Found;
    CStr                 Errors(128);
    CStr                 S(64);
    CStr                 FoundOrder;
    CStringSortColl      MonthLongOrders;
    CStringSortColl      MonthLongDup;
    long                 men;
    EValueType           type;
    CUnitPaneFltr      * pUnitPaneF = NULL;
    int                  errcount = 0;
    int                  turnlvl;
    CBaseColl            Hexes(64);
    int                  nl, unitidx;
    CLand              * pLand;
    CMapPane           * pMapPane  = (CMapPane* )m_Panes[AH_PANE_MAP];


    p = SkipSpaces(GetConfig(SZ_SECT_COMMON, SZ_KEY_ORD_MONTH_LONG));
    while (p && *p)
    {
        p = SkipSpaces(S.GetToken(p, ','));
        if (!S.IsEmpty())
            MonthLongOrders.Insert(strdup(S.GetData()));
    }

    p = SkipSpaces(GetConfig(SZ_SECT_COMMON, SZ_KEY_ORD_DUPLICATABLE));
    while (p && *p)
    {
        p = SkipSpaces(S.GetToken(p, ','));
        if (!S.IsEmpty())
            MonthLongDup.Insert(strdup(S.GetData()));
    }

    if (1==atol(SkipSpaces(GetConfig(SZ_SECT_COMMON, SZ_KEY_CHECK_OUTPUT_LIST))))
    {
        // Output will go into the unit filter window
        OpenUnitFrameFltr(FALSE);
        pUnitPaneF = (CUnitPaneFltr*)m_Panes [AH_PANE_UNITS_FILTER];
    }

    if (pUnitPaneF)
        pUnitPaneF->InsertUnitInit();

    pMapPane->GetSelectedOrAllHexes(Hexes, FALSE);
    for (nl=0; nl<Hexes.Count(); nl++)
    {
        pLand = (CLand*)Hexes.At(nl);
        for (unitidx=0; unitidx<pLand->Units.Count(); unitidx++)
        {
            pUnit    = (CUnit*)pLand->Units.At(unitidx);

            if (!pUnit->IsOurs)
                continue;
            src   = pUnit->Orders.GetData();
            IsNew = FALSE;
            Found = FALSE;
            turnlvl = 0;
            while (src && *src)
            {
                dupord = src;
                src    = Line.GetToken(src, '\n', TRIM_ALL);
                Ord.GetToken(SkipSpaces(Line.GetData()), " \t", ch, TRIM_ALL);
                order = Ord.GetData();
                if ('@'==*order)
                    order++;
                if (0==SafeCmp("FORM", order))
                    IsNew = TRUE;
                else if (0==SafeCmp("END", order))
                    IsNew = FALSE;
                else if (0==SafeCmp("TURN", order))
                    turnlvl++;
                else if (0==SafeCmp("ENDTURN", order))
                    turnlvl--;
                else if (!IsNew && 0==turnlvl && MonthLongOrders.Search((void*)order, x) )
                {
                    if (Found)
                    {
                        if (0==stricmp(order, FoundOrder.GetData()) &&
                            MonthLongDup.Search((void*)order, x))
                            continue; // it is an order which can be duplicated

                        errcount++;
                        if (pUnitPaneF)
                        {
                            int newpos;

                            pUnitPaneF->InsertUnit(pUnit);
                            S = dup_ord_msg;
                            S << EOL_SCR;
                            newpos = dupord - pUnit->Orders.GetData() + S.GetLength();
                            pUnit->Orders.InsBuf(S.GetData(), dupord - pUnit->Orders.GetData(), S.GetLength());
                            src = &pUnit->Orders.GetData()[newpos];
                        }
                        else
                        {
                            S.Format("Unit % 5d Error : Duplicate month long orders - %s", pUnit->Id, Line.GetData());
                            Errors << S << EOL_SCR;
                        }
                        break;
                    }
                    Found      = TRUE;
                    FoundOrder = order;
                }
            }
            if (!Found)
            {
                if (!pUnit->GetProperty(PRP_MEN, type, (const void *&)men, eNormal) ||
                    (eLong==type && 0==men))
                    continue; // no men - no orders is ok

                errcount++;
                if (pUnitPaneF)
                {
                    pUnitPaneF->InsertUnit(pUnit);
                }
                else
                {
                    S.Format("Unit % 5d Warning : No month long orders", pUnit->Id);
                    Errors << S << EOL_SCR;
                }
            }
        }
    }

    Hexes.DeleteAll();


    if (pUnitPaneF)
        pUnitPaneF->InsertUnitDone();

    if (!pUnitPaneF && errcount>0)
        ShowError(Errors.GetData(), Errors.GetLength(), TRUE);

    if (0==errcount)
        wxMessageBox(wxT("No problems found."), wxT("Order checking"), wxOK | wxCENTRE, m_Frames[AH_FRAME_MAP]);


//int wxMessageBox(const wxString& message, const wxString& caption = "Message", int style = wxOK | wxCENTRE,
// wxWindow *parent = NULL, int x = -1, int y = -1)

    MonthLongOrders.FreeAll();
    MonthLongDup.FreeAll();
}

//--------------------------------------------------------------------------

void CAhApp::GetUnitsMovingIntoHex(long HexId, std::vector<CUnit*>& stopped, std::vector<CUnit*>& ended_moveorder) const
{
    CLand          * pLand;
    int              nl, np;

    for (np=0; np<m_pAtlantis->m_Planes.Count(); np++)
    {
        CPlane * pPlane = (CPlane*)m_pAtlantis->m_Planes.At(np);
        if (pPlane)
        {
            for (nl=0; nl<pPlane->Lands.Count(); nl++)
            {
                pLand = (CLand*)pPlane->Lands.At(nl);
                land_control::perform_on_each_unit(pLand, [&](CUnit* unit) {
                    if (unit->movements_.size() > 0)
                    {
                        long last_hex_movements = unit->movements_[unit->movements_.size()-1];
                        if (unit->movement_stop_ == HexId)
                            stopped.push_back(unit);
                        else if (last_hex_movements == HexId)
                            ended_moveorder.push_back(unit);//should be filled just if its not `stopped`
                    }
                });
            }
        }
    }
}

void CAhApp::ShowUnitsMovingIntoHex(long CurHexId)
{
    CUnitPaneFltr  * pUnitPaneF = NULL;
    CStr             UnitText(128), S(16);

    std::vector<CUnit*> stopping;
    std::vector<CUnit*> ending_moving_orders;
    GetUnitsMovingIntoHex(CurHexId, stopping, ending_moving_orders);

    if (stopping.size() > 0)
    {
        if (1==atol(SkipSpaces(GetConfig(SZ_SECT_COMMON, SZ_KEY_CHECK_OUTPUT_LIST))))
        {
            // Output will go into the unit filter window
            OpenUnitFrameFltr(FALSE);
            pUnitPaneF = (CUnitPaneFltr*)m_Panes [AH_PANE_UNITS_FILTER];
            pUnitPaneF->InsertUnitInit();
        }

        for (auto& unit : stopping)
        {
            if (pUnitPaneF)
                pUnitPaneF->InsertUnit(unit);
            else
            {
                S.Format("Unit % 5d", unit->Id);
                UnitText << S << EOL_SCR;
            }
        }

        if (pUnitPaneF)
            pUnitPaneF->InsertUnitDone();
        else
            ShowError(UnitText.GetData(), UnitText.GetLength(), TRUE);
    }
    else
        wxMessageBox(wxT("Found no units moving into the current hex."), wxT("Units moving"), wxOK | wxCENTRE, m_Frames[AH_FRAME_MAP]);
}

//--------------------------------------------------------------------------

void CAhApp::AddTempHex(int X, int Y, int Plane)
{
    CLand  * pCurLand = m_pAtlantis->GetLand(X, Y, Plane, TRUE);
    if (pCurLand)
        return;

    CPlane * pPlane = (CPlane*)m_pAtlantis->m_Planes.At(Plane);
    if (!pPlane)
        return;

    assert(Plane == pPlane->Id);

    CStr     sTerrain;
    wxString strTerrain = wxGetTextFromUser(wxT("Terrain"), wxT("Please specify terrain type"));
    sTerrain = strTerrain.mb_str();

    if (sTerrain.IsEmpty())
        return;

    CLand * pLand       = new CLand;
    pLand->Id           = LandCoordToId ( X,Y, pPlane->Id );
    pLand->pPlane       = pPlane;
    pLand->Name         = SZ_MANUAL_HEX_PROVINCE;
    pLand->TerrainType  = sTerrain;
    pLand->Description  << sTerrain << " (" << (long)X << "," << (long)Y << ") in " SZ_MANUAL_HEX_PROVINCE; // ", 0 peasants (unknown), $0.";
    pPlane->Lands.Insert ( pLand );
}

//--------------------------------------------------------------------------

void CAhApp::DelTempHex(int X, int Y, int Plane)
{
    int      idx;
    CLand  * pCurLand = m_pAtlantis->GetLand(X, Y, Plane, TRUE);
    if (!pCurLand)
        return;

    CPlane * pPlane = (CPlane*)m_pAtlantis->m_Planes.At(Plane);
    if (!pPlane)
        return;

    assert(Plane == pPlane->Id);

    if (pPlane->Lands.Search(pCurLand, idx))
        pPlane->Lands.AtFree(idx);
}

//--------------------------------------------------------------------------

void CAhApp::RerunOrders()
{
    m_pAtlantis->RunOrders(NULL);//, TurnSequence::SQ_FIRST, TurnSequence::SQ_BUY);
    CUnitPane * pUnitPane = (CUnitPane*)gpApp->m_Panes[AH_PANE_UNITS_HEX];
    if (pUnitPane) {
        pUnitPane->Update(pUnitPane->m_pCurLand);
    }
        

}

//--------------------------------------------------------------------------

int CAhApp::SaveHistory(const char * FNameOut)
{
    CLand            * pLand;
    CFileWriter        Dest;
    int                nl, np;
    CPlane           * pPlane;
    SAVE_HEX_OPTIONS   options;

    memset(&options, 0, sizeof(options));
    options.AlwaysSaveImmobStructs = TRUE;
    options.SaveResources          = TRUE;

    if ( (m_pAtlantis->m_Planes.Count()>0) &&
         (0==m_pAtlantis->m_ParseErr)      && // don't destroy if not loaded!
         Dest.Open(FNameOut)
       )
    {
        for (np=0; np<m_pAtlantis->m_Planes.Count(); np++)
        {
            pPlane = (CPlane*)m_pAtlantis->m_Planes.At(np);
            for (nl=0; nl<pPlane->Lands.Count(); nl++)
            {
                pLand    = (CLand*)pPlane->Lands.At(nl);
                m_pAtlantis->SaveOneHex(Dest, pLand, pPlane, &options);
            }
        }
        Dest.Close();
    }
    return ERR_OK;
}

//--------------------------------------------------------------------------




BOOL CAhApp::GetExportHexOptions(CStr & FName, CStr & FMode, SAVE_HEX_OPTIONS & options, eHexIncl & HexIncl,
                                 bool & InclTurnNoAcl )
{

    static CStr     stFName;
    static bool     stOverwrite     = FALSE;
    static eHexIncl stHexIncl       = HexNew;
    static bool     stInclStructs   = TRUE;
    static bool     stInclUnits     = TRUE;
    static bool     stInclTurnNoAcl = FALSE;
    static bool     stInclResources = TRUE;

    CHexExportDlg   dlg(m_Frames[AH_FRAME_MAP]);

    memset(&options, 0, sizeof(options));
    options.SaveUnits = TRUE;

    if (stFName.IsEmpty())
        stFName.Format("map.%04d", m_pAtlantis->m_YearMon);

    dlg.m_tcFName         ->SetValue(wxString::FromAscii(stFName.GetData()));

    dlg.m_rbHexNew        ->SetValue(HexNew      == stHexIncl);
    dlg.m_rbHexCurrent    ->SetValue(HexCurrent  == stHexIncl);
    dlg.m_rbHexSelected   ->SetValue(HexSelected == stHexIncl);
    dlg.m_rbHexAll        ->SetValue(HexAll      == stHexIncl);

    dlg.m_rbFileOverwrite ->SetValue(false); //stOverwrite);
    dlg.m_rbFileAppend    ->SetValue(true);  //!stOverwrite);

    dlg.m_chbInclStructs  ->SetValue(stInclStructs  );
    dlg.m_chbInclUnits    ->SetValue(stInclUnits    );
    dlg.m_chbInclTurnNoAcl->SetValue(stInclTurnNoAcl);
    dlg.m_chbInclResources->SetValue(stInclResources);



    if (wxID_OK == dlg.ShowModal())
    {
        stFName.SetStr(dlg.m_tcFName->GetValue().mb_str());

        if (dlg.m_rbHexNew->GetValue())
            stHexIncl = HexNew;
        else if (dlg.m_rbHexCurrent->GetValue())
            stHexIncl = HexCurrent;
        else if (dlg.m_rbHexSelected->GetValue())
            stHexIncl = HexSelected;
        else if (dlg.m_rbHexAll->GetValue())
            stHexIncl = HexAll;

        stOverwrite = dlg.m_rbFileOverwrite->GetValue();

        stInclStructs   = dlg.m_chbInclStructs  ->GetValue();
        stInclUnits     = dlg.m_chbInclUnits    ->GetValue();
        stInclTurnNoAcl = dlg.m_chbInclTurnNoAcl->GetValue();
        stInclResources = dlg.m_chbInclResources->GetValue();

        FName = stFName;
#if defined(_MSC_VER)
        FMode = stOverwrite?"wb":"ab";
#else
        FMode = stOverwrite?"w":"a";
#endif
        options.SaveStructs  = stInclStructs;
        options.SaveUnits    = stInclUnits;
        options.SaveResources= stInclResources;
        HexIncl = stHexIncl;
        InclTurnNoAcl = stInclTurnNoAcl;

        return TRUE;
    }


    return FALSE;
}

//--------------------------------------------------------------------------

// will discriminate by new hex

void CAhApp::ExportOneHex(CFileWriter & Dest, CPlane * pPlane, CLand * pLand, SAVE_HEX_OPTIONS & options, bool InclTurnNoAcl, bool OnlyNew)
{
    CStr               sData, sName;
    const char       * p;
    int                ym_first = 0;
    int                ym_last  = 0;

    m_pAtlantis->ComposeLandStrCoord(pLand, sName);

    p  = sData.GetToken(GetConfig(SZ_SECT_LAND_VISITED, sName.GetData()), ',');
    if (sData.IsEmpty())
    {
/*        ym_first = m_pAtlantis->m_YearMon;
        ym_last  = m_pAtlantis->m_YearMon;*/
    }
    else
    {
        ym_last = atol(sData.GetData());
        sData.GetToken(SkipSpaces(p), ',');
        ym_first = atol(sData.GetData());
    }

    if (InclTurnNoAcl)
        options.WriteTurnNo = (ym_last/100 - 1)*12 + ym_last%100;
    else
        options.WriteTurnNo = 0;

    if (ym_first==m_pAtlantis->m_YearMon || !OnlyNew)
    {
        m_pAtlantis->SaveOneHex(Dest, pLand, pPlane, &options);
    }
}

//--------------------------------------------------------------------------

void CAhApp::ExportHexes()
{
    CStr               sData, sName;
    CMapPane         * pMapPane  = (CMapPane* )m_Panes[AH_PANE_MAP];

    CLand            * pLand;
    CFileWriter        Dest;
    int                nl;
    CPlane           * pPlane;
    SAVE_HEX_OPTIONS   options;
    eHexIncl           HexIncl;
    bool               InclTurnNoAcl ;

    if ( GetExportHexOptions(sName, sData, options, HexIncl, InclTurnNoAcl) &&
         Dest.Open(sName.GetData(), sData.GetData()) )
    {
        if (HexCurrent==HexIncl)
        {
            pPlane   = (CPlane*)m_pAtlantis->m_Planes.At(pMapPane->m_SelPlane);
            pLand    = m_pAtlantis->GetLand(pMapPane->m_SelHexX, pMapPane->m_SelHexY, pMapPane->m_SelPlane, TRUE);
            ExportOneHex(Dest, pPlane, pLand, options, InclTurnNoAcl, FALSE);
        }
        else
        {
            CBaseColl  Hexes(64);
            pMapPane->GetSelectedOrAllHexes(Hexes, HexSelected==HexIncl);
            for (nl=0; nl<Hexes.Count(); nl++)
            {
                pLand = (CLand*)Hexes.At(nl);
                ExportOneHex(Dest, pLand->pPlane, pLand, options, InclTurnNoAcl, HexNew==HexIncl);
            }

            Hexes.DeleteAll();
        }
    }
    Dest.Close();
}

//--------------------------------------------------------------------------

void CAhApp::FindTradeRoutes()
{
    CMapPane    * pMapPane  = (CMapPane* )m_Panes[AH_PANE_MAP];
    CBaseColl     Hexes(64);
    CLand       * pSellLand, * pBuyLand;
    int           i, j;
    CStr          Report(64);
    int           idx;
    const char  * propnameprice;
    EValueType    type;
    const void  * value;
    CStr          GoodsName(32), PropName(32), sCoord(32);
    long          nSaleAmount, nSalePrice, nBuyAmount, nBuyPrice;

    if (!pMapPane)
        return;

    pMapPane->GetSelectedOrAllHexes(Hexes, TRUE);
    if (0==Hexes.Count())
    {
        wxMessageBox(wxT("Please select area on the map first."));
        return;
    }
    wxBeginBusyCursor();

    for (i=0; i<Hexes.Count(); i++)
    {
        pSellLand = (CLand*)Hexes.At(i);

        idx      = 0;
        propnameprice = pSellLand->GetPropertyName(idx);
        while (propnameprice)
        {
            if (pSellLand->GetProperty(propnameprice, type, value, eOriginal) &&
                eLong==type &&
                0==strncmp(propnameprice, PRP_SALE_PRICE_PREFIX, sizeof(PRP_SALE_PRICE_PREFIX)-1))
            {
                nSalePrice = (long)value;
                GoodsName = &(propnameprice[sizeof(PRP_SALE_PRICE_PREFIX)-1]);

                PropName.Empty();
                PropName << PRP_SALE_AMOUNT_PREFIX << GoodsName;
                if (!pSellLand->GetProperty(PropName.GetData(), type, value, eOriginal) || eLong!=type)
                    continue;
                nSaleAmount = (long)value;

                for (j=0; j<Hexes.Count(); j++)
                {
                    pBuyLand = (CLand*)Hexes.At(j);

                    PropName.Empty();
                    PropName << PRP_WANTED_PRICE_PREFIX << GoodsName;
                    if (!pBuyLand->GetProperty(PropName.GetData(), type, value, eOriginal) || eLong!=type)
                        continue;
                    nBuyPrice = (long)value;

                    PropName.Empty();
                    PropName << PRP_WANTED_AMOUNT_PREFIX << GoodsName;
                    if (!pBuyLand->GetProperty(PropName.GetData(), type, value, eOriginal) || eLong!=type)
                        continue;
                    nBuyAmount = (long)value;

                    if (nBuyPrice > nSalePrice)
                    {
                        m_pAtlantis->ComposeLandStrCoord(pSellLand, sCoord);
                        Report << pSellLand->TerrainType << " (" << sCoord << ") " << EOL_SCR;
                        m_pAtlantis->ComposeLandStrCoord(pBuyLand, sCoord);
                        Report << "         to " << pBuyLand->TerrainType << " (" << sCoord << ")   ("
                               << nBuyPrice << "-" << nSalePrice << ")*" << std::min(nSaleAmount,nBuyAmount)
                               << " " << GoodsName
                               << " = " << (nBuyPrice - nSalePrice) * std::min(nSaleAmount,nBuyAmount) << EOL_SCR;
                    }
                }
            }
            propnameprice = pSellLand->GetPropertyName(++idx);
        }
    }

    if (Report.IsEmpty())
        wxMessageBox(wxT("No trade routes found."));
    else
        ShowError(Report.GetData()      , Report.GetLength()      , TRUE);

    Hexes.DeleteAll();
    wxEndBusyCursor();
}

//--------------------------------------------------------------------------

void CAhApp::EditListColumns(int command)
{
    CMapFrame   * pMapFrame  = (CMapFrame *)m_Frames[AH_FRAME_MAP];
    CUnitPane   * pUnitPane  = NULL;
    const char  * szConfigSectionHdr;


    const char * szKey = NULL;
    switch (command)
    {
    case menu_ListColUnits:
        szKey = SZ_KEY_LIS_COL_UNITS_HEX;
        pUnitPane = (CUnitPane*)m_Panes[AH_PANE_UNITS_HEX];
        break;

    case menu_ListColUnitsFltr:
        szKey = SZ_KEY_LIS_COL_UNITS_FILTER;
        pUnitPane = (CUnitPane*)m_Panes[AH_PANE_UNITS_FILTER];
        break;

    default:
        return;
    }
    if (pUnitPane)
        pUnitPane->SaveUnitListHdr();

    CListHeaderEditDlg dlg(pMapFrame, szKey);

    if (wxID_OK == dlg.ShowModal())
    {
        szConfigSectionHdr = GetListColSection(SZ_SECT_LIST_COL_UNIT, szKey);
        if (pUnitPane)
            pUnitPane->ReloadHdr(szConfigSectionHdr);
    }
}

//--------------------------------------------------------------------------

const char * CAhApp::GetListColSection(const char * sectprefix, const char * key)
{
    const char * sect;

    sect = GetConfig(SZ_SECT_LIST_COL_CURRENT, key);
    if (!sect || !*sect)
        sect  = GetNextSectionName(CONFIG_FILE_CONFIG, sectprefix);

    return sect;
}

//--------------------------------------------------------------------------

void CAhApp::StdRedirectInit()
{
#ifdef __WXMAC_OSX__
	char cwd[MAXPATHLEN];
	// Setup new working directory in case we got started from /Applications
	if((getcwd(cwd, MAXPATHLEN)) != NULL){
		if((strncmp(cwd, "/Applications", strlen("/Applications"))) == 0){
			const char *home = getenv("HOME");
			if(home != NULL){
				if(0 == chdir(home)){
					mkdir(".alh", 0750);
					if(0 != chdir(".alh"))
						chdir("/Applications");
				}
			}
		}
	}
#endif
    freopen("ah.stdout", "w", stdout);
    freopen("ah.stderr", "w", stderr);
    m_nStdoutLastPos = 0;
    m_nStderrLastPos = 0;
}

//--------------------------------------------------------------------------

void CAhApp::StdRedirectReadMore(BOOL FromStdout, CStr & sData)
{
    FILE       * f;
    int        * pCurPos;
    char         buf[1024];
    int          n;

    sData.Empty();
    if (FromStdout)
    {
        fflush(stdout);
        pCurPos  =  &m_nStdoutLastPos;
        f        = fopen("ah.stdout", "rb");
    }
    else
    {
        fflush(stderr);
        pCurPos  =  &m_nStderrLastPos;
        f        = fopen("ah.stderr", "rb");
    }

    if (f)
    {
        fseek(f, *pCurPos, SEEK_SET);
        do
        {
            n = fread(buf, 1, sizeof(buf), f);
            if (n>0)
                sData.AddBuf(buf, n);
        } while (n>0);
        *pCurPos = ftell(f);
        fclose(f);
    }
}

//--------------------------------------------------------------------------

void CAhApp::CheckRedirectedOutputFiles()
{
    CStr S;

    gpApp->StdRedirectReadMore(FALSE, S);
    if (!S.IsEmpty())
        ShowError(S.GetData(), S.GetLength(), TRUE);
    gpApp->StdRedirectReadMore(TRUE, S);
    if (!S.IsEmpty())
        ShowError(S.GetData(), S.GetLength(), TRUE);
}

//--------------------------------------------------------------------------

void CAhApp::StdRedirectDone()
{
}

//--------------------------------------------------------------------------

void CAhApp::InitMoveModes()
{
    const char * p;
    CStr         S;
    int          n;
    BOOL         Update = FALSE;

    p     = SkipSpaces(GetConfig(SZ_SECT_COMMON, SZ_KEY_MOVEMENTS));
    while (p && *p)
    {
        p = SkipSpaces(S.GetToken(p, ','));
        m_MoveModes.Insert(strdup(S.GetData()));
    }

    // do update here for 2.3.2
    p = SZ_DEFAULT_MOVEMENT_MODE;
    n = 0;
    while (p && *p)
    {
        p = SkipSpaces(S.GetToken(p, ','));
        n++;

        if (n > m_MoveModes.Count())
        {
            m_MoveModes.Insert(strdup(S.GetData()));
            Update = TRUE;
        }
    }
    if (Update)
    {
        S.Empty();
        for (n=0; n<m_MoveModes.Count(); n++)
        {
            if (n>0)
                S << ',';
            S << (const char *)m_MoveModes.At(n);
        }
        SetConfig(SZ_SECT_COMMON, SZ_KEY_MOVEMENTS, S.GetData());
    }
}

//--------------------------------------------------------------------------

void CAhApp::CreateNewUnit(wxCommandEvent& event)
{
    if (m_Panes[AH_PANE_UNITS_HEX])
        ((CUnitPane*)m_Panes[AH_PANE_UNITS_HEX])->OnPopupMenuCreateNew(event);
}

void CAhApp::UnitReceiveOrder(wxCommandEvent& event)
{
    if (m_Panes[AH_PANE_UNITS_HEX])
        ((CUnitPane*)m_Panes[AH_PANE_UNITS_HEX])->OnPopupMenuReceiveItems(event);
}

void CAhApp::SelectNextUnit()
{
    if (m_Panes[AH_PANE_UNITS_HEX])
        ((CUnitPane*)m_Panes[AH_PANE_UNITS_HEX])->SelectNextUnit();
}

//--------------------------------------------------------------------------

void CAhApp::SelectPrevUnit()
{
    if (m_Panes[AH_PANE_UNITS_HEX])
        ((CUnitPane*)m_Panes[AH_PANE_UNITS_HEX])->SelectPrevUnit();
}

//--------------------------------------------------------------------------

void CAhApp::SelectUnitsPane()
{
    if (m_Panes[AH_PANE_UNITS_HEX])
        ((CUnitPane*)m_Panes[AH_PANE_UNITS_HEX])->SetFocus();
}

//--------------------------------------------------------------------------

void CAhApp::SelectOrdersPane()
{
    if (m_Panes[AH_PANE_UNIT_COMMANDS])
        ((CUnitOrderEditPane*)m_Panes[AH_PANE_UNIT_COMMANDS])->SetFocus();
}

//--------------------------------------------------------------------------

void CAhApp::ViewMovedUnits()
{
}

//=========================================================================

void  CGameDataHelper::ReportError(const char * msg, int msglen, BOOL orderrelated)
{
    gpApp->ShowError(msg, msglen, !orderrelated);
};

const char *  CGameDataHelper::ResolveAlias(const char * alias)
{
    return gpApp->ResolveAlias(alias);
}

bool CGameDataHelper::ResolveAliasItems (const std::string& phrase, std::string& codename, std::string& long_name, std::string& long_name_plural)
{
    return gpApp->ResolveAliasItems(phrase, codename, long_name, long_name_plural);
}

BOOL CGameDataHelper::GetItemWeights(const char * item, int *& weights, const char **& movenames, int & movecount )
{
    return gpApp->GetItemWeights(ResolveAlias(item), weights, movenames, movecount );
}

void CGameDataHelper::GetMoveNames(const char **& movenames)
{
    gpApp->GetMoveNames(movenames);
}

const char * CGameDataHelper::GetConfString(const char * section, const char * param)
{
    if (!section)
        section = SZ_SECT_COMMON;
    return gpApp->GetConfig(section, param);
}

BOOL CGameDataHelper::GetOrderId(const char * order, long & id)
{
    return gpApp->GetOrderId(order, id);
}

BOOL CGameDataHelper::IsTradeItem(const char * item)
{
    return gpApp->IsTradeItem(item);
}

BOOL CGameDataHelper::IsMan(const char * item)
{
    return gpApp->IsMan(item);
}

const char * CGameDataHelper::GetWeatherLine(BOOL IsCurrent, BOOL IsGood, int Zone)
{
    return gpApp->GetWeatherLine(IsCurrent, IsGood, Zone);
}

BOOL CGameDataHelper::GetTropicZone  (const char * plane, long & y_min, long & y_max)
{
    const char * value;
    CStr         S;

    value = SkipSpaces(gpApp->GetConfig(SZ_SECT_TROPIC_ZONE, plane));
    if (!value || !*value)
        return FALSE;

    value = S.GetToken(value, ',');
    y_min = atol(S.GetData());

    value = S.GetToken(value, ',');
    y_max = atol(S.GetData());

    return TRUE;
}

void CGameDataHelper::SetTropicZone  (const char * plane, long y_min, long y_max)
{
    CStr S;
    S << y_min << ',' << y_max;
    gpApp->SetConfig(SZ_SECT_TROPIC_ZONE, plane, S.GetData());
}

const char * CGameDataHelper::GetPlaneSize (const char * plane)
{
    return gpApp->GetConfig(SZ_SECT_PLANE_SIZE, plane);
}

std::shared_ptr<TProdDetails> CGameDataHelper::GetProdDetails (const char * item)
{
    return gpApp->GetProdDetails(item);
}

BOOL CGameDataHelper::ImmediateProdCheck()
{
    return atol(gpApp->GetConfig(SZ_SECT_COMMON,  SZ_KEY_CHK_PROD_REQ));
}

BOOL CGameDataHelper::CanSeeAdvResources(const char * skillname, const char * terrain, CLongColl & Levels, CBufColl & Resources)
{
    return gpApp->CanSeeAdvResources(skillname, terrain, Levels, Resources);
}

int64_t CGameDataHelper::GetAttitudeForFaction(int id)
{
    return gpApp->GetAttitudeForFaction(id);
}

void CGameDataHelper::SetAttitudeForFaction(int id, int attitude)
{
    gpApp->SetAttitudeForFaction(id, attitude);
}

void CGameDataHelper::SetPlayingFaction(long id)
{
    // set playing faction to ATT_FRIEND2
    gpApp->SetAttitudeForFaction(id, ATT_FRIEND2);

    int fileno = gpApp->GetConfigFileNo(SZ_SECT_ATTITUDES);
    long playing_faction_id = gpApp->config_[fileno].get(SZ_SECT_ATTITUDES, SZ_ATT_PLAYER_ID, 0);
    if (playing_faction_id == 0)
        gpApp->config_[fileno].set(SZ_SECT_ATTITUDES, SZ_ATT_PLAYER_ID, id);

    //gpApp->SetConfig(SZ_SECT_ATTITUDES, SZ_ATT_PLAYER_ID, id);
}

BOOL CGameDataHelper::IsRawMagicSkill(const char * skillname)
{
    static int     postlen = strlen(PRP_SKILL_POSTFIX);
    CStr           S;

    S = skillname;
    if (S.FindSubStrR(PRP_SKILL_POSTFIX) == S.GetLength()-postlen)
    {
        S.DelSubStr(S.GetLength()-postlen, postlen);
        return gpApp->IsMagicSkill(S.GetData());
    }

    return FALSE;
}

BOOL CGameDataHelper::IsWagon(const char * item)
{
    if (!item)
        return FALSE;
    CStr S = gpApp->GetConfig(SZ_SECT_COMMON, SZ_KEY_WAGONS);
    CStr T;
    const char * p = S.GetData();
    while (p && *p)
    {
        p = T.GetToken(p, ',', TRIM_ALL);
        if (0==stricmp(item, T.GetData()))
            return TRUE;
    }
    return FALSE;
}

BOOL CGameDataHelper::IsWagonPuller(const char * item)
{
    if (!item)
        return FALSE;
    CStr S = gpApp->GetConfig(SZ_SECT_COMMON, SZ_KEY_WAGON_PULLERS);
    CStr T;
    const char * p = S.GetData();
    while (p && *p)
    {
        p = T.GetToken(p, ',', TRIM_ALL);
        if (0==stricmp(item, T.GetData()))
            return TRUE;
    }
    return FALSE;
}

int CGameDataHelper::WagonCapacity()
{
    return atol(gpApp->GetConfig(SZ_SECT_COMMON, SZ_KEY_WAGON_CAPACITY));
}

//==========================================================================

void FontToStr(const wxFont * font, CStr & s)
{
    s.Empty();
    s << (long)font->GetPointSize()  << ","
      << (long)font->GetFamily   ()  << ","
      << (long)font->GetStyle    ()  << ","
      << (long)font->GetWeight   ()  << ","
      << (long)font->GetEncoding ()  << ","
      <<       font->GetFaceName ().mb_str() ;
}

//--------------------------------------------------------------------------

#if defined(_WIN32)
   #define AH_DEFAULT_FONT_SIZE 10
#else
   #define AH_DEFAULT_FONT_SIZE 12
#endif


wxFont * NewFontFromStr(const char * p)
{
    int            size;
    wxFontFamily   family;
    wxFontStyle    style;
    wxFontWeight   weight;
    wxFontEncoding encoding;
    wxString       facename;
    wxFont     *   font;


    CStr           S;

    if (p && *p)
    {
        p = S.GetToken(SkipSpaces(p), ',');  size     = atol(S.GetData());
        p = S.GetToken(SkipSpaces(p), ',');  family   = static_cast<wxFontFamily>(atol(S.GetData()));
        p = S.GetToken(SkipSpaces(p), ',');  style    = static_cast<wxFontStyle>( atol(S.GetData()));
        p = S.GetToken(SkipSpaces(p), ',');  weight   = static_cast<wxFontWeight>(atol(S.GetData()));
        p = S.GetToken(SkipSpaces(p), ',');  encoding = static_cast<wxFontEncoding>(atol(S.GetData()));
                                             facename = wxString::FromAscii(SkipSpaces(p));
    }
    else
    {
        size     = AH_DEFAULT_FONT_SIZE;
        family   = wxFONTFAMILY_DEFAULT;
        style    = wxFONTSTYLE_NORMAL;
        weight   = wxFONTWEIGHT_NORMAL;
        encoding = wxFONTENCODING_SYSTEM;
        facename = wxT("");
    }

    font = new wxFont(size, family, style, weight, FALSE, facename, encoding);

    return font;
}

//--------------------------------------------------------------------------

void StrToColor(wxColour * cr, const char * p)
{
    CStr          S;
    int           r, g, b;

    p = S.GetToken(p, ',');
    r = atol(S.GetData());

    p = S.GetToken(p, ',');
    g = atol(S.GetData());

    p = S.GetToken(p, ',');
    b = atol(S.GetData());

    cr->Set(r,g,b);
}

//--------------------------------------------------------------------------

void ColorToStr(char * p, wxColour * cr)
{
    sprintf(p, "%d, %d, %d",
            (int)(cr->Red()  ),
            (int)(cr->Green()),
            (int)(cr->Blue() )
        );
}

//--------------------------------------------------------------------------

#if defined(__WXMSW__)
  #define EQUAL_PATH_CHARS(a, b) (tolower(a) == tolower(b))
#else
  #define EQUAL_PATH_CHARS(a, b) (a == b)
#endif

#if defined(__WXMSW__)
#define SEP '\\'
#else
#define SEP '/'
#endif


void MakePathRelative(const char * cur_dir, CStr & path)
{
    const char * p = path.GetData();
    CStr         rel_path;

    while (*p && EQUAL_PATH_CHARS(*p, *cur_dir) )
    {
        p++;
        cur_dir++;
    }

    if (*p==SEP)
        p++;
    else
        rel_path << ".." << SEP;

    while (*cur_dir)
    {
        if (*cur_dir == SEP)
            rel_path << ".." << SEP;

        cur_dir++;
    }

    rel_path << p;

    if (path.GetLength() > rel_path.GetLength())
        path = rel_path;
}

//-------------------------------------------------------------------------

void MakePathFull(const char * cur_dir, CStr & path)
{
    CStr full_path;
    CStr rel_path;

    full_path = cur_dir;
    rel_path = path;

    if (!full_path.IsEmpty() && full_path.GetData()[full_path.GetLength()-1] != SEP)
        full_path.AddCh( SEP);

    if (!rel_path.IsEmpty())
    {
        if (rel_path.GetData()[0]=='.' && rel_path.GetData()[1]==SEP)
            rel_path.DelSubStr(0,2);
    }

    path = full_path;
    path << rel_path;
}

//-------------------------------------------------------------------------

void GetDirFromPath(const char * path, CStr & dir)
{
    int n = 0;
    const char * p;

    if (!path || !*path)
        return;

    dir = path;
    p   = dir.GetData() + (dir.GetLength()-1);
    while (*p!='\\' && *p!='/' && n<dir.GetLength())
    {
        p--;
        n++;
    }
    if (*p=='\\' || *p=='/')
        n++;

    if (n>0)
        dir.DelSubStr(dir.GetLength()-n, n);
    if (dir.IsEmpty())
        dir = ".";
}

//-------------------------------------------------------------------------

void GetFileFromPath(const char * path, CStr & file)
{
    const char * p = strrchr(path, SEP);

    file.Empty();
    if (p && *p)
    {
        p++;
        file = p;
    }
    else
        file = path;
}

//-------------------------------------------------------------------------
