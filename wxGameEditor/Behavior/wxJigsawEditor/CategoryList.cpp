/////////////////////////////////////////////////////////////////////////////
// Name:        CategoryList.cpp
// Purpose:     
// Author:      Volodymir (T-Rex) Tryapichko
// Modified by: 
// Created:     30/11/2008 15:16:31
// RCS-ID:      
// Copyright:   Volodymir (T-Rex) Tryapichko, 2008
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "CategoryList.h"
#include "CategoryButton.h"
#include <wxJigsawShape.h>

////@begin XPM images
////@end XPM images


/*!
 * CategoryList type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CategoryList, wxWindow )


/*!
 * CategoryList event table definition
 */

BEGIN_EVENT_TABLE( CategoryList, wxWindow )

////@begin CategoryList event table entries
    EVT_SIZE( CategoryList::OnSize )

////@end CategoryList event table entries

END_EVENT_TABLE()


/*!
 * CategoryList constructors
 */

CategoryList::CategoryList()
{
    Init();
}

CategoryList::CategoryList(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CategoryList creator
 */

bool CategoryList::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
////@begin CategoryList creation
    wxWindow::Create(parent, id, pos, size, style);
    CreateControls();
////@end CategoryList creation
    return true;
}


/*!
 * CategoryList destructor
 */

CategoryList::~CategoryList()
{
	//DestroyChildren();
////@begin CategoryList destruction
////@end CategoryList destruction
}


/*!
 * Member initialisation
 */

void CategoryList::Init()
{
////@begin CategoryList member initialisation
////@end CategoryList member initialisation
}


/*!
 * Control creation for CategoryList
 */

void CategoryList::CreateControls()
{    
////@begin CategoryList content construction
////@end CategoryList content construction
}


/*!
 * Should we show tooltips?
 */

bool CategoryList::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CategoryList::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CategoryList bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CategoryList bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CategoryList::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CategoryList icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CategoryList icon retrieval
}


/*!
 * wxEVT_SIZE event handler for ID_CATEGORYLIST
 */

void CategoryList::OnSize( wxSizeEvent& event )
{
	ResizeButtons();
	ReCreateScrollBars();
}

void CategoryList::ResizeButtons()
{
	int y = 5;
	int w = GetClientSize().GetWidth();
	for(wxWindowList::Node * node = GetChildren().GetFirst(); node; node = node->GetNext())
	{
		wxWindow * window = node->GetData();
		if(!window) continue;
		if(!wxIsKindOf(window, CategoryButton)) continue;
		int h = window->GetSize().GetHeight();
		window->SetSize(5, y, w-10, h);
		y += (h+5);
	}
}

void CategoryList::DestroyChildren()
{
	for(wxWindowList::Node * node = GetChildren().GetFirst(); node; node = node->GetNext())
	{
		node->GetData()->Destroy();
	}
}

void CategoryList::AddCategory(const wxString & name, wxJigsawShapeList * shapes)
{
	long newButtonID = wxNewId();
	CategoryButton * newButton = new CategoryButton(this, newButtonID, name);
	newButton->SetShapes(shapes);
	Connect(newButtonID, wxEVT_COMMAND_BUTTON_CLICKED, 
		wxCommandEventHandler(CategoryList::OnCategoryButtonPressed));
	ResizeButtons();
	ReCreateScrollBars();
	if(GetCategoryCount() == 1)
	{
		wxCommandEvent e(wxEVT_COMMAND_BUTTON_CLICKED, newButtonID);
		ProcessEvent(e);
	}
}

size_t CategoryList::GetCategoryCount()
{
	size_t result(0);
	for(wxWindowList::Node * node = GetChildren().GetFirst(); node; node = node->GetNext())
	{
		if(node->GetData()->IsKindOf(CLASSINFO(CategoryButton)))
		{
			result++;
		}
	}
	return result;
}

void CategoryList::OnCategoryButtonPressed(wxCommandEvent & event)
{
	CategoryButton * button = wxDynamicCast(wxWindow::FindWindowById(event.GetId(), this),
		CategoryButton);
	if(button)
	{
		wxCommandEvent e(wxEVT_COMMAND_LISTBOX_SELECTED, GetId());
		e.SetClientData(button->GetShapes());
		GetEventHandler()->AddPendingEvent(e);
	}
}

void CategoryList::ReCreateScrollBars()
{
	int clientHeight = GetClientSize().GetHeight();
	int maxY = clientHeight;
	for(wxWindowList::Node * node = GetChildren().GetFirst(); node; node = node->GetNext())
	{
		maxY  = wxMax(maxY, node->GetData()->GetPosition().y+node->GetData()->GetSize().GetHeight());
	}
	maxY += (maxY <= clientHeight ? 0 : 5);
	SetScrollbar(wxVERTICAL, 0, clientHeight, maxY);
}