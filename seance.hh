#pragma once

#include "actor.hh"

#include <memory>

using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

// Handles an HTTP proxy server connection

class seance : public std::enable_shared_from_this<seance>
{
    // This is the C++11 equivalent of a generic lambda.
    // The function object is used to send an HTTP message.

    class send_action
    {
        seance& self_;
    public:

        explicit send_action(seance& self) : self_(self)
        {
        }

        template<bool isRequest, class Body, class Fields>
        void operator()(http::message<isRequest, Body, Fields>&& msg) const
        {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<http::message<isRequest, Body, Fields> > (std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            self_.res_ = sp;

            // Write the response
            http::async_write
            ( self_.stream_
            , *sp
            , beast::bind_front_handler(&seance::on_write, self_.shared_from_this(), sp->need_eof())
            );
        }
    };

    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    std::shared_ptr<const dict_t> dict_;
    http::request<http::string_body> req_;
    std::shared_ptr<void> res_;
    send_action action_;

public:
    // Take ownership of the stream

    seance(tcp::socket&& socket, std::shared_ptr<const dict_t> const& dict) 
    : stream_(std::move(socket))
    , dict_(dict)
    , action_(*this)
    {
    }

    void run();

    void do_read();

    void on_read(beast::error_code ec, std::size_t bytes_transferred);

    void on_write(bool close, beast::error_code ec, std::size_t bytes_transferred);

    void do_close();
};

