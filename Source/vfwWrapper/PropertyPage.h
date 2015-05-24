//
// VFW Video Compression Wrapper Filter for DirectShow
// Copyright (C) 2015 RenderHeads Ltd.
//
// This source is available for distribution and/or modification
// only under the terms of the MIT license.  All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the MIT license
// for more details.
//
#ifndef	PROPERTYPAGE_H
#define	PROPERTYPAGE_H

#include <commctrl.h>
#include <nspapi.h>

// {B09C43F2-65CC-476d-A4B5-7C1EA071905B}
DEFINE_GUID(CLSID_VfwVideoCompressionWrapper_Property, 
			0xb09c43f2, 0x65cc, 0x476d, 0xa4, 0xb5, 0x7c, 0x1e, 0xa0, 0x71, 0x90, 0x5b);

class VideoForWindowsFilter;

class PropertyPage : public CBasePropertyPage
{
public:
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);
	DECLARE_IUNKNOWN;

private:
	INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HRESULT OnConnect(IUnknown *pUnknown);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnApplyChanges();

	void SetDirty();

	HWND m_combo;
	HWND m_aboutButton;
	HWND m_configButton;

	void		PopulateUI(HWND parentWindow);

	PropertyPage(LPUNKNOWN pUnk, HRESULT *phr);

	IVideoForWindowsFilter *m_pHDvRtpOutFilter;
};

#endif