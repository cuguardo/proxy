#pragma once

#include <boost/beast.hpp>

namespace beast = boost::beast;

void fail(beast::error_code, const char * const);

#include <string>
#include <utility>
#include <unordered_map>

using dict_t = std::unordered_map<std::string, std::pair<std::string, std::string>>;