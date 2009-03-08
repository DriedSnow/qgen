// Copyright (C) 2005-2009 Valeriy Argunov (nporep AT mail DOT ru)
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "LocationsListBox.h"
#include "wx/arrimpl.cpp"

WX_DEFINE_OBJARRAY(wxTreeItemIdArray);

IMPLEMENT_CLASS(LocationsListBox, wxTreeCtrl)

BEGIN_EVENT_TABLE(LocationsListBox, wxTreeCtrl)
	EVT_RIGHT_DOWN(LocationsListBox::OnRightClick)
	EVT_LEFT_DCLICK(LocationsListBox::OnDoubleClick)
	EVT_TREE_END_LABEL_EDIT(wxID_ANY, LocationsListBox::OnEndLabelEdit)
	EVT_TREE_BEGIN_DRAG(wxID_ANY, LocationsListBox::OnBeginDrag)
	EVT_TREE_END_DRAG(wxID_ANY, LocationsListBox::OnEndDrag)
END_EVENT_TABLE()

LocationsListBox::LocationsListBox(wxWindow *parent, wxWindowID id, IControls *controls, long style) :
	wxTreeCtrl(parent, id, wxDefaultPosition, wxDefaultSize, style)
{
	_controls = controls;

	_statesImageList.Create(15, 15);
	_statesImageList.Add(wxBitmap(location_closed_xpm));
	_statesImageList.Add(wxBitmap(location_xpm));
	_statesImageList.Add(wxBitmap(action_xpm));

	AddRoot(wxT("Locs"));
	Update();
	_controls->GetSettings()->AddObserver(this);
}

LocationsListBox::~LocationsListBox()
{
	Clear();
	_controls->GetSettings()->RemoveObserver(this);
}

void LocationsListBox::Update(bool isFromObservable)
{
	Settings *settings = _controls->GetSettings();
	SetFont(settings->GetFont(SYNTAX_BASE));
	SetForegroundColour(settings->GetColour(SYNTAX_BASE));
	SetBackgroundColour(settings->GetBackColour());
	ApplyStatesImageList();
	if (isFromObservable)
	{
		_controls->UpdateLocationsList();
		_controls->ShowOpenedLocationsIcons();
	}
}

bool LocationsListBox::IsItemOk( wxTreeItemId id, int flags )
{
	return id.IsOk() &&
		(flags == wxTREE_HITTEST_ONITEMINDENT ||
		 flags == wxTREE_HITTEST_ONITEMICON ||
		 flags == wxTREE_HITTEST_ONITEMLABEL ||
		 flags == wxTREE_HITTEST_ONITEMRIGHT);
}

void LocationsListBox::OnRightClick( wxMouseEvent &event )
{
	wxMenu menu;
	int flags;
	wxTreeItemId id = HitTest(event.GetPosition(), flags);
	if (IsItemOk(id, flags))
	{
		SetFocus();
		SelectItem(id);
	}
	menu.Append(LOC_CREAT, wxT("�������..."));
	menu.Append(LOC_RENAME, wxT("�������������..."));
	menu.Append(LOC_DEL, wxT("�������"));
	menu.AppendSeparator();
	menu.Append(LOC_COPY, wxT("����������"));
	menu.Append(LOC_PASTE, wxT("��������"));
	menu.Append(LOC_REPLACE, wxT("��������"));
	menu.Append(LOC_PASTE_NEW, wxT("�������� �..."));
	menu.Append(LOC_CLEAR, wxT("��������"));
	menu.AppendSeparator();
	menu.Append(LOC_SORT_ASC, wxT("����������� �� ��������"));
	menu.Append(LOC_SORT_DESC, wxT("����������� � �������� �������"));
	menu.AppendSeparator();
	menu.Append(LOC_EXPAND, wxT("���������� ���"));
	menu.Append(LOC_COLLAPSE, wxT("�������� ���"));
	_controls->UpdateMenuItems(&menu);
	PopupMenu(&menu);
}

void LocationsListBox::OnDoubleClick(wxMouseEvent &event )
{
	int flags;
	wxTreeItemId id = HitTest(event.GetPosition(), flags);
	if (IsItemOk(id, flags))
	{
		if (GetItemParent(id) == GetRootItem())
			_controls->ShowLocation(GetItemText(id));
		else
		{
			LocationPage *page = _controls->ShowLocation(GetItemText(GetItemParent(id)));
			page->SelectAction(_controls->GetContainer()->FindActionIndex(page->GetLocationIndex(), GetItemText(id)));
		}
	}
	else
		event.Skip();
}

void LocationsListBox::Insert(const wxString &text, size_t pos)
{
	if (_controls->GetSettings()->GetShowLocsIcons())
		_items.Insert(InsertItem(GetRootItem(), pos, text, ICON_NOTACTIVELOCATION), pos);
	else
		_items.Insert(InsertItem(GetRootItem(), pos, text), pos);
}

void LocationsListBox::Select( size_t index )
{
	SelectItem(_items[index]);
}

wxString LocationsListBox::GetString( size_t index )
{
	return GetItemText(_items[index]);
}

