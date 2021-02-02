#pragma once
#include "win.h"
#include <d2d1.h>
#include <wrl.h>
#include <wincodec.h>
#include "winerror.h"

using Microsoft::WRL::ComPtr;

class COMHandler
{
public:
	COMHandler()
	{
		HRESULT hr;
		if (FAILED(hr = CoInitialize(nullptr)))
			throw WinError("Failed to initialise COM", hr);
	}

	~COMHandler()
	{
		CoUninitialize();
	}

	COMHandler(const COMHandler&) = delete;
	COMHandler& operator=(const COMHandler&) = delete;
};

class D2DFactory
{
	friend class RenderTarget;
private:
	ComPtr<ID2D1Factory> factory;
public:
	D2DFactory();
};

class WICFactory
{
	friend class RenderTarget;
private:
	ComPtr<IWICImagingFactory> factory;

	ComPtr<IWICBitmapFrameDecode> loadBitmap(const wchar_t* filename);
public:
	WICFactory();
};

class RenderTarget
{
private:
	HWND hWnd;
	ComPtr<ID2D1HwndRenderTarget> rt;

	void createRenderTarget(D2DFactory& d2dfac);
public:
	RenderTarget(HWND hWnd, D2DFactory& d2dfac);

	constexpr void beginDraw() noexcept { rt->BeginDraw(); }
	constexpr void endDraw(D2DFactory& d2dfac)
	{
		HRESULT hr;
		if (FAILED(hr = rt->EndDraw()))
		{
			if (hr == D2DERR_RECREATE_TARGET)
				createRenderTarget(d2dfac);
			else
				throw WinError("Direct2D drawing error", hr);
		}
	}

	constexpr void resize(unsigned int w, unsigned int h)
	{
		rt->Resize(D2D1::SizeU(w, h));
	}

	ComPtr<ID2D1Bitmap> loadBitmap(WICFactory& wicfac, const wchar_t* filename);
};