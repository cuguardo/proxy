#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
//#include <boost/config.hpp>
//#include <cstdlib>
//#include <functional>
//#include <memory>
//#include <string>
#include <thread>
#include <vector>
#include <forward_list>
#include <iostream>

#include "seance.hh"
#include "ingress.hh"

namespace po = boost::program_options;
namespace fs = boost::filesystem;
//namespace beast = boost::beast;         // from <boost/beast.hpp>
//namespace http = beast::http;           // from <boost/beast/http.hpp>
//namespace net = boost::asio;            // from <boost/asio.hpp>
//using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

int main(int ac, char* av[])
{
    auto context = std::make_shared<context_t>();
    
    auto parse_rules = [context](const std::vector<std::string>& rules)
    {
        boost::regex re("([^@\\s]+)@(\\[[\\w:]+\\]|[^:\\s]+)(:(\\w+))?", boost::regex::extended);
        for(auto rule = rules.begin(); rule != rules.end(); ++rule)
        {
            boost::smatch res;
            if(boost::regex_match(*rule, res, re, boost::match_default)) //do not emplace but replace - a simplest duplication error avoidance
                context->dict[res[1]] = {res[2], res[4].matched ? res[4].str() : std::string("http"), stream_lnk()};
        }
    };
    
    po::options_description conf("General options:");
    conf.add_options()
        ("verbose,V", po::value<short>(&context->swear)->implicit_value(1)->default_value(0), "debug output verbosity level")
        ("address,H", po::value<std::string>(&context->addr)->default_value("127.0.0.1"), "listening address")
        ("port,P", po::value<unsigned short>(&context->port)->default_value(8080), "listening port number")
        ("scale,S", po::value<unsigned short>()->default_value(1), "thread pool size")
        ("queue,Q", po::value<unsigned short>(&context->queue)->default_value(8), "client queue depth")
        ("body,B", po::value<unsigned int>(&context->body)->default_value(65536), "http body limit - in bytes")
        ("timeout,T", po::value<unsigned short>(&context->timeout)->default_value(30), "connection timeout - in seconds")
        ("rule,R", po::value<std::vector<std::string>>()->composing()->multitoken()->notifier(parse_rules), "rule definition - in form <id>@<host>:<port>")
    ;
    
    po::options_description desc("Command line only options:");
    desc.add(conf).add_options()
        ("help,?", "this help message")
        ("config,C", po::value<std::string>(&context->conf)->default_value(fs::path(*av).replace_extension(".ini").filename().string()), "configuration file")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);
    if(fs::exists(context->conf))
    {
        po::store(po::parse_config_file(context->conf.c_str(), desc), vm);
        po::notify(vm);
    }

    if (vm.count("help")) 
    {
        std::clog << desc << std::endl;
        return EXIT_SUCCESS;
    }

    if(context->swear)
    {
        std::clog << std::endl << "Rules:" << std::endl;
        for(auto itr = context->dict.begin(); itr != context->dict.end() ; ++itr)
            std::clog << itr->first << " => " << itr->second.addr << " " << itr->second.port << std::endl;
    }
    
    auto const address = net::ip::make_address(vm["address"].as<std::string>());
    context->scale = std::max<unsigned short>(1, vm["scale"].as<unsigned short>());
    
    // The io_context is required for all I/O
    net::io_context ioc(context->scale);

    // Create and launch a listening port
    std::make_shared<listener>(ioc, tcp::endpoint{address, context->port}, context)->run();

       
    // Run the I/O service on the requested number of threads
    std::forward_list<std::thread> workers;
    for(auto cnt = 1; cnt < context->scale; ++cnt)
        workers.emplace_front( [&ioc] { ioc.run(); } );
    ioc.run();
    
    
    
    return EXIT_SUCCESS;
}
