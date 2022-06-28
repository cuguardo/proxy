#pragma once

#include <string>
#include <vector>

using EndPoint = struct Server 
{
    std::string Path;
    std::string Address;
    int Port;
};

using EndPoints = std::vector<EndPoint>;
