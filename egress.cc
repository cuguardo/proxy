#include "egress.hh"

void shutdown_graceful::operator()(beast::tcp_stream* ptr) noexcept
{ 
    if(ptr)
    {
        beast::error_code ec;
        ptr->socket().shutdown(tcp::socket::shutdown_both, ec);
        if(buf && ec && ec != beast::errc::not_connected)
            std::ostream(buf) << ec.message() << std::endl;
        delete ptr;
    }
}

