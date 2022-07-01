#pragma once

#include "common.hh"

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>

#include <memory>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

// Accepts incoming connections and launches the seances
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::shared_ptr<dict_t> dict_;
public:
    listener(net::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<dict_t> dict);
    // Start accepting incoming connections
    void run();
private:
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
};