wxString LocationsListBox::GetStringSelection()
{
	wxTreeItemId id(GetSelection());
	if (id.IsOk())
	{
		if (GetItemParent(id) == GetRootItem())
			return GetItemText(id);
		else
			return GetItemText(GetItemParent(id));
	}
	else
		return wxEmptyString;
}

long LocationsListBox::GetStringIndex( const wxString &text )
{
	for (size_t i = 0; i < _items.GetCount(); ++i)
		if (GetItemText(_items[i]) == text) return (long)i;
	return wxNOT_FOUND;
}

void LocationsListBox::Clear()
{
	_items.Clear();
	Freeze();
	DeleteChildren(GetRootItem());
	Thaw();
}

void LocationsListBox::UpdateLocationActions( size_t locIndex, const wxArrayString & actions )
{
	size_t i, count = actions.GetCount();
	wxTreeItemId id = _items[locIndex];
	DeleteChildren(id);
	if (_controls->GetSettings()->GetShowLocsIcons())
		for (i = 0; i < count; ++i) AppendItem(id, actions[i], ICON_ACTION);
	else
		for (i = 0; i < count; ++i) AppendItem(id, actions[i]);
}

size_t LocationsListBox::GetCount()
{
	return _items.GetCount();
}

void LocationsListBox::Delete( size_t index )
{
	wxTreeCtrl::Delete(_items[index]);
	_items.RemoveAt(index);
}

void LocationsListBox::SetString( size_t index, const wxString & text )
{
	SetItemText(_items[index], text);
}

void LocationsListBox::ExpandItems()
{
	if (!_items.GetCount()) return;
	Freeze();
	for (size_t i = 0; i < _items.GetCount(); ++i)
		ExpandAllChildren(_items[i]);
	ScrollTo(_items[0]);
	Thaw();
}

void LocationsListBox::CollapseItems()
{
	if (!_items.GetCount()) return;
	Freeze();
	for (size_t i = 0; i < _items.GetCount(); ++i)
		CollapseAllChildren(_items[i]);
	ScrollTo(_items[0]);
	Thaw();
}

void LocationsListBox::OnEndLabelEdit( wxTreeEvent &event )
{
	if (event.IsEditCancelled()) return;
	wxTreeItemId id(event.GetItem());
	wxString label(event.GetLabel());
	wxString oldLabel(GetItemText(id));
	DataContainer *container = _controls->GetContainer();
	if (GetItemParent(id) == GetRootItem()) // �������
	{
		label = label.Trim().Trim(false);
		if (label.IsEmpty())
			_controls->ShowMessage( QGEN_MSG_EMPTYDATA );
		else
			_controls->RenameLocation(container->FindLocationIndex(oldLabel), label);
	}
	else // ��������
	{
		if (label.IsEmpty())
			_controls->ShowMessage( QGEN_MSG_EMPTYDATA );
		else
		{
			size_t locIndex = container->FindLocationIndex(GetItemText(GetItemParent(id)));
			_controls->RenameAction(locIndex, container->FindActionIndex(locIndex, oldLabel), label);
		}
	}
	event.Veto();
}

void LocationsListBox::OnBeginDrag( wxTreeEvent &event )
{
	wxTreeItemId id(event.GetItem());
	if (GetItemParent(id) == GetRootItem()) // �������
	{
		_draggedLocIndex = GetStringIndex(GetItemText(id));
		_draggedActIndex = wxNOT_FOUND;
	}
	else // ��������
	{
		_draggedLocIndex = GetStringIndex(GetItemText(GetItemParent(id)));
		_draggedActIndex = _controls->GetContainer()->FindActionIndex(_draggedLocIndex, GetItemText(id));
	}
	SetImageList(&_statesImageList);
	event.Allow();
}

void LocationsListBox::OnEndDrag( wxTreeEvent &event )
{
	ApplyStatesImageList();
	wxTreeItemId id(event.GetItem());
	if (!id.IsOk()) return;
	if (_draggedActIndex < 0)
	{
		if (GetItemParent(id) == GetRootItem()) // �������
			_controls->MoveLocationTo(_draggedLocIndex, GetStringIndex(GetItemText(id)));
	}
	else
	{
		if (GetItemParent(id) != GetRootItem()) // ��������
		{
			if (GetStringIndex(GetItemText(GetItemParent(id))) == _draggedLocIndex)
				_controls->MoveActionTo(_draggedLocIndex, _draggedActIndex, _controls->GetContainer()->FindActionIndex(_draggedLocIndex, GetItemText(id)));
		}
	}
}

void LocationsListBox::MoveItemTo( size_t locIndex, size_t moveTo )
{
	wxString label(GetString(locIndex));
	int image = GetItemImage(_items[locIndex]);
	Delete(locIndex);
	_items.Insert(InsertItem(GetRootItem(), moveTo, label, image), moveTo);
	Select(moveTo);
}

void LocationsListBox::SetLocStatus( size_t locIndex, bool isOpened )
{
	if (_controls->GetSettings()->GetShowLocsIcons())
		SetItemImage(_items[locIndex], isOpened ? ICON_ACTIVELOCATION : ICON_NOTACTIVELOCATION);
}

void LocationsListBox::ApplyStatesImageList()
{
	if (_controls->GetSettings()->GetShowLocsIcons())
		SetImageList(&_statesImageList);
	else
		SetImageList(NULL);
}
