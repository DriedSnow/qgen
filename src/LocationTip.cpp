// Copyright (C) 2005-2009
// BaxZzZz (bauer_v AT mail DOT ru)
// Nex (nex AT otaku DOT ru)
// Shchannikov Dmitry (rrock DOT ru AT gmail DOT com)
// Valeriy Argunov (nporep AT mail DOT ru)
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

#include "LocationTip.h"

IMPLEMENT_CLASS(LocationTip, wxFrame);

LocationTip::LocationTip(wxWindow *parent, IControls *controls) : wxFrame(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(TIP_SIZE_X, TIP_SIZE_Y), wxFRAME_TOOL_WINDOW)
{
	_mainFrame = parent;
	_controls = controls;
	wxColour textColor(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
	wxColour backColor(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));

	SetBackgroundColour(backColor);
	wxBoxSizer *_sizer = new wxBoxSizer(wxVERTICAL);
	_sizer->SetMinSize(TIP_SIZE_X, TIP_SIZE_Y);

	_title = new wxStaticText(this, wxID_ANY, wxEmptyString);
	_title->SetFont(_title->GetFont().MakeBold().MakeLarger());
	_title->SetForegroundColour(textColor);

	_desc = new wxStaticText(this, wxID_ANY, wxT("��������:"));
	_desc->SetFont(_desc->GetFont().MakeBold());
	_desc->SetForegroundColour(textColor);
	_locDesc = new SyntaxTextBox(this, _controls, NULL, SYNTAX_STYLE_NOSCROLLBARS | SYNTAX_STYLE_SIMPLE | SYNTAX_STYLE_NOHOTKEYS | SYNTAX_STYLE_SIMPLEMENU);

	_code = new wxStaticText(this, wxID_ANY, wxEmptyString);
	_code->SetFont(_code->GetFont().MakeBold());
	_code->SetForegroundColour(textColor);
	_locCode = new SyntaxTextBox(this, _controls, NULL, SYNTAX_STYLE_NOSCROLLBARS | SYNTAX_STYLE_COLORED | SYNTAX_STYLE_NOHOTKEYS | SYNTAX_STYLE_SIMPLEMENU | SYNTAX_STYLE_NOMARGINS);

	_emptyLabel = new wxButton(this, wxID_ANY, wxT("(�����)"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	_emptyLabel->SetFont(_emptyLabel->GetFont().MakeLarger().MakeLarger());
	_emptyLabel->SetBackgroundColour(backColor);
	_emptyLabel->SetForegroundColour(textColor);

	_sizer->Add(_title, 0, wxALL|wxALIGN_CENTER, 4);
	_sizer->Add(_desc, 0, wxLEFT|wxRIGHT|wxGROW, 4);
	_sizer->Add(_locDesc, 1, wxALL|wxGROW, 4);
	_sizer->Add(_code, 0, wxLEFT|wxRIGHT|wxGROW, 4);
	_sizer->Add(_locCode, 1, wxALL|wxGROW, 4);
	_sizer->Add(_emptyLabel, 1, wxGROW);

	SetSizer(_sizer);
	SetAutoLayout( true );
	Layout();
}

void LocationTip::MoveTip(const wxPoint &pos)
{
	wxPoint position(pos);
	LoadTip();
	wxRect displaySize(wxGetClientDisplayRect());
	if (displaySize.GetHeight() <= (position.y + TIP_SIZE_Y))
		position.y -= TIP_SIZE_Y;
	else
		position.y += 20;
	if (displaySize.GetWidth() <= (position.x + TIP_SIZE_X))
		position.x -= TIP_SIZE_X;
	else
		position.x += 5;
	Move(position.x, position.y);
	if (!IsShown())
	{
		if (CanSetTransparent())
			SetTransparent(255);
		Show();
	}
}

void LocationTip::HideTip()
{
	if (IsShown())
	{
		if (CanSetTransparent())
		{
			for (int trans = 255; trans > 0; trans -= 5)
				SetTransparent(trans);
		}
		Hide();
	}
	_locName.Clear();
	_actName.Clear();
}

void LocationTip::LoadTip()
{
	bool hasDesc, hasCode;
	wxSizer *_sizer = GetSizer();
	DataContainer *container = _controls->GetContainer();
	size_t locIndex = container->FindLocationIndex(_locName);
	_sizer->Hide(_emptyLabel);
	if (_actName.IsEmpty())
	{
		_title->SetLabel(_locName);
		wxString descText(container->GetLocationDesc(locIndex));
		hasDesc = !descText.IsEmpty();
		_sizer->Show(_desc, hasDesc);
		_sizer->Show(_locDesc, hasDesc);
		_locDesc->SetValue(descText);
		_code->SetLabel(wxT("��� �������:"));
		wxString codeText(container->GetLocationCode(locIndex));
		hasCode = !codeText.IsEmpty();
		_sizer->Show(_code, hasCode);
		_sizer->Show(_locCode, hasCode);
		_locCode->SetValue(codeText);
		if (!(hasCode || hasDesc)) _sizer->Show(_emptyLabel);
	}
	else
	{
		_title->SetLabel(_actName);
		size_t actIndex = container->FindActionIndex(locIndex, _actName);
		wxString actCode(container->GetActionCode(locIndex, actIndex));
		hasCode = !actCode.IsEmpty();
		_sizer->Hide(_desc);
		_sizer->Hide(_locDesc);
		_sizer->Show(_code, hasCode);
		_sizer->Show(_locCode, hasCode);
		_code->SetLabel(wxT("��� ��������:"));
		_locCode->SetValue(actCode);
		if (!hasCode) _sizer->Show(_emptyLabel);
	}

	Layout();
}

void LocationTip::SetLocName( const wxString &name )
{
	_locName = name;
}

void LocationTip::SetActName( const wxString &name )
{
	_actName = name;
}
