#pragma once

#include "common.hh"
#include "egress.hh"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>

#include <algorithm>
#include <iostream>
#include <sstream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = boost::filesystem;
using tcp = boost::asio::ip::tcp;
using msg_t = http::response<http::dynamic_body>;

void homologate(std::shared_ptr<context_t> context, const std::string& id, msg_t& res, beast::error_code& ec);

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<class Body, class Allocator, class Send, class Stream>
void handle_request(std::shared_ptr<context_t> context, Stream &cli, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send)
{
    typename Stream::executor_type ex(cli.get_executor());
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
    
    auto const about_req = 
            [&req]()
            {
                for(auto i = req.begin(); i != req.end(); ++i)
                {
                    std::string a(i->name_string()), v(i->value());
                    std::clog << "\t" << a << " = " << v << std::endl;
                }
            };
            
    // Request path must be absolute and not contain "..".
    if (req.target().empty() || req.target().front() != '/')
        return send(bad_request("Illegal request-target"));

    std::string dest(req.target());
    fs::path inbound(dest);
    beast::error_code ec;
    if(context->swear > 0)
        std::clog << "INBOUND PATH=" << dest << std::endl;
    stream_ptr stream = summon_stream(context, ex, dest, ec);
    if(ec)
    {
        switch(ec.value())
        {
        case EADDRNOTAVAIL:
            return send(not_found(dest));
        default:
            return send(server_error(ec.message()));
        }
    }
    
    fs::path outbound = inbound.lexically_proximate(dest).lexically_normal();
    if(!outbound.has_root_directory())
        outbound = fs::path("/") / outbound;
    if(outbound.filename_is_dot())
        outbound.remove_filename();
    // Build the path to the target server
    req.target(outbound.generic_string());
    if(context->swear > 0)
    {
        std::clog << "OUTBOUND PATH=" << req.target() << std::endl;
        if(context->swear > 1)
            about_req();
    }
    std::ostringstream buf("for=");
    buf << cli.socket().remote_endpoint().address().to_string();
    buf << ";host=" << stream->socket().remote_endpoint().address().to_string();
    buf << ";proto=http, for=" << cli.socket().local_endpoint().address().to_string();
    req.set(http::field::forwarded, buf.str());
    
    // Relay the HTTP request to the remote host
    http::write(*stream, req, ec);
    if(ec)
        return send(server_error(ec.message()));
    
    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;
    
    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::read(*stream, buffer, res, ec);
    if(ec)
        return send(server_error(ec.message()));
    
    homologate(context, dest, res, ec);
    if(ec)
        return send(bad_request(ec.message()));
   
    
    return send(std::move(res));
}

