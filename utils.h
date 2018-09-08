#pragma once
#include <regex>
#include <string>
#include <algorithm>
#include "console.h"

bool is_whitespace(const std::string& str){
	static std::regex emptystr{"^\\s*$"};
	return std::regex_search(str, emptystr);
}

bool is_not_whitespace(const std::string& str){
	return !is_whitespace(str);
}

std::wstring string_to_wstring(const std::string& str){
	std::wstring ret;
	ret.reserve(str.size());
	std::copy(std::begin(str), std::end(str), std::back_inserter(ret));

	return ret;
}


template <class Str=std::string>
bool is_int(const Str& str){
	return std::all_of(std::begin(str), std::end(str), ::isdigit);
}

bool is_int(int i){ return true; }

template <class Str=std::string>
int to_int(const Str& str){
	return std::stoi(str, nullptr, 10);
}