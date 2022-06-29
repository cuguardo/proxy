#include "ingress.hh"
#include "seance.hh"
#include "common.hh"

listener::listener(net::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<const dict_t> const& dict)
: ioc_(ioc)
, acceptor_(net::make_strand(ioc))
, dict_(dict)
{
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec)
    {
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec)
    {
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec)
    {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec)
    {
        fail(ec, "listen");
        return;
    }
}

// Start accepting incoming connections

void
listener::run()
{
    do_accept();
}

void
listener::do_accept()
{
    // The new connection gets its own strand
    acceptor_.async_accept(net::make_strand(ioc_), beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}

void
listener::on_accept(beast::error_code ec, tcp::socket socket)
{
    if (ec)
    {
        fail(ec, "accept");
        return; // To avoid infinite loop
    }
    else
    {
        // Create the seance and run it
        std::make_shared<seance>(std::move(socket), dict_)->run();
    }

    // Accept another connection
    do_accept();
}