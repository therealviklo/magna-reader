#include "d2d.h"

D2DFactory::D2DFactory()
{
	HRESULT hr;

	if (FAILED(hr = D2D1CreateFactory<ID2D1Factory>(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&factory
	))) throw WinError("Failed to create Direct2D factory", hr);
}

ComPtr<IWICBitmapFrameDecode> WICFactory::loadBitmap(const wchar_t* filename)
{
	HRESULT hr;

	ComPtr<IWICBitmapDecoder> decoder;
	if (FAILED(hr = factory->CreateDecoderFromFilename(
		filename,
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		&decoder
	))) throw WinError("Failed to create WIC image decoder", hr);

	ComPtr<IWICBitmapFrameDecode> bmpDcd;
	if (FAILED(hr = decoder->GetFrame(
		0,
		&bmpDcd
	))) throw WinError("Failed to decode image", hr);

	return bmpDcd;
}

WICFactory::WICFactory()
{
	HRESULT hr;

	if (FAILED(hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		__uuidof(IWICImagingFactory),
		&factory
	))) throw WinError("Failed to create WIC factory", hr);
}

void RenderTarget::createRenderTarget(D2DFactory& d2dfac)
{
	HRESULT hr;

	RECT rc;
	if (!GetClientRect(hWnd, &rc)) throw WinError("Failed to get window client area");

	if (FAILED(hr = d2dfac.factory->CreateHwndRenderTarget(
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

RenderTarget::RenderTarget(HWND hWnd, D2DFactory& d2dfac)
	: hWnd(hWnd)
{
	createRenderTarget(d2dfac);
}

ComPtr<ID2D1Bitmap> RenderTarget::loadBitmap(WICFactory& wicfac, const wchar_t* filename)
{
	HRESULT hr;

	const auto bmpDcd = wicfac.loadBitmap(filename);

	ComPtr<IWICBitmapSource> bmpSrc;
	if (FAILED(hr = bmpSrc->QueryInterface(
		__uuidof(IWICBitmapSource),
		&bmpSrc
	))) throw WinError("Failed to query WIC bitmap source interface", hr);

	ComPtr<ID2D1Bitmap> bmp;
	if (FAILED(rt->CreateBitmapFromWicBitmap(
		bmpSrc.Get(),
		&bmp
	))) throw WinError("Failed to create Direct2D bitmap", hr);

	return bmp;
}