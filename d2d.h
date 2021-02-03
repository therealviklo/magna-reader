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
	friend class Bitmap;
private:
	ComPtr<IWICImagingFactory> factory;
public:
	WICFactory();
};

class Bitmap;

class RenderTarget
{
	friend class Bitmap;
private:
	HWND hWnd;
	ComPtr<ID2D1HwndRenderTarget> rt;

	void createRenderTarget(D2DFactory& d2dfac);
public:
	RenderTarget(HWND hWnd, D2DFactory& d2dfac);

	constexpr void beginDraw() noexcept { rt->BeginDraw(); }
	// Returnerar false om rendertargeten återskapades
	constexpr bool endDraw(D2DFactory& d2dfac)
	{
		HRESULT hr;
		if (FAILED(hr = rt->EndDraw()))
		{
			if (hr == D2DERR_RECREATE_TARGET)
			{
				createRenderTarget(d2dfac);
				return false;
			}
			else
			{
				throw WinError("Direct2D drawing error", hr);
			}
		}
		return true;
	}

	constexpr void resize(unsigned int w, unsigned int h)
	{
		rt->Resize(D2D1::SizeU(w, h));
	}

	void drawBitmap(const Bitmap& bitmap, float x, float y, float w, float h, float alpha = 1.0f) noexcept;
};

class Bitmap
{
	friend class RenderTarget;
private:
	ComPtr<ID2D1Bitmap> bmp;
	unsigned w;
	unsigned h;
public:
	Bitmap(const wchar_t* filename, WICFactory& wicfac, RenderTarget& rt);

	constexpr unsigned getWidth() const noexcept { return w; }
	constexpr unsigned getHeight() const noexcept { return h; }
};