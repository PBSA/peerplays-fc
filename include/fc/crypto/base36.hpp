#pragma once
#include <vector>

namespace fc
{
    std::vector<char> from_base36( const std::string& b36 );
    std::string to_base36( const std::vector<char>& vec );
    std::string to_base36( const char* data, size_t len );
}
