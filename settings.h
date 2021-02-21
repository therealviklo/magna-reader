#pragma once
#include <cstdlib>
#include <fstream>
#include <string>

/* En struct med inställningar som kan konverteras till nyare
   versioner. För att göra en ny version krävs bara att man
   skapar en ny Settings<ver> med ett versionsnummer som är
   ett större än det förra och att man lägger till numret i 
   loadSettings(). */

enum struct FitMode
{
	realSizeOrWidth,
	width,
	realSizeOrHeight,
	height,
	realSize
};

template <size_t ver>
struct Settings;

template <size_t fromVer, size_t toVer>
inline Settings<toVer> upgrade(const Settings<fromVer>& us) noexcept
{
	if constexpr (toVer == fromVer)
	{
		return us;
	}
	else if constexpr (toVer == fromVer + 1)
	{
		return Settings<toVer>(us);
	}
	else
	{
		return upgrade<fromVer, toVer - 1>(us);
	}
}

template <>
struct Settings<0>
{
	bool easternReadingOrder;
	FitMode fitMode;

	Settings() noexcept :
		easternReadingOrder(false),
		fitMode(FitMode::width) {}
};

template <size_t ver>
class AutoSaveSettings
{
private:
	static void saveSettings(const Settings<ver>& usettings, const wchar_t* file) noexcept
	{
		try
		{
			constexpr auto cver = ver;
			std::ofstream fs(file, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
			fs.write((const char*)&cver, sizeof(ver));
			fs.write((const char*)&usettings, sizeof(usettings));
		}
		catch (...) {}
	}
	
	template <size_t fromVer>
	static Settings<ver> readUS(std::ifstream& fs)
	{
		Settings<fromVer> us;
		fs.read((char*)&us, sizeof(us));
		return upgrade<fromVer, ver>(us);
	};

	static Settings<ver> loadSettings(const wchar_t* file) noexcept
	{
		try
		{
			std::ifstream fs(file, std::ios_base::in | std::ios_base::binary);
			size_t readVer = 0;
			fs.read((char*)&readVer, sizeof(readVer));
			switch (readVer)
			{
				case 0: return readUS<0>(fs);
			}
		}
		catch (...) {}
		return Settings<ver>();
	}

	Settings<ver> us;
	std::wstring filename;
public:
	AutoSaveSettings(std::wstring filename) noexcept :
		us(loadSettings(filename.c_str())),
		filename(std::move(filename)) {}
	
	constexpr const Settings<ver>* operator->() const noexcept { return &us; }
	template <typename T>
	constexpr void set(T Settings<ver>::* m, const T& v)
	{
		us.*m = v;
		saveSettings(us, filename.c_str());
	}
};