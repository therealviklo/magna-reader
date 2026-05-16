#include "json.h"
#include <cctype>
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <cstdint>
#include <clocale>
#include <iomanip>
#include <charconv>

namespace
{
	char32_t readUtf8Char(const char*& cur)
	{
		if (static_cast<unsigned char>(*cur) < 0b1000'0000)
			return *cur++;
		char32_t ret = 0;
		int numConts = 0;
		if (static_cast<unsigned char>(*cur) >= 0b1111'1000)
		{
			cur++;
			return U'\uFFFD';
		}
		if (static_cast<unsigned char>(*cur) >= 0b1111'0000)
		{
			ret = *cur++ & 0b0000'0111;
			numConts = 3;
		}
		else if (static_cast<unsigned char>(*cur) >= 0b1110'0000)
		{
			ret = *cur++ & 0b0000'1111;
			numConts = 2;
		}
		else if (static_cast<unsigned char>(*cur) >= 0b110'0000)
		{
			ret = *cur++ & 0b0001'1111;
			numConts = 1;
		}
		else
		{
			cur++;
			return U'\uFFFD';
		}
		while (static_cast<unsigned char>(*cur) >= 0b1000'0000 && static_cast<unsigned char>(*cur) < 0b1100'0000)
		{
			if (numConts-- == 0)
				return U'\uFFFD';
			ret <<= 6;
			ret |= *cur++ & 0b0011'1111;
		}
		if (numConts != 0 || ret >= 0x110000)
			return U'\uFFFD';
		return ret;
	}

	std::string utf32CharToUtf8(char32_t c)
	{
		auto g = [&c](int offset, int num) -> char32_t {
			return (c >> offset) & ~(~U'\0' << num);
		};
		auto gbytes = [&g](int num) -> std::string {
			std::string ret;
			ret += g(6 * (num - 1), 7 - num) | (0b1111'0000 & (0b1111'0000 << (4 - num)));
			for (int i = 0; i < num - 1; i++)
			{
				ret += g(6 * (num - 2 - i), 6) | 0b1000'0000;
			}
			return ret;
		};
		if (c >= 0x110000)
			return "\uFFFD";
		else if (c >= 0x10000)
			return gbytes(4);
		else if (c >= 0x800)
			return gbytes(3);
		else if (c >= 0x80)
			return gbytes(2);
		else
			return std::string(1, static_cast<char>(c));
	}
}

struct LineColCur
{
	const char* cur;
	std::size_t line;
	std::size_t col;

	[[noreturn]] void error(const std::string& msg)
	{
		std::stringstream ss;
		ss << msg << " (row " << line << ", column " << col << ")";
		throw std::runtime_error(ss.str());
	}

	char32_t getUnicodeChar()
	{
		const char32_t c = readUtf8Char(cur);
		if (!c)
			error("Unexpected end of file");
		if (c == U'\n')
		{
			line++;
			col = 1;
		}
		else
		{
			col++;
		}
		return c;
	}

	char32_t peekUnicodeChar() const
	{
		const char* tmpcur = cur;
		return readUtf8Char(tmpcur);
	}
};

class ParseCursor
{
private:
	std::string text;
	LineColCur cur;

	void skipWhitespace()
	{
		auto isJsonWhitespace = [](char32_t c) -> bool {
			return c == U' ' || c == U'\n' || c == U'\r' || c == U'\t';
		};
		while (isJsonWhitespace(cur.peekUnicodeChar()))
			cur.getUnicodeChar();
	}
public:
	ParseCursor(std::string text) :
		text(std::move(text)),
		cur{this->text.c_str(), 1, 1}
	{
		if (!tryParse("\xEF\xBB\xBF"))
			skipWhitespace();
	}

	bool atEnd() const { return *cur.cur == '\0'; }

	bool tryParse(const char* str)
	{
		LineColCur tmpcur = cur;
		while (*str != '\0')
		{
			if (*tmpcur.cur == '\0' || tmpcur.peekUnicodeChar() != readUtf8Char(str))
				return false;
			tmpcur.getUnicodeChar();
		}
		cur = tmpcur;
		skipWhitespace();
		return true;
	}

	std::string parseString(const char* terminator)
	{
		std::string ret;
		while (!tryParse(terminator))
		{
			const char32_t c = cur.getUnicodeChar();
			if (c == '\\')
			{
				const char32_t ec = cur.getUnicodeChar();
				switch (ec)
				{
					case U'"':
					case U'\\':
					case U'/':
					{
						ret += utf32CharToUtf8(ec);
					}
					break;
					case U'b':
					{
						ret += '\b';
					}
					break;
					case U'f':
					{
						ret += '\f';
					}
					break;
					case U'n':
					{
						ret += '\n';
					}
					break;
					case U'r':
					{
						ret += '\r';
					}
					break;
					case U't':
					{
						ret += '\t';
					}
					break;
					case U'u':
					{
						auto readUnicodeEscapeChar = [&]() -> std::int16_t {
							const char32_t c = cur.getUnicodeChar();
							if (U'0' <= c && c <= U'9')
								return c - U'0';
							if (U'a' <= c && c <= U'f')
								return c - U'a' + 10;
							if (U'A' <= c && c <= U'F')
								return c - U'A' + 10;
							error("Invalid hex digit");
						};
						const std::int16_t d1 = readUnicodeEscapeChar();
						const std::int16_t d2 = readUnicodeEscapeChar();
						const std::int16_t d3 = readUnicodeEscapeChar();
						const std::int16_t d4 = readUnicodeEscapeChar();
						const char16_t c1 = (d1 << 12) | (d2 << 8) | (d3 << 4) | d4;
						if (0xD800 <= c1 && c1 <= 0xDBFF)
						{
							if (tryParse("\\u"))
							{
								const std::int16_t d5 = readUnicodeEscapeChar();
								const std::int16_t d6 = readUnicodeEscapeChar();
								const std::int16_t d7 = readUnicodeEscapeChar();
								const std::int16_t d8 = readUnicodeEscapeChar();
								const char16_t c2 = (d5 << 12) | (d6 << 8) | (d7 << 4) | d8;
								if (!(0xDC00 <= c2 && c2 <= 0xDFFF))
								{
									ret += "\uFFFD\uFFFD";
								}
								else
								{
									ret += utf32CharToUtf8((((char32_t(c1) & 0x3FF) << 10) | (char32_t(c2) & 0x3FF)) + 0x10000);
								}
							}
							else
							{
								ret += "\uFFFD";
							}
						}
						else if (0xDC00 <= c1 && c1 <= 0xDFFF)
						{
							ret += "\uFFFD";
						}
						else
						{
							ret += utf32CharToUtf8(c1);
						}
					}
					break;
					default:
					{
						ret += "\uFFFD";
					}
					break;
				}
			}
			else
			{
				ret += utf32CharToUtf8(c);
			}
		}
		return ret;
	}

	double parseNumber()
	{
		const bool negative = tryParse("-");
		// double ret = 0.0;
		std::string num = negative ? "-" : "";
		if (!tryParse("0"))
		{
			while (!atEnd())
			{
				const char32_t c = cur.peekUnicodeChar();
				if (U'0' <= c && c <= U'9')
				{
					// ret *= 10.0;
					// ret += c - U'0';
					num += c;
					cur.getUnicodeChar();
				}
				else break;
			}
		}
		else
		{
			num += '0';
		}
		if (tryParse("."))
		{
			num += '.';
			// double order = 0.1;
			while (!atEnd())
			{
				const char32_t c = cur.peekUnicodeChar();
				if (U'0' <= c && c <= U'9')
				{
					// ret += (c - U'0') * order;
					// order *= 0.1;
					num += c;
					cur.getUnicodeChar();
				}
				else break;
			}
		}
		if (tryParse("e") || tryParse("E"))
		{
			num += 'e';
			const bool negativeExp = !tryParse("+") && tryParse("-");
			if (negativeExp) num += "-";
			// double exponent = 0.0;
			while (!atEnd())
			{
				const char32_t c = cur.peekUnicodeChar();
				if (U'0' <= c && c <= U'9')
				{
					// exponent *= 10.0;
					// exponent += c - U'0';
					num += c;
					cur.getUnicodeChar();
				}
				else break;
			}
			// ret *= std::pow(10, negativeExp ? -exponent : exponent);
		}
		// return negative ? -ret : ret;
		double res = 0;
		const auto fcres = std::from_chars(num.c_str(), num.c_str() + num.length(), res);
		if (fcres.ec == std::errc::invalid_argument)
			error("Failed to parse number");
		if (fcres.ec == std::errc::result_out_of_range)
			error("Number is out of range");
		skipWhitespace();
		return res;
	}

	char32_t peekUnicodeChar() const
	{
		return cur.peekUnicodeChar();
	}

	[[noreturn]] void error(const std::string& msg)
	{
		cur.error(msg);
	}
};

JSONValue parseValue(ParseCursor& pc);

std::string parseString(ParseCursor& pc)
{
	return pc.parseString("\"");
}

JSONObject parseObject(ParseCursor& pc)
{
	JSONObject c;
	bool first = true;
	while (!pc.tryParse("}"))
	{
		if (first)
			first = false;
		else if (!pc.tryParse(","))
			pc.error("Expected ','");
		if (!pc.tryParse("\""))
			pc.error("Expected '\"'");
		std::string name = pc.parseString("\"");
		if (!pc.tryParse(":"))
			pc.error("Expected ':'");
		c[name] = parseValue(pc);
	}
	return c;
}

JSONArray parseArray(ParseCursor& pc)
{
	JSONArray a;
	bool first = true;
	while (!pc.tryParse("]"))
	{
		if (first)
			first = false;
		else if (!pc.tryParse(","))
			pc.error("Expected ','");
		a.push_back(parseValue(pc));
	}
	return a;
}

double parseNumber(ParseCursor& pc)
{
	return pc.parseNumber();
}

JSONValue parseValue(ParseCursor& pc)
{
	if (pc.tryParse("\""))
		return JSONValue(parseString(pc));
	else if (pc.tryParse("{"))
		return JSONValue(parseObject(pc));
	else if (pc.tryParse("["))
		return JSONValue(parseArray(pc));
	else if (pc.tryParse("true"))
		return JSONValue(true);
	else if (pc.tryParse("false"))
		return JSONValue(false);
	else if (pc.tryParse("null"))
		return JSONValue();
	else if (pc.atEnd())
		pc.error("Unexpected end of file");
	else if (const char32_t c = pc.peekUnicodeChar(); c == U'-' || (U'0' <= c && c <= U'9'))
		return JSONValue(parseNumber(pc));
	pc.error("Expected value");
}

JSONValue parseJson(std::string text)
{
	ParseCursor pc(std::move(text));
	return parseValue(pc);
}

void encodeJsonValue(const JSONValue& val, std::stringstream& ss);

void encodeString(const std::string& str, std::stringstream& ss)
{
	ss << '\"';
	const char* cur = str.c_str();
	while (*cur)
	{
		const char32_t c = readUtf8Char(cur);
		if (c == U'"')
			ss << "\\\"";
		else if (c == U'\\')
			ss << "\\\\";
		else if (c == U'\b')
			ss << "\\b";
		else if (c == U'\f')
			ss << "\\f";
		else if (c == U'\n')
			ss << "\\n";
		else if (c == U'\r')
			ss << "\\r";
		else if (c == U'\t')
			ss << "\\t";
		else
			ss << utf32CharToUtf8(c);
	}
	ss << '\"';
}

void encodeNumber(double number, std::stringstream& ss)
{
	if (std::isinf(number) || std::isnan(number))
	{
		ss << "null";
	}
	else
	{
		std::ostringstream numss;
		numss.imbue(std::locale::classic());
		numss << std::setprecision(17) << number;
		ss << numss.str();
	}
}

void encodeObject(const JSONObject& obj, std::stringstream& ss)
{
	ss << '{';
	bool first = true;
	for (const auto& i : obj)
	{
		if (first) first = false;
		else ss << ',';
		encodeString(i.first, ss);
		ss << ':';
		encodeJsonValue(i.second, ss);
	}
	ss << '}';
}

void encodeArray(const JSONArray& arr, std::stringstream& ss)
{
	ss << '[';
	bool first = true;
	for (const auto& i : arr)
	{
		if (first) first = false;
		else ss << ',';
		encodeJsonValue(i, ss);
	}
	ss << ']';
}

void encodeJsonValue(const JSONValue& val, std::stringstream& ss)
{
	if (val.valueless_by_exception())
		throw std::runtime_error("JSON value does not have value");
	switch (JSONType(val.index()))
	{
		case JSONType::null:
		{
			ss << "null";
		}
		break;
		case JSONType::string:
		{
			encodeString(std::get<std::string>(val), ss);
		}
		break;
		case JSONType::number:
		{
			encodeNumber(std::get<double>(val), ss);
		}
		break;
		case JSONType::object:
		{
			encodeObject(std::get<JSONObject>(val), ss);
		}
		break;
		case JSONType::array:
		{
			encodeArray(std::get<JSONArray>(val), ss);
		}
		break;
		case JSONType::boolean:
		{
			ss << (std::get<bool>(val) ? "true" : "false");
		}
		break;
	}
}

std::string encodeJson(const JSONValue& val)
{
	std::stringstream ss;
	encodeJsonValue(val, ss);
	return ss.str();
}