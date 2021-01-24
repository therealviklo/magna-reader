#pragma once
#include <stdexcept>
#include <utility>
#include <string>
#include <sstream>
#include "win.h"

template <typename T>
std::string toString(T t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

template <typename T>
std::wstring toWString(T t)
{
	std::wstringstream ss;
	ss << t;
	return ss.str();
}

inline std::string wstringToString(const std::wstring& s)
{
	size_t len = WideCharToMultiByte(
		CP_UTF8,
		0,
		s.c_str(),
		s.length(),
		nullptr,
		0,
		nullptr,
		nullptr
	);
	if (!len) throw std::runtime_error("Failed to convert UTF-16 string to UTF-8 string");
	std::string ret(len, '\0');
	if (!WideCharToMultiByte(
		CP_UTF8,
		0,
		s.c_str(),
		s.length(),
		&ret[0],
		ret.length(),
		nullptr,
		nullptr
	)) throw std::runtime_error("Failed to convert UTF-16 string to UTF-8 string");
	return ret;
}

inline std::wstring stringToWstring(const std::string& s)
{
	size_t len = MultiByteToWideChar(
		CP_UTF8,
		0,
		s.c_str(),
		s.length(),
		nullptr,
		0
	);
	if (!len) throw std::runtime_error("Failed to convert UTF-8 string to UTF-16 string");
	std::wstring ret(len, L'\0');
	if (!MultiByteToWideChar(
		CP_UTF8,
		0,
		s.c_str(),
		s.length(),
		&ret[0],
		ret.length()
	)) throw std::runtime_error("Failed to convert UTF-8 string to UTF-16 string");
	return ret;
}