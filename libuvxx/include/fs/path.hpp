#pragma once
#include <string>

namespace uvxx { namespace fs 
{
    class path
    {
        path();

    public:
        static const char* DIRECTORY_SEPARATOR;
     
        static const char* ALT_DIRECTORY_SEPARATOR;

        static const char* VOLUME_SEPARATOR_CHAR;

        static bool is_path_rooted(std::string const& path);

        static std::string combine(std::string const& path1, std::string const& path2);

        static int get_root_length(std::string const& path);

        static bool is_directory_separator(char c);
    };
}}