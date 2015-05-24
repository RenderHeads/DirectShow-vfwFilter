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

#include <windows.h>
#include <streams.h>
#include <dvdmedia.h>
#include <initguid.h>
#include "VideoForWindowsFilter.h"
#include <aviriff.h>

// Provide the way for COM to create a VideoForWindowsFilter object
CUnknown*
VideoForWindowsFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
	return new VideoForWindowsFilter(NAME("VFW Video Compression Wrapper"), punk, phr);
}

VideoForWindowsFilter::VideoForWindowsFilter(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr) : CTransformFilter(tszName, punk, CLSID_VfwVideoCompressionWrapper)
{ 
	_hasStarted = false;
	_frameNumber = 0;
	_maxCompressedFrameSize = 0;
	_hic = NULL;

	//_hic = FindCompressor(L"Lagarith Lossless Codec");
	//InitFilter(_hic);
}

VideoForWindowsFilter::~VideoForWindowsFilter()
{
	if (_hic != NULL)
	{
		ICClose(_hic);
		_hic = NULL;
	}
}

HIC
VideoForWindowsFilter::FindCompressor(const WCHAR* desc) const
{
	HIC result = NULL;
	DWORD fccType = ICTYPE_VIDEO;
	int i;
	ICINFO icinfo; 
	for (i=0; ICInfo(fccType, i, &icinfo); i++) 
	{ 
		HIC hic = ICOpen(icinfo.fccType, icinfo.fccHandler, ICMODE_COMPRESS);//QUERY); 
		if (hic) 
		{
			ICGetInfo(hic, &icinfo, sizeof(icinfo)); 

			if (_wcsicmp(icinfo.szDescription, desc) == 0 )
			{
				result = hic;
				break;
			}

			ICClose(hic);
		}
	}

	return result;
}

void 
VideoForWindowsFilter::InitFilterByDescription(const WCHAR* desc)
{
	if (desc != NULL)
	{
		_hic = FindCompressor(desc);
		if (_hic != NULL)
		{
			_codecDescription = desc;
		}
	}
}

const WCHAR*
VideoForWindowsFilter::GetFilterDescription() const
{
	return _codecDescription.c_str();
}

bool
VideoForWindowsFilter::HasAboutWindow(const WCHAR* desc) const
{
	if (desc != NULL)
	{
		HIC hic = FindCompressor(desc);
		if (hic != NULL)
		{
			return ICQueryAbout(hic);
		}
	}
	return false;
}

bool
VideoForWindowsFilter::HasConfigWindow(const WCHAR* desc) const
{
	if (desc != NULL)
	{
		HIC hic = FindCompressor(desc);
		if (hic != NULL)
		{
			return ICQueryConfigure(hic);
		}
	}
	return false;
}

void
VideoForWindowsFilter::ShowAboutWindow(const WCHAR* desc, HWND parentWindow)
{
	if (desc != NULL)
	{
		HIC hic = FindCompressor(desc);
		if (hic != NULL)
		{
			if (ICQueryAbout(hic))
			{
				ICAbout(hic, parentWindow);
			}
		}
	}
}

void
VideoForWindowsFilter::ShowConfigWindow(const WCHAR* desc, HWND parentWindow)
{
	if (desc != NULL)
	{
		HIC hic = FindCompressor(desc);
		if (hic != NULL)
		{
			if (ICQueryConfigure(hic))
			{
				ICConfigure(hic, parentWindow);
			}
		}
	}
}

bool
VideoForWindowsFilter::InitFilter(HIC hic)
{
	if (hic == NULL)
		return false;

	_hic = hic;

	return true;
}

HRESULT
VideoForWindowsFilter::EndOfStream()
{
	if (_hasStarted)
	{
		ICCompressEnd(_hic); 
		_hasStarted = false;
	}

	return CTransformFilter::EndOfStream();
}

HRESULT
VideoForWindowsFilter::Transform(IMediaSample *pSource, IMediaSample *pDest)
{
	HRESULT hr;
	// Get pointers to the underlying buffers.
	BYTE *pBufferIn, *pBufferOut;
	hr = pSource->GetPointer(&pBufferIn);
	if (FAILED(hr))
	{
		return hr;
	}
	hr = pDest->GetPointer(&pBufferOut);
	if (FAILED(hr))
	{
		return hr;
	}

	CMediaType *inMediaType = &m_pInput->CurrentMediaType();
	VIDEOINFOHEADER *vihIn = (VIDEOINFOHEADER*)inMediaType->Format();

	CMediaType *outMediaType = &m_pOutput->CurrentMediaType();
	VIDEOINFOHEADER *vihOut = (VIDEOINFOHEADER*)outMediaType->Format();

	BITMAPINFOHEADER* lpbiIn = &vihIn->bmiHeader;
	BITMAPINFOHEADER* lpbiOut = &vihOut->bmiHeader;

	
	if (!_hasStarted)
	{
		if (ICERR_OK != ICCompressBegin(_hic, &vihIn->bmiHeader, &vihOut->bmiHeader))
		{
			return S_FALSE;
		}

		_frameNumber = 0;
		_hasStarted = true;
	}

	lpbiOut->biSizeImage = (DWORD)_maxCompressedFrameSize;

	DWORD dwCkID = 0;
	DWORD dwCompFlags = AVIIF_KEYFRAME;
	DWORD dwQuality = 0;
	if (ICCompress(_hic, ICCOMPRESS_KEYFRAME, lpbiOut, pBufferOut, lpbiIn, pBufferIn, &dwCkID, &dwCompFlags, _frameNumber, 0, dwQuality, NULL, NULL) == ICERR_OK)
	{
		dwCkID = dwCkID;
	}
	else
	{
		return S_FALSE;
	}


	_frameNumber++;

	LONGLONG MediaStart, MediaEnd;
	REFERENCE_TIME TimeStart, TimeEnd;

	pSource->GetTime(&TimeStart, &TimeEnd);
	pSource->GetMediaTime(&MediaStart,&MediaEnd);

	pDest->SetTime(&TimeStart, &TimeEnd);
	//pDest->SetMediaTime(&MediaStart,&MediaEnd);

	pDest->SetActualDataLength(lpbiOut->biSizeImage);
	pDest->SetSyncPoint(TRUE);
	pDest->SetDiscontinuity(pSource->IsDiscontinuity() == S_OK);

	return S_OK;
}

HRESULT 
VideoForWindowsFilter::CheckInputType(const CMediaType *mtIn)
{
	// check this is a VIDEOINFOHEADER type and is supported by the codec
	if( *mtIn->FormatType() == FORMAT_VideoInfo)
	{
		BITMAPINFOHEADER *pbih = &((VIDEOINFOHEADER*)mtIn->Format())->bmiHeader;

		// VFW filters can't handle negative height values (used to denote vertical orientation
		// either top-bottom or bottom-top).  We could handle the flip ourselves in this filter
		// but we will instead not allow connection which should allow the Color Converter
		// filter to be used to handle the flip.
		if (pbih->biHeight < 0)
			return VFW_E_INVALID_MEDIA_TYPE;

		if (ICERR_OK == ICCompressQuery(_hic, pbih, NULL))
		{
			return S_OK;
		}
	}
	return VFW_E_INVALID_MEDIA_TYPE;
}

// Check a transform can be done between these formats
HRESULT
VideoForWindowsFilter::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
	if( *mtIn->FormatType() == FORMAT_VideoInfo && 
		*mtOut->FormatType() == FORMAT_VideoInfo)
	{
		BITMAPINFOHEADER *pbih = &((VIDEOINFOHEADER*)mtIn->Format())->bmiHeader;
		BITMAPINFOHEADER *pboh = &((VIDEOINFOHEADER*)mtOut->Format())->bmiHeader;
		if (ICERR_OK == ICCompressQuery(_hic, pbih, pboh))
		{
			return S_OK;
		}
	}

	return VFW_E_INVALID_MEDIA_TYPE;
}

