#include "d2d.h"

D2DFactory::D2DFactory()
{
	HRESULT hr;

	if (FAILED(hr = D2D1CreateFactory<ID2D1Factory>(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&factory
	))) throw WinError("Failed to create Direct2D factory", hr);
}

RenderTarget::RenderTarget(HWND hWnd, ID2D1Factory* d2dfac)
{
	HRESULT hr;

	RECT rc;
	if (!GetClientRect(hWnd, &rc)) throw WinError("Failed to get window client area");

	if (FAILED(hr = d2dfac->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_SOFTWARE),
		D2D1::HwndRenderTargetProperties(
			hWnd,
			D2D1::SizeU(
				rc.right - rc.left,
				rc.bottom - rc.top)
		),
		&rt
	))) throw WinError("Failed to create hWnd render target", hr);

	rt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	rt->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);
}