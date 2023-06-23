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
#include "listpane.h"
#include "unitpane.h"
#include "editpane.h"
#include "unitframe.h"

BEGIN_EVENT_TABLE(CUnitFrame, CAhFrame)
    EVT_MENU   (menu_SaveOrders         , CUnitFrame::OnSaveOrders)
    EVT_MENU   (accel_NextUnit          , CUnitFrame::OnNextUnit)
    EVT_MENU   (accel_PrevUnit          , CUnitFrame::OnPrevUnit)
    EVT_MENU   (accel_UnitList          , CUnitFrame::OnUnitList)
    EVT_MENU   (accel_Orders            , CUnitFrame::OnOrders  )
    EVT_CLOSE  (                          CUnitFrame::OnCloseWindow)
END_EVENT_TABLE()


//--------------------------------------------------------------------

CUnitFrame::CUnitFrame(wxWindow * parent)
           :CAhFrame (parent, "Units (hex)", (wxDEFAULT_FRAME_STYLE | wxCLIP_CHILDREN) & ~wxMINIMIZE_BOX)
{
    m_Splitter1 = NULL;
    m_Splitter2= NULL;
    m_Splitter3= NULL;
}

//--------------------------------------------------------------------

void CUnitFrame::Init(const char * szConfigSection)
{
    gpApp->GetListColSection(SZ_SECT_LIST_COL_UNIT, SZ_KEY_LIS_COL_UNITS_HEX);
    CAhFrame::Init("");
}

//--------------------------------------------------------------------

void CUnitFrame::Done(BOOL SetClosedFlag)
{
    CUnitPane         * pUnitPane;

    pUnitPane  = (CUnitPane*)gpApp->m_Panes[AH_PANE_UNITS_HEX];
    if (pUnitPane)
        pUnitPane->Done();

    CAhFrame::Done(SetClosedFlag);
}


//--------------------------------------------------------------------


void CUnitFrame::OnCloseWindow(wxCloseEvent& )
{
    gpApp->FrameClosing(this);
    Destroy();
}

//--------------------------------------------------------------------

