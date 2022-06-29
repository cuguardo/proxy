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

beast::string_view mime_type(beast::string_view path);

std::string path_cat(beast::string_view base, beast::string_view path);

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.

template<class Body, class Allocator, class Send, class Executor>
void handle_request(std::shared_ptr<const dict_t> const& dict, Executor ex, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send)
{
    // Returns a bad request response
    auto const bad_request =
            [&req](beast::string_view why)
            {
                http::response<http::string_body> res{http::status::bad_request, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = std::string(why);
                res.prepare_payload();
                return res;
            };

    // Returns a not found response
    auto const not_found =
            [&req](beast::string_view target)
            {
                http::response<http::string_body> res{http::status::not_found, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = "The resource '" + std::string(target) + "' was not found.";
                res.prepare_payload();
                return res;
            };

    // Returns a server error response
    auto const server_error =
            [&req](beast::string_view what)
            {
                http::response<http::string_body> res{http::status::internal_server_error, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = "An error occurred: '" + std::string(what) + "'";
                res.prepare_payload();
                return res;
            };
    
    auto partial_match =
            [&req](const dict_t::value_type& sample)
            {   std::clog << "TESTEE=`" << sample.first << "' TESTER=`" << req.target() << "' ";
                bool res{false};
                if(sample.first.size() <= req.target().size())
                    res = !req.target().compare(0, sample.first.size(), sample.first);
                std::clog << (res ? " HIT!" : " MISSED") << std::endl;
                return res;
            };

    // Make sure we can handle the method
    //if (req.method() != http::verb::get &&
    //        req.method() != http::verb::head)
    //    return send(bad_request("Unsupported HTTP-method"));

    
    // Request path must be absolute and not contain "..".
    if (req.target().empty() || req.target().front() != '/')
        return send(bad_request("Illegal request-target"));

    auto item = std::find_if(dict->begin(), dict->end(), partial_match);
    // Handle the case where the target not matched on dictionary
    if (item == dict->end())
        return send(not_found(req.target()));
            
    // Build the path to the target server
    std::string dest(req.target());
    req.target(dest.substr(item->first.size()));
    std::clog << "TARGET=" << req.target() << std::endl; 
        
    // Attempt to open the file
    beast::error_code ec;
    //http::file_body::value_type body;
    //body.open(path.c_str(), beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    //if (ec == beast::errc::no_such_file_or_directory)
        //return send(not_found(req.target()));

    // Prepare the output stream
    tcp::resolver resolver(ex);
    beast::tcp_stream stream(ex);
    
    // Look up the domain name
    auto const results = resolver.resolve(item->second.first, item->second.second);

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);
    
    // Relay the HTTP request to the remote host
    http::write(stream, req);
    
    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;
    
    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);
    
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes
    // so don't bother reporting it.
    //
    if(ec && ec != beast::errc::not_connected)
        return send(server_error(ec.message()));
    /*
    // Handle an unknown error
    if (ec)
        return send(server_error(ec.message()));

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if (req.method() == http::verb::head)
    {
        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        //res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

    // Respond to GET request
    http::response<http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    //res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    */
    return send(std::move(res));
}

