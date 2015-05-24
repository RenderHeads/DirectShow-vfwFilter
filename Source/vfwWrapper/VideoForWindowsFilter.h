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
#ifndef VIDEOFORWINDOWSFILTER_H
#define VIDEOFORWINDOWSFILTER_H

#include <vfw.h>
#include <string>

// {EFE52A34-FF3F-475e-AEA5-E67CD04A9715}
DEFINE_GUID(CLSID_VfwVideoCompressionWrapper, 
			0xefe52a34, 0xff3f, 0x475e, 0xae, 0xa5, 0xe6, 0x7c, 0xd0, 0x4a, 0x97, 0x15);

// {91E373C5-C386-457b-AF06-2ECBECCD3E12}
DEFINE_GUID(IID_VfwVideoCompressionWrapper,
			0x91e373c5, 0xc386, 0x457b, 0xaf, 0x6, 0x2e, 0xcb, 0xec, 0xcd, 0x3e, 0x12);

interface IVideoForWindowsFilter : public IUnknown
{
	virtual void InitFilterByDescription(const WCHAR* desc) = 0;
	virtual const WCHAR* GetFilterDescription() const = 0;

	virtual bool HasAboutWindow(const WCHAR* desc) const = 0;
	virtual bool HasConfigWindow(const WCHAR* desc) const = 0;
	virtual void ShowAboutWindow(const WCHAR* desc, HWND parentWindow) = 0;
	virtual void ShowConfigWindow(const WCHAR* desc, HWND parentWindow) = 0;
};

class VideoForWindowsFilter : public CTransformFilter, public ISpecifyPropertyPages, public IVideoForWindowsFilter
{
public:
	DECLARE_IUNKNOWN;
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

	VideoForWindowsFilter(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);
	~VideoForWindowsFilter();

	bool	InitFilter(HIC hic);

	// Reveals the interfaces (ISpecifyPropertyPages and IVideoForWindowsFilter)
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// Overridden from CTransformFilter base class
	HRESULT	EndOfStream();
	HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);
	HRESULT CheckInputType(const CMediaType *mtIn);
	HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties);
	HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

	// Implements ISpecifyPropertyPages
	HRESULT STDMETHODCALLTYPE GetPages(CAUUID *pPages);

	// Implements IVideoForWindowsFilter
	void			InitFilterByDescription(const WCHAR* desc);
	const WCHAR*	GetFilterDescription() const;
	bool			HasAboutWindow(const WCHAR* desc) const;
	bool			HasConfigWindow(const WCHAR* desc) const;
	void			ShowAboutWindow(const WCHAR* desc, HWND parentWindow);
	void			ShowConfigWindow(const WCHAR* desc, HWND parentWindow);

private:

	HIC	FindCompressor(const WCHAR* name) const;
	
	HIC		_hic;
	long	_frameNumber;
	long	_maxCompressedFrameSize;
	bool	_hasStarted;
	std::wstring _codecDescription;
};

#endif