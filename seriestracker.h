#pragma once
#include <string>

size_t getFolderNumber(std::wstring pageTrackerPath, std::wstring path) noexcept;
void saveFolderNumber(std::wstring pageTrackerPath, std::wstring path, size_t folder) noexcept;