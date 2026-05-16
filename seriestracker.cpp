#include "seriestracker.h"
#include <filesystem>
#include <fstream>
#include "json.h"
#include "utils.h"

JSONValue readJsonFromFile(std::wstring pageTrackerPath)
{
	const auto size = std::filesystem::file_size(pageTrackerPath);
	std::string content(size, '\0');
	{
		std::ifstream in(pageTrackerPath, std::ios::binary);
		in.read(content.data(), size);
	}

	return parseJson(content);
}

size_t getFolderNumber(std::wstring pageTrackerPath, std::wstring path) noexcept
{
	try
	{
		if (!std::filesystem::exists(pageTrackerPath))
		{
			return 0;
		}
	
		const std::string spath = wstringToString(std::filesystem::canonical(path));
	
		const auto obj = std::get<JSONObject>(readJsonFromFile(pageTrackerPath));
		if (!obj.contains(spath))
		{
			return 0;
		}
	
		return std::get<double>(obj.at(spath));
	}
	catch (const std::exception& e)
	{
		try
		{
			std::wostringstream ss;
			ss << L"Unable to load series number (Error: "
				<< e.what()
				<< L")";
			MessageBoxW(nullptr, ss.str().c_str(), L"Nonfatal error", MB_ICONERROR);
		}
		catch (...) {}
		return 0;
	}
}

void saveFolderNumber(std::wstring pageTrackerPath, std::wstring path, size_t folder) noexcept
{
	try
	{
		const std::string spath = wstringToString(std::filesystem::canonical(path));
		
		JSONObject obj;
		if (std::filesystem::exists(pageTrackerPath))
			obj = std::get<JSONObject>(readJsonFromFile(pageTrackerPath));
		obj.insert_or_assign(spath, (JSONValue)(double)folder);
	
		const std::string jsonString = encodeJson(obj);
	
		std::ofstream out(pageTrackerPath, std::ios::binary);
		out.write(jsonString.data(), jsonString.size());
	}
	catch (const std::exception& e)
	{
		try
		{
			std::wostringstream ss;
			ss << L"Unable to save series number (Error: "
				<< e.what()
				<< L")";
			MessageBoxW(nullptr, ss.str().c_str(), L"Nonfatal error", MB_ICONERROR);
		}
		catch (...) {}
	}
}
