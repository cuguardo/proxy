#pragma once

#include <boost/beast.hpp>

namespace beast = boost::beast;

void fail(beast::error_code, const char * const);

#include <utility>
#include <memory>
#include <string>
#include <tuple>
#include <map>

using stream_ptr = std::shared_ptr<beast::tcp_stream>;
using stream_lnk = std::weak_ptr<beast::tcp_stream>;
struct peer_t
{
    std::string addr;
    std::string port;
    mutable stream_lnk peer;
};
using dict_t = std::map<std::string, peer_t>;

struct context_t
{
    std::string conf;
    std::string addr;
    unsigned short port;
    unsigned short scale;
    unsigned short queue;
    unsigned short timeout;
    unsigned int body;
    short swear;
    dict_t dict;
};
