#include "seance.hh"
#include "common.hh"

#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

namespace net = boost::asio; // from <boost/asio.hpp>

// Start the asynchronous operation

void seance::run()
{
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this seance. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    net::dispatch(stream_.get_executor(), beast::bind_front_handler(&seance::do_read, shared_from_this()));
}

void seance::do_read()
{
    // Construct a new parser for each message
    parser_.emplace();
    
    // Apply a reasonable limit to the allowed size
    // of the body in bytes to prevent abuse.
    parser_->body_limit(65536);
    
    // Set the timeout.
    stream_.expires_after(std::chrono::seconds(30));

    // Read a request
    http::async_read(stream_, buffer_, *parser_, beast::bind_front_handler(&seance::on_read, shared_from_this()));
}

void seance::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if (ec == http::error::end_of_stream)
        return do_close();

    if (ec)
        return fail(ec, "read");

    // Send the response
    handle_request(dict_, stream_.get_executor(), parser_->release(), queue_);
    
    // If we aren't at the queue limit, try to pipeline another request
    if(! queue_.is_full())
        do_read();
}

void seance::on_write(bool close, beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return fail(ec, "write");

    if (close)
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return do_close();
    }

    // Inform the queue that a write completed
    if(queue_.on_write())
    {
        // Read another request
        do_read();
    }
}

void seance::do_close()
{
    // Send a TCP shutdown
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
}

seance::queue::queue(seance& self)
: self_(self)
{
    static_assert(limit > 0, "queue limit must be positive");
}

// Returns `true` if we have reached the queue limit
bool seance::queue::is_full() const
{
    return items_.size() >= limit;
}

// Called when a message finishes sending
// Returns `true` if the caller should initiate a read
bool seance::queue::on_write()
{
    BOOST_ASSERT(!items_.empty());
    auto const was_full = is_full();
    items_.pop_front();
    if (!items_.empty())
        (*items_.front())();
    return was_full;
}
