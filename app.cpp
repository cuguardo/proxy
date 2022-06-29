#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
//#include <boost/config.hpp>
//#include <algorithm>
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
    bool swear;
    std::string cfg;
    unsigned short port;
    auto dict = std::make_shared<dict_t>();
    
    auto parse_rules = [dict](const std::vector<std::string>& rules)
    {
        boost::regex re("([^@\\s]+)@(\\[[\\w:]+\\]|[^:\\s]+)(:(\\w+))?", boost::regex::extended);
        for(auto rule = rules.begin(); rule != rules.end(); ++rule)
        {
            boost::smatch res;
            if(boost::regex_match(*rule, res, re, boost::match_default))
                (*dict)[res[1]] = dict_t::mapped_type(res[2], res[4].matched ? res[4].str() : std::string("http"));
        }
    };
    
    po::options_description conf("General options:");
    conf.add_options()
        ("verbose,V", po::value<bool>(&swear)->implicit_value(true)->default_value(false), "debug output")
        ("address,H", po::value<std::string>()->default_value("127.0.0.1"), "listening address")
        ("port,P", po::value<unsigned short>(&port)->default_value(8080), "listening port number")
        ("threads,T", po::value<unsigned short>()->default_value(1), "thread pool size")
        ("rule,R", po::value<std::vector<std::string>>()->multitoken()->notifier(parse_rules), "rule definition")
    ;
    
    po::options_description desc("Command line only options:");
    desc.add(conf).add_options()
        ("help,?", "this help message")
        ("config,C", po::value<std::string>(&cfg)->default_value(fs::path(*av).replace_extension(".ini").filename().string()), "configuration file")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);
    if(fs::exists(cfg))
    {
        po::store(po::parse_config_file(cfg.c_str(), desc), vm);
        po::notify(vm);
    }

    if (vm.count("help")) 
    {
        std::clog << desc << std::endl;
        return EXIT_SUCCESS;
    }

    if(swear)
    {
        std::clog << std::endl << "Rules:" << std::endl;
        for(auto itr = dict->begin(); itr != dict->end() ; ++itr)
            std::clog << itr->first << " => " << itr->second.first << " " << itr->second.second << std::endl;
    }
    
    //exit(0);
   
    auto const address = net::ip::make_address(vm["address"].as<std::string>());
    auto const threads = std::max<unsigned short>(1, vm["threads"].as<unsigned short>());
    
    // The io_context is required for all I/O
    net::io_context ioc(threads);

    // Create and launch a listening port
    std::make_shared<listener>(ioc, tcp::endpoint{address, port}, dict)->run();

       
    // Run the I/O service on the requested number of threads
    std::forward_list<std::thread> workers;
    for(auto cnt = 1; cnt < threads; ++cnt)
        workers.emplace_front( [&ioc] { ioc.run(); } );
    ioc.run();
    
    
    
    return EXIT_SUCCESS;
}
