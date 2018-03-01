/*
 * util.h
 * 
 */
#pragma once

#include <string>

std::string string_to_hex(const std::string& in);
std::string hex_to_string(const std::string& in);
std::string base64_encode(std::vector<unsigned char> bytes_to_encode);
std::vector<unsigned char> base64_decode(std::string const& encoded_string);
std::vector<std::string> split(const std::string& s, const std::string& delimiter);
std::tuple<std::string, std::string, std::string> parse_point(const std::string& s);