#include "common.hh"

#include <boost/filesystem.hpp>

#include <algorithm>
#include <iterator>
#include <iostream>


namespace fs = boost::filesystem;

// Report a failure

void fail(beast::error_code ec, const char * const what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

dict_t::const_iterator part_match(const dict_t& dictionary, const std::string& pattern)
{
    auto part = [](const fs::path& p, const fs::path& b) -> std::string
    {
        fs::path o;
        //reinventing the std::copy_n(p, std::distance(b.begin(), b.end(), o.begin());
        auto e = p.begin();
        auto d = std::distance(b.begin(), b.end());
        for(ssize_t i = 0; i < d; ++i)
        {
            if(e == p.end())
                break;
            o.append(*e++);
        }
        return o.generic_string();
    };
    auto comparator = [part](const dict_t::value_type& testee, const dict_t::value_type& tester) -> bool
    {
        return 0 < part(tester.first, testee.first).compare(testee.first);
    };
    const dict_t::value_type sample{ pattern, {} };
    const auto range = std::equal_range(dictionary.cbegin(), dictionary.cend(), sample, comparator);
    for(auto itr = range.first; itr != range.second; ++itr)
    {
        if(itr == dictionary.cend() || part(pattern, itr->first).compare(itr->first))
            continue;
        return itr; 
    }
    return dictionary.cend();
}
