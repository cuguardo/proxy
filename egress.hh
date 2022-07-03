#pragma once

#include "common.hh"

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <algorithm>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

class shutdown_graceful
{
    std::streambuf *buf;
public:
    shutdown_graceful(void) : buf(nullptr) {}
    shutdown_graceful(std::ostream& output) : buf(output.rdbuf()) {}
    void operator()(beast::tcp_stream* ptr) noexcept;
};

dict_t::const_iterator part_match(std::shared_ptr<context_t> context, const std::string& pattern);

template<class Executor>
stream_ptr summon_stream(std::shared_ptr<context_t> context, Executor ex, std::string& origin, beast::error_code& ec)
{
    stream_ptr stream;
    do
    {
        auto item = part_match(context, origin);

        // Handle the case where the target not matched on dictionary
        if (item == context->dict.cend())
        {
            ec.assign(EADDRNOTAVAIL, boost::system::generic_category());
            if(context->swear > 0)
                std::cerr << "dict: not found [" << origin << "] " << std::endl;
            break;
        }
        
        origin = item->first;
       
        if(!item->second.peer.expired())
        {
            stream = item->second.peer.lock();
            if(stream && stream->socket().is_open())
                break;
        }
        if(context->swear > 0)
            std::clog << "PEER: " << item->second.addr << ":" << item->second.port << std::endl;

        // Look up the domain name
        tcp::resolver resolver(ex);
        auto const results = resolver.resolve(('[' == item->second.addr.front() && ']' == item->second.addr.back()) ? item->second.addr.substr(1, item->second.addr.size() - 2) : item->second.addr, item->second.port, ec);
        if(ec)
        {
            stream.reset();
            if(context->swear > 0)
                std::cerr << "resolve: " << ec.message() << std::endl;
            break;
        }

        // Prepare the output stream
        stream.reset(new beast::tcp_stream(ex), shutdown_graceful(std::cerr));

        // Make the connection on the IP address we get from a lookup
        stream->connect(results, ec);
        if(ec)
        {
            stream.reset();
            if(context->swear > 0)
                std::cerr << "connect: " << ec.message() << std::endl;
            break;
        }
        
        item->second.peer = stream;
        
    } while(false);
    return stream;
}
