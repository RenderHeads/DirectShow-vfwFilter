#ifdef _DEBUG
#pragma comment( lib, "strmbasd" )
#else
#pragma comment( lib, "strmbase" )
#endif

#pragma comment( lib, "strmiids" )
#pragma comment( lib, "winmm" )

#include <streams.h>
#include <initguid.h>
#include "VideoForWindowsFilter.h"
#include "PropertyPage.h"

//////////////////////////////////////////////////////////////////////////
// This file contains the standard COM glue code required for registering
// the filter 
//////////////////////////////////////////////////////////////////////////

#define g_wszFilterName L"VFW Wrapper"
#define g_wszFilterPropertyPageName L"VFW Wrapper Properties"

// Filter setup data
const AMOVIESETUP_MEDIATYPE sudPinTypes = { &MEDIATYPE_Video, &MEDIASUBTYPE_NULL};

const AMOVIESETUP_PIN sudFilterPins[] =
{
    { 
        L"Input",             // Pins string name
        FALSE,                // Is it rendered
        FALSE,                // Is it an output
        FALSE,                // Are we allowed none
        FALSE,                // And allowed many
        &CLSID_NULL,          // Connects to filter
        NULL,                 // Connects to pin
        1,                    // Number of types
        &sudPinTypes          // Pin information
    },
    { 
        L"Output",            // Pins string name
        FALSE,                // Is it rendered
        TRUE,                 // Is it an output
        FALSE,                // Are we allowed none
        FALSE,                // And allowed many
        &CLSID_NULL,          // Connects to filter
        NULL,                 // Connects to pin
        1,                    // Number of types
        &sudPinTypes          // Pin information
    }
};

const AMOVIESETUP_FILTER sudFilter =
{
    &CLSID_VfwVideoCompressionWrapper,	// Filter CLSID
    g_wszFilterName,					// String name
    MERIT_NORMAL,						// Filter merit
    2,									// Number of pins
    sudFilterPins						// Pin information
};

const REGFILTER2 rf2FilterReg = 
{
	1, // Version 1 (no pin mediums or pin category).
	MERIT_NORMAL, // Merit.
	2, // Number of pins.
	sudFilterPins // Pointer to pin information.
}; 

CFactoryTemplate g_Templates[] = 
{
    { g_wszFilterName, &CLSID_VfwVideoCompressionWrapper, VideoForWindowsFilter::CreateInstance, NULL, &sudFilter },
	{ g_wszFilterPropertyPageName, &CLSID_VfwVideoCompressionWrapper_Property, PropertyPage::CreateInstance, NULL, NULL }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);    

////////////////////////////////////////////////////////////////////////
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{

	HRESULT hr;
	IFilterMapper2 *pFM2 = NULL;

	hr = AMovieDllRegisterServer2(TRUE);
	if (FAILED(hr))
		return hr;

	hr = CoCreateInstance(CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER, IID_IFilterMapper2, (void **)&pFM2);

	if (FAILED(hr))
		return hr;

		hr = pFM2->RegisterFilter(
			CLSID_VfwVideoCompressionWrapper,		// Filter CLSID. 
			g_wszFilterName,						// Filter name.
			NULL,									// Device moniker. 
			&CLSID_VideoCompressorCategory,			// Video compressor category.
			g_wszFilterName,						// Instance data.
			&rf2FilterReg							// Pointer to filter information.
			);
	pFM2->Release();

	return hr;
}

STDAPI DllUnregisterServer()
{
    //return AMovieDllRegisterServer2( FALSE );

	HRESULT hr = S_OK;

	if (hr == S_OK)
	{
		hr = AMovieDllRegisterServer2(FALSE);
	}
	if (hr == S_OK)
	{
		IFilterMapper2 *pFM2 = NULL;
		hr = CoCreateInstance(CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER, IID_IFilterMapper2, (void **)&pFM2);
		if (hr == S_OK)
		{
			hr = pFM2->UnregisterFilter(&CLSID_VideoCompressorCategory, g_wszFilterName, CLSID_VfwVideoCompressionWrapper);
			pFM2->Release(); 
		}
	}

	return hr;
}

//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

