#include "egress.hh"

#include <boost/filesystem.hpp>

#include <algorithm>
#include <iterator>
#include <iostream>

namespace fs = boost::filesystem;

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

dict_t::const_iterator part_match(std::shared_ptr<context_t> context, const std::string& pattern)
{
    dict_t &dictionary = context->dict;
    
    auto part = 
            [](const fs::path& p, const fs::path& b) -> std::string
            {
                fs::path o;
                //reinventing the std::copy_n(p, std::distance(b.begin(), b.end(), o.begin());
                auto e = p.begin();
                auto d = std::distance(b.begin(), b.end());
                for(ssize_t i = 0; i < d; ++i)
                {
                    if(e == p.end())
                        break;
                    o /= *e++;
                }
                return o.generic_string();
            };
            
    auto left = 
            [part, context](const dict_t::value_type& testee, const dict_t::value_type& tester) -> bool
            {
                std::string subpath = part(tester.first, testee.first);
                int cmp = subpath.compare(testee.first);
                if(context->swear > 1)
                    std::clog << "\tlocate left: " << testee.first << ( cmp ? cmp > 0 ? " < " : " > " : " = " ) << subpath << " [" << tester.first << "] " << std::endl;
                return 0 < cmp;
            };
            
    auto right = 
            [part, context](const dict_t::value_type& tester, const dict_t::value_type& testee) -> bool
            {
                std::string subpath = part(tester.first, testee.first);
                int cmp = subpath.compare(testee.first);
                if(context->swear > 1)
                    std::clog << "\tlocate right: " << testee.first << ( cmp ? cmp > 0 ? " < " : " > " : " = " ) << subpath << " [" << tester.first << "] " << std::endl;
                return 0 < cmp;
            };
            
    const dict_t::value_type sample{ pattern, {} };
    const auto range = std::make_pair(std::lower_bound(dictionary.cbegin(), dictionary.cend(), sample, left), std::upper_bound(dictionary.cbegin(), dictionary.cend(), sample, right));
    auto ret = dictionary.cend();
    for(auto itr = range.first; itr != range.second; ++itr)
    {
        if(itr == dictionary.cend())
            break;
        std::string subpath = part(pattern, itr->first);
        int cmp = subpath.compare(itr->first);
        if(context->swear > 1)
            std::clog << "\tcompare: " << itr->first << ( cmp ? cmp > 0 ? " < " : " > " : " = " ) << subpath << " [" << pattern << "] " << std::endl;
        if(cmp)
        {
            if(ret == dictionary.cend())
                continue;
            break;
        }
        ret = itr;
    }
    return ret;
}