// Tell the output pin's allocator what size buffers we
// require. Can only do this when the input is connected
HRESULT
VideoForWindowsFilter::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
	// Is the input pin connected
	if (!m_pInput->IsConnected()) 
		return E_UNEXPECTED;

	HRESULT hr = NOERROR;

	CMediaType *inMediaType = &m_pInput->CurrentMediaType();
	VIDEOINFOHEADER *vihIn = (VIDEOINFOHEADER *)inMediaType->Format();

	CMediaType *outMediaType = &m_pOutput->CurrentMediaType();
	VIDEOINFOHEADER *vihOut = (VIDEOINFOHEADER*)outMediaType->Format();

	if (pProperties->cBuffers == 0)
	{
		pProperties->cBuffers = 4;
	}
	_maxCompressedFrameSize = ICCompressGetSize(_hic, &vihIn->bmiHeader, &vihOut->bmiHeader); 
	pProperties->cbBuffer = _maxCompressedFrameSize;	
	pProperties->cbAlign = 16;	// SSE2 memory alignment requirements

	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(pProperties,&Actual);
	if (SUCCEEDED(hr))
	{
		// Is this allocator unsuitable?
		if (pProperties->cBuffers > Actual.cBuffers || 
			pProperties->cbBuffer > Actual.cbBuffer) 
		{
			hr = E_FAIL;
		}
	}

	return hr;
}

// Returns the supported media types in order of preferred  types (starting with iPosition=0)
HRESULT
VideoForWindowsFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	// Is the input pin connected
	if (!m_pInput->IsConnected()) 
		return E_UNEXPECTED;

	if (iPosition < 0)
		return E_INVALIDARG;

	// Do we have more items to offer
	if (iPosition > 0)
		return VFW_S_NO_MORE_ITEMS;


	// get input dimensions
	CMediaType *inMediaType = &m_pInput->CurrentMediaType();
	VIDEOINFOHEADER *vihIn = (VIDEOINFOHEADER*)inMediaType->Format();
	pMediaType->SetFormatType(&FORMAT_VideoInfo);
	pMediaType->SetType(&MEDIATYPE_Video);
	

	
	pMediaType->SetTemporalCompression(TRUE);


	DWORD bmStructSize = ICCompressGetFormatSize(_hic, &vihIn->bmiHeader);
	DWORD vihSize = sizeof(VIDEOINFOHEADER) - sizeof(BITMAPINFOHEADER) + bmStructSize;
	VIDEOINFOHEADER *vihOut = (VIDEOINFOHEADER *)pMediaType->ReallocFormatBuffer(vihSize);


	

	// set VIDEOINFOHEADER
	memset(vihOut, 0, vihSize);



	if (ICERR_OK != ICCompressGetFormat(_hic, &vihIn->bmiHeader, &vihOut->bmiHeader))
		return S_FALSE;

	//vihOut->bmiHeader.biHeight = abs(vihOut->bmiHeader.biHeight);

	DWORD fourcc = vihOut->bmiHeader.biCompression;
	fourcc = (fourcc & 0x000000ff) << 24 | (fourcc & 0x0000ff00) << 8 | (fourcc & 0x00ff0000) >> 8 | (fourcc & 0xff000000) >> 24;
	GUID g2 = (GUID)FOURCCMap(FCC(fourcc));
	pMediaType->SetSubtype(&g2);

	DWORD maxSize = ICCompressGetSize(_hic, &vihIn->bmiHeader, &vihOut->bmiHeader);
	//DWORD maxSize = vihOut->bmiHeader.biHeight * vihOut->bmiHeader.biWidth * 4;

	vihOut->bmiHeader.biSizeImage = maxSize;
	double frameRate = UNITS / (double)vihIn->AvgTimePerFrame;
	vihOut->dwBitRate = (int)(frameRate * vihOut->bmiHeader.biSizeImage * 8);
	vihOut->AvgTimePerFrame = vihIn->AvgTimePerFrame;
	pMediaType->SetSampleSize(vihOut->bmiHeader.biSizeImage);
	pMediaType->SetVariableSize();

	return NOERROR;
}

STDMETHODIMP
VideoForWindowsFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_ISpecifyPropertyPages) 
	{
		return GetInterface((ISpecifyPropertyPages *) this, ppv);
	}
	if (riid == IID_VfwVideoCompressionWrapper)
	{
		return GetInterface((IVideoForWindowsFilter *) this, ppv);
	}

	return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
}

#include "PropertyPage.h"


HRESULT STDMETHODCALLTYPE
VideoForWindowsFilter::GetPages(CAUUID *pPages)
{
	if (pPages == NULL)
		return E_POINTER;

	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID) * pPages->cElems);
	if (pPages->pElems == NULL) 
	{
		return E_OUTOFMEMORY;
	}

	pPages->pElems[0] = CLSID_VfwVideoCompressionWrapper_Property;

	return S_OK;
}