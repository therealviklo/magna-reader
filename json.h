#pragma once
#include <string>
#include <map>
#include <vector>
#include <variant>

struct JSONValue;
using JSONObject = std::map<std::string, JSONValue>;
using JSONArray = std::vector<JSONValue>;
using JSONNull = std::monostate;
struct JSONValue : std::variant<JSONNull, std::string, double, JSONObject, JSONArray, bool>
{
	typedef std::variant<JSONNull, std::string, double, JSONObject, JSONArray, bool> JSONVariant;

	JSONValue() noexcept(noexcept(JSONVariant())) :
		JSONVariant() {}
		
	JSONValue(std::string v) noexcept(noexcept(JSONVariant(std::move(v)))) :
		JSONVariant(std::move(v)) {}
		
	JSONValue(double v) noexcept(noexcept(JSONVariant(v))) :
		JSONVariant(v) {}
		
	JSONValue(JSONObject v) noexcept(noexcept(JSONVariant(std::move(v)))) :
		JSONVariant(std::move(v)) {}
		
	JSONValue(JSONArray v) noexcept(noexcept(JSONVariant(std::move(v)))) :
		JSONVariant(std::move(v)) {}
		
	JSONValue(bool v) noexcept(noexcept(JSONVariant(v))) :
		JSONVariant(v) {}
};

enum struct JSONType
{
	null,
	string,
	number,
	object,
	array,
	boolean
};

JSONValue parseJson(std::string text);
std::string encodeJson(const JSONValue& val);