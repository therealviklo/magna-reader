#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <d3d11.h>
#include <stdexcept>
#include <wrl.h>
#include <directxmath.h>
#include <wincodec.h>
#include <memory>
#include <optional>
#include <shlwapi.h>

using Microsoft::WRL::ComPtr;

/* Länka med:
   - user32
   - d3d11
   - shlwapi */

struct Colour
{
	float r;
	float g;
	float b;
	float a;
};

class DV2;

class Texture
{
	friend class DV2;
private:
	UINT width;
	UINT height;
	ComPtr<ID3D11Texture2D> tex;
	ComPtr<ID3D11ShaderResourceView> texView;

	Texture(const wchar_t* filename, ID3D11Device* device, IWICImagingFactory* wicFactory);
	Texture(const void* data, UINT size, ID3D11Device* device, IWICImagingFactory* wicFactory);
	
	void createTextureWithDecoder(IWICBitmapDecoder* decoder, ID3D11Device* device, IWICImagingFactory* wicFactory);
public:
	UINT getWidth() const noexcept {return width;}
	UINT getHeight() const noexcept {return height;}
};

class DV2
{
public:
	class Exception : public std::runtime_error
	{
	public:
		Exception(const char* msg) : std::runtime_error(msg) {}
	};
	class NoFullscreenChange : public Exception
	{
	public:
		NoFullscreenChange() : Exception("Unable to change fullscreen state") {}
	};
	class NoSwapChain : public Exception
	{
		public:
		NoSwapChain() : Exception("No swapchain was initialised") {}
	};
private:
	HWND hWnd;

	ComPtr<IDXGISwapChain> swap;
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;

	class SwapChain
	{
	public:
		float width;
		float height;
		
		ComPtr<ID3D11RenderTargetView> target;
		
		ComPtr<ID3D11Buffer> vertexBuffer;
		ComPtr<ID3D11Buffer> colourShiftBuffer;
		ComPtr<ID3D11Buffer> matrixBuffer;
		ComPtr<ID3D11PixelShader> pixelShader;
		ComPtr<ID3D11VertexShader> vertexShader;
		ComPtr<ID3D11InputLayout> inputLayout;
		ComPtr<ID3D11BlendState> blendState;
		ComPtr<ID3D11RasterizerState> rasterizerState;
		ComPtr<ID3D11SamplerState> samplerState;

		SwapChain(
			IDXGISwapChain* swap,
			ID3D11Device* device,
			ID3D11DeviceContext* context,
			HWND hWnd
		);
	};
	std::optional<SwapChain> swapChain;
	
	ComPtr<IWICImagingFactory> wicFactory;
public:
	DV2(HWND hWnd);
	~DV2();

	DV2(const DV2&) = delete;
	DV2& operator=(const DV2&) = delete;

	/* Denna funktion återskapar många saker. Det betyder att om denna
	   funktion kastar en exception så går det nog inte att använda
	   DV2-klassen längre, eftersom vissa saker kanske inte är återskapade. */
	void resize();
	/* Ibland kan det bli så att denna funktion misslyckas men att
	   programmet kan fortsätta sin körning. Om det är så kastar
	   funktionen en exception av typen DV2::NoFullscreenChange istället
	   för en av typen DV2::Exception. (Notera att DV2::NoFullscreenChange
	   är en underklass till DV2::Exception.) */
	void setFullscreen(bool on);

	Texture createTexture(const void* data, UINT size);
	Texture createTexture(const wchar_t* resource, const wchar_t* type);
	Texture createTexture(const wchar_t* filename);

	void clear();
	void clear(Colour clr);

	void draw(
		const Texture& texture,
		float x,
		float y,
		float width,
		float height,
		float srcX,
		float srcY,
		float srcWidth,
		float srcHeight,
		float angle = 0.0f,
		Colour clrShift = Colour{1.0f, 1.0f, 1.0f, 1.0f}
	);
	void draw(
		const Texture& texture,
		float x,
		float y,
		float width,
		float height,
		float angle = 0.0f,
		Colour clrShift = Colour{1.0f, 1.0f, 1.0f, 1.0f}
	) { draw(texture, x, y, width, height, 0.0f, 0.0f, texture.getWidth(), texture.getHeight(), angle, clrShift); }
	void draw(
		const Texture& texture,
		float x,
		float y,
		float srcX,
		float srcY,
		float srcWidth,
		float srcHeight,
		float angle = 0.0f,
		Colour clrShift = Colour{1.0f, 1.0f, 1.0f, 1.0f}
	) { draw(texture, x, y, srcWidth, srcHeight, srcX, srcY, srcWidth, srcHeight, angle, clrShift); }
	void draw(
		const Texture& texture,
		float x,
		float y,
		float angle = 0.0f,
		Colour clrShift = Colour{1.0f, 1.0f, 1.0f, 1.0f}
	) { draw(texture, x, y, texture.getWidth(), texture.getHeight(), 0.0f, 0.0f, texture.getWidth(), texture.getHeight(), angle, clrShift); }

	// Synkar med refreshraten.
	void presentSync();
	// 1 -> synka med refreshraten, 2 -> synka med 1/2 av refreshraten, 3 -> 1/3 etc.
	void presentSync(unsigned int syncInterval);
	// Ingen synkning, visar direkt.
	void presentNoSync();

	float getWidth() const
	{
		if (!swapChain)
			throw NoSwapChain();
		return swapChain->width;
	}
	float getHeight() const
	{
		if (!swapChain)
			throw NoSwapChain();
		return swapChain->height;
	}

	float clientToDVX(float x) const
	{
		if (!swapChain)
			throw NoSwapChain();
		return (x / swapChain->width - 0.5) * getWidth();
	}
	float DVToClientX(float x) const
	{
		if (!swapChain)
			throw NoSwapChain();
		return (x / getWidth() + 0.5) * swapChain->width;
	}
	float clientToDVY(float y) const
	{
		if (!swapChain)
			throw NoSwapChain();
		return -(y / swapChain->height - 0.5) * getHeight();
	}
	float DVToClientY(float y) const
	{
		if (!swapChain)
			throw NoSwapChain();
		return -(y / getHeight() + 0.5) * swapChain->height;
	}

	constexpr bool hasSwapChain() const noexcept
	{
		return swapChain.has_value();
	}

	static void changeScreenResolution(int width, int height);
};