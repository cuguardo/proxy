//#include <boost/config.hpp>
//#include <algorithm>
//#include <cstdlib>
//#include <functional>
//#include <memory>
//#include <string>
#include <thread>
#include <forward_list>
#include <iostream>

#include "seance.hh"
#include "ingress.hh"

//namespace beast = boost::beast;         // from <boost/beast.hpp>
//namespace http = beast::http;           // from <boost/beast/http.hpp>
//namespace net = boost::asio;            // from <boost/asio.hpp>
//using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 5)
    {
        std::cerr <<
            "Usage: http-server-async <address> <port> <doc_root> <threads>\n" <<
            "Example:\n" <<
            "    http-server-async 0.0.0.0 8080 . 1\n";
        return EXIT_FAILURE;
    }
    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const doc_root = std::make_shared<std::string>(argv[3]);
    auto const threads = std::max<int>(1, std::atoi(argv[4]));
    
    // The io_context is required for all I/O
    net::io_context ioc(threads);

    // Create and launch a listening port
    std::make_shared<listener>(ioc, tcp::endpoint{address, port}, doc_root)->run();

       
    // Run the I/O service on the requested number of threads
    std::forward_list<std::thread> workers;
    for(auto cnt = 1; cnt < threads; ++cnt)
        workers.emplace_front( [&ioc] { ioc.run(); } );
    ioc.run();
    
    
    
    return EXIT_SUCCESS;
}
