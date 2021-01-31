#pragma once
#include "win.h"
#include <d2d1.h>
#include <wrl.h>
#include "winerror.h"

using Microsoft::WRL::ComPtr;

class D2DFactory
{
private:
	ComPtr<ID2D1Factory> factory;
public:
	D2DFactory();

	constexpr operator ID2D1Factory*() const noexcept { return factory.Get(); }
	constexpr ID2D1Factory* operator->() const noexcept { return factory.Get(); }
};

class RenderTarget
{
private:
	ComPtr<ID2D1HwndRenderTarget> rt;
public:
	RenderTarget(HWND hWnd, ID2D1Factory* d2dfac);

	constexpr void beginDraw() noexcept { rt->BeginDraw(); }
	constexpr void endDraw()
	{
		HRESULT hr;
		if (FAILED(hr = rt->EndDraw()))
			throw WinError("Direct2D drawing error", hr);
	}

	constexpr operator ID2D1HwndRenderTarget*() const noexcept { return rt.Get(); }
	constexpr ID2D1HwndRenderTarget* operator->() const noexcept { return rt.Get(); }
};