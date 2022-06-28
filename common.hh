#pragma once

#include <boost/beast.hpp>

namespace beast = boost::beast;

void fail(beast::error_code, const char * const);
