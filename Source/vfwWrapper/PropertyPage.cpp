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
#include <streams.h>
#include <commctrl.h>

#include "resource.h"

#include "VideoForWindowsFilter.h"
#include "PropertyPage.h"

void
PropertyPage::PopulateUI(HWND parentWindow)
{
	m_combo = GetDlgItem(parentWindow, IDC_COMBO1);
	m_aboutButton = GetDlgItem(parentWindow, IDC_BUTTONABOUT);
	m_configButton = GetDlgItem(parentWindow, IDC_BUTTONCONFIG);

	ComboBox_ResetContent(m_combo);

	const WCHAR* currentName = m_pHDvRtpOutFilter->GetFilterDescription();

	DWORD fccType = ICTYPE_VIDEO;
	int i;
	ICINFO icinfo;
	for (i=0; ICInfo(fccType, i, &icinfo); i++) 
	{ 
		HIC hic = ICOpen(icinfo.fccType, icinfo.fccHandler, ICMODE_COMPRESS); 
		if (hic) 
		{
			ICGetInfo(hic, &icinfo, sizeof(icinfo)); 

			ComboBox_AddString(m_combo, icinfo.szDescription);

			if (currentName != NULL)
			{
				if (_wcsicmp(icinfo.szDescription, currentName) == 0)
				{
					//ComboBox_SetCurSel(m_combo, ComboBox_GetCount(m_combo) - 1);
					ComboBox_SelectString(m_combo, 0, currentName);
					currentName = NULL;
				}
			}
			

			ICClose(hic);
		}
	}
}

PropertyPage::PropertyPage(LPUNKNOWN pUnk, HRESULT *phr) :
						CBasePropertyPage(NAME("HDV Send Filter Property Page"),
										  pUnk,
										  IDD_OLE_PROPPAGE, IDS_TITLE),
										  m_pHDvRtpOutFilter(NULL)
{
	ASSERT(phr);

	//InitCommonControls();
}

CUnknown * WINAPI PropertyPage::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
	CUnknown *lpunk = new PropertyPage(pUnk, phr);
	if (lpunk == NULL) {
		*phr = E_OUTOFMEMORY;
	}

	return lpunk;
}

void PropertyPage::SetDirty()
{
	m_bDirty = TRUE;
	if (m_pPageSite) 
	{
		m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
	}

	TCHAR sDestAddr[128];
	ComboBox_GetText(m_combo, sDestAddr, 128);
	EnableWindow(m_aboutButton, m_pHDvRtpOutFilter->HasAboutWindow(sDestAddr));
	EnableWindow(m_configButton, m_pHDvRtpOutFilter->HasConfigWindow(sDestAddr));
}

BOOL PropertyPage::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		PopulateUI(hwnd);
		return (LRESULT) 1;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BUTTONABOUT) 
		{
			TCHAR sDestAddr[128];
			ComboBox_GetText(m_combo, sDestAddr, 128);
			m_pHDvRtpOutFilter->ShowAboutWindow(sDestAddr, hwnd);
		}
		else if (LOWORD(wParam) == IDC_BUTTONCONFIG) 
		{
			TCHAR sDestAddr[128];
			ComboBox_GetText(m_combo, sDestAddr, 128);
			m_pHDvRtpOutFilter->ShowConfigWindow(sDestAddr, hwnd);
		}
		else if (LOWORD(wParam) == IDC_COMBO1) 
		{
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				SetDirty();
			}
		}
		return (LRESULT) 1;
	default:
		return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
	}

	return FALSE;
}

HRESULT PropertyPage::OnConnect(IUnknown *pUnknown)
{
	ASSERT(m_pHDvRtpOutFilter == NULL);

	// Store instance of the parent filter class
	HRESULT hr = pUnknown->QueryInterface(IID_VfwVideoCompressionWrapper, (void **) &m_pHDvRtpOutFilter);
	if (FAILED(hr)) {
		return E_NOINTERFACE;
	}

	ASSERT(m_pHDvRtpOutFilter);

	return NOERROR;
}

HRESULT PropertyPage::OnDisconnect()
{
	if (m_pHDvRtpOutFilter == NULL) {
		return E_UNEXPECTED;
	}

	m_pHDvRtpOutFilter->Release();
	m_pHDvRtpOutFilter = NULL;

	return NOERROR;
}

HRESULT PropertyPage::OnActivate()
{
	SetDirty();
	return NOERROR;
}

HRESULT PropertyPage::OnApplyChanges()
{
	m_bDirty = FALSE;

	TCHAR sDestAddr[128];
	ComboBox_GetText(m_combo, sDestAddr, 128);

	m_pHDvRtpOutFilter->InitFilterByDescription(sDestAddr);

	return NOERROR;
}