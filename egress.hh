#pragma once

#include <boost/system/detail/generic_category.hpp>


#pragma once

#include "common.hh"

//#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <algorithm>
//#include <memory>
#include <iostream>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>
//namespace net = boost::asio; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class shutdown_graceful
{
    std::streambuf *buf;
public:
    shutdown_graceful(void) : buf(nullptr) {}
    shutdown_graceful(std::ostream& output) : buf(output.rdbuf()) {}
    void operator()(beast::tcp_stream* ptr) noexcept;
};

template<class Executor>
stream_ptr summon_stream(std::shared_ptr<dict_t> dict, Executor ex, std::string& origin, beast::error_code& ec)
{
    stream_ptr stream;
    do
    {
        auto item = part_match(*dict, origin);

        // Handle the case where the target not matched on dictionary
        if (item == dict->cend())
        {
            ec.assign(EADDRNOTAVAIL, boost::system::generic_category());
            std::cerr << "dict: not found" << std::endl;
            break;
        }
        
        origin = item->first;
       
        if(!item->second.peer.expired())
        {
            stream = item->second.peer.lock();
            if(stream && stream->socket().is_open())
                break;
        }

        // Look up the domain name
        tcp::resolver resolver(ex);
        auto const results = resolver.resolve(('[' == item->second.addr.front() && ']' == item->second.addr.back()) ? item->second.addr.substr(1, item->second.addr.size() - 2) : item->second.addr, item->second.port, ec);
        if(ec)
        {
            stream.reset();
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
            std::cerr << "connect: " << ec.message() << std::endl;
            break;
        }
        
        item->second.peer = stream;
        
    } while(false);
    return std::move(stream);
}
