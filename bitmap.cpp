#include "bitmap.h"

Bitmap::Bitmap(const wchar_t* file)
	: bmp([&]{
		const HBITMAP bmp = (HBITMAP)LoadImageW(
			nullptr,
			file,
			IMAGE_BITMAP,
			0,
			0,
			LR_LOADFROMFILE
		);
		if (!bmp) throw NoResWinError("Failed to load image");
		return bmp;
	  }()),
	  dc([&]{
		const HDC dc = CreateCompatibleDC(nullptr);
		if (!dc) throw NoResWinError("Failed to create memory device context");
		return dc;
	  }()),
	  prevBmp{}
{
	BITMAP bmpStruct{};
	if (!GetObjectW(bmp.get(), sizeof(bmpStruct), &bmpStruct))
		throw NoResWinError("Failed to get bitmap object");
	bmpWidth = bmpStruct.bmWidth;
	bmpHeight = bmpStruct.bmHeight;

	prevBmp = SelectObject(dc.get(), bmp.get());
}