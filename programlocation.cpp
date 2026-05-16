#include "programlocation.h"
#include "winerror.h"

std::filesystem::path getProgramLocation()
{
	static std::filesystem::path path = [](){
		std::vector<wchar_t> buf;
		for (size_t size = MAX_PATH; size < 32'767; size *= 2)
		{
			buf.resize(size, L'\0');
			SetLastError(0);
			if (!GetModuleFileNameW(
				nullptr,
				buf.data(),
				buf.size()
			)) throw WinError(L"Unable to get program location");
			// Om inget fel, avsluta loopen.
			// Annars, dubbla storleken på bufferten och försök igen.
			if (!GetLastError())
				break;
		}
		return std::filesystem::path(std::wstring(buf.data()));
	}();
	return path;
}

std::filesystem::path getProgramDirectory()
{
	return getProgramLocation().parent_path();
}