#pragma once
#include "window.h"
#include "winerror.h"

class Bitmap
{
private:
	using UBitmap = UHandle<HBITMAP, DeleteObject>;
	using UDC = UHandle<HDC, DeleteDC>;

	UBitmap bmp;
	UDC dc;
	HGDIOBJ prevBmp;
	long bmpWidth;
	long bmpHeight;
public:
	Bitmap(const wchar_t* file);

	Bitmap(Bitmap&& o) noexcept
		: bmp(std::move(o.bmp)),
		  dc(std::move(o.dc)),
		  prevBmp(o.prevBmp),
		  bmpWidth(o.bmpWidth),
		  bmpHeight(o.bmpHeight) {}

	Bitmap& operator=(Bitmap&& o) noexcept
	{
		if (&o != this)
		{
			if (dc) SelectObject(dc.get(), prevBmp);

			bmpHeight = o.bmpHeight;
			bmpWidth = o.bmpWidth;
			prevBmp = o.prevBmp;
			dc = std::move(o.dc);
			bmp = std::move(o.bmp);
		}
		return *this;
	}
	
	~Bitmap()
	{
		if (dc) SelectObject(dc.get(), prevBmp);
	}

	Bitmap(const Bitmap&) = delete;
	Bitmap& operator=(const Bitmap&) = delete;

	void blit(HDC hDc, int x, int y, int w, int h) const
	{
		if (!StretchBlt(
			hDc,
			x,
			y,
			w,
			h,
			dc.get(),
			0,
			0,
			bmpWidth,
			bmpHeight,
			SRCCOPY
		)) throw NoResWinError("Failed to blit bitmap");
	}
};