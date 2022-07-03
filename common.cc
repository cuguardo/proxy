#include "common.hh"

#include <iostream>

// Report a failure
void fail(beast::error_code ec, const char * const what)
{
    std::cerr << what << ": " << ec.message() << std::endl;
}
