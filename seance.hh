#pragma once

#include "actor.hh"

#include <deque>
#include <memory>

using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

// Handles an HTTP proxy server connection

class seance : public std::enable_shared_from_this<seance>
{
    // This queue is used for HTTP pipelining.

    class queue
    {
        // The type-erased, saved work item
        struct work
        {
            virtual ~work() = default;
            virtual void operator()() = 0;
        };

        seance& self_;
        std::deque<std::unique_ptr<work>> items_;

    public:

        explicit queue(seance& self);

        // Returns `true` if we have reached the queue limit
        bool is_full() const;

        // Called when a message finishes sending
        // Returns `true` if the caller should initiate a read
        bool on_write();

        // Called by the HTTP handler to send a response.
        template<bool isRequest, class Body, class Fields>
        void operator()(http::message<isRequest, Body, Fields>&& msg)
        {
            // This holds a work item
            struct work_impl : work
            {
                seance& self_;
                http::message<isRequest, Body, Fields> msg_;

                work_impl(seance& self, http::message<isRequest, Body, Fields>&& msg)
                : self_(self)
                , msg_(std::move(msg))
                {
                }

                void operator()()
                {
                    http::async_write(self_.stream_, msg_, beast::bind_front_handler(&seance::on_write, self_.shared_from_this(), msg_.need_eof()));
                }
            };

            // Allocate and store the work
            items_.push_back(boost::make_unique<work_impl>(self_, std::move(msg)));

            // If there was no previous work, start this one
            if (items_.size() == 1)
                (*items_.front())();
        }
    };

    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    std::shared_ptr<context_t> context_;
    queue queue_;
    // The parser is stored in an optional container so we can
    // construct it from scratch it at the beginning of each new message.
    boost::optional<http::request_parser<http::string_body>> parser_;

public:
    // Take ownership of the stream

    seance(tcp::socket&& socket, std::shared_ptr<context_t> context) 
    : stream_(std::move(socket))
    , context_(context)
    , queue_(*this)
    {
    }

    void run();

    void do_read();

    void on_read(beast::error_code ec, std::size_t bytes_transferred);

    void on_write(bool close, beast::error_code ec, std::size_t bytes_transferred);

    void do_close();
};

