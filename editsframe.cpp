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

#include "wx/listctrl.h"
#include "wx/splitter.h"

#include "cstr.h"
#include "collection.h"
#include "files.h"
#include "atlaparser.h"
#include "consts.h"
#include "consts_ah.h"
#include "hash.h"

#include "ahapp.h"
#include "ahframe.h"
#include "editpane.h"
#include "editsframe.h"

BEGIN_EVENT_TABLE(CEditsFrame, CAhFrame)
    EVT_MENU   (menu_SaveOrders         , CEditsFrame::OnSaveOrders)
    EVT_MENU   (accel_NextUnit          , CEditsFrame::OnNextUnit)
    EVT_MENU   (accel_PrevUnit          , CEditsFrame::OnPrevUnit)
    EVT_MENU   (accel_UnitList          , CEditsFrame::OnUnitList)
    EVT_MENU   (accel_Orders            , CEditsFrame::OnOrders  )
    EVT_CLOSE  (                          CEditsFrame::OnCloseWindow)
END_EVENT_TABLE()


//--------------------------------------------------------------------

CEditsFrame::CEditsFrame(wxWindow * parent)
            :CAhFrame (parent, "Editor panes", (wxDEFAULT_FRAME_STYLE | wxCLIP_CHILDREN) & ~wxMINIMIZE_BOX)
{
    m_Splitter1= NULL;
    m_Splitter2= NULL;
    m_Splitter3= NULL;
}


//--------------------------------------------------------------------

void CEditsFrame::Init(const char * szConfigSection)
{
    //TODO ARKADY
}

//--------------------------------------------------------------------

void CEditsFrame::Done(BOOL SetClosedFlag)
{
    CAhFrame::Done(SetClosedFlag);
}


//--------------------------------------------------------------------


void CEditsFrame::OnCloseWindow(wxCloseEvent& )
{
    gpApp->FrameClosing(this);
    Destroy();
}

//--------------------------------------------------------------------

