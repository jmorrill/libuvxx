#include "fs/path.hpp"

namespace uvxx { namespace fs 
{
#ifdef _WIN32
    const char*  path::DIRECTORY_SEPARATOR = "\\";
#else
    const char* path::DIRECTORY_SEPARATOR = "/";
#endif

#ifdef _WIN32
    const char* path::ALT_DIRECTORY_SEPARATOR = "\\";
#else
    const char* path::ALT_DIRECTORY_SEPARATOR = "/";
#endif

#if _WIN32
    const char* path::VOLUME_SEPARATOR_CHAR = ":";
#else
    const char* path::VOLUME_SEPARATOR_CHAR = "/";
#endif

    path::path()
    {

    }

    bool path::is_path_rooted(std::string const& path)
    {
        int length = path.length();
        if ((length >= 1 &&
            (path[0] == *DIRECTORY_SEPARATOR ||
             path[0] == *ALT_DIRECTORY_SEPARATOR))
#if _WIN32                      
            || (length >= 2 && path[1] == *VOLUME_SEPARATOR_CHAR)
#endif
            )
        {
            return true;
        }

        return false;
    }

    std::string path::combine(std::string const& path1, std::string const& path2)
    {
        if (path2.length() == 0)
        {
            return path1;
        }

        if (path1.length() == 0)
        {
            return path2;
        }

        if (is_path_rooted(path2))
        {
            return path2;
        }

        char ch = path1[path1.length() - 1];

        if (ch != *DIRECTORY_SEPARATOR && ch != *ALT_DIRECTORY_SEPARATOR && ch != *VOLUME_SEPARATOR_CHAR)
        {
            std::string ret;

            ret.reserve(path1.size() + path2.size() + 2);

            ret.append(path1);

            ret.push_back(*DIRECTORY_SEPARATOR);

            ret.append(path2);

            return ret;
        }

        std::string combined;

        combined.reserve(path1.size() + path2.size() + 2);

        combined.append(path1);

        combined.append(path2);

        return combined;
    }

    int path::get_root_length(std::string const& path)
    {
        int i = 0;
        int length = path.size();
#ifdef _WIN32
        if (length >= 1 && is_directory_separator(path[0]))
        {
            i = 1;
            if (length >= 2 && is_directory_separator(path[1]))
            {
                i = 2;
                int n = 2;

                while (i < length && ((path[i] != *DIRECTORY_SEPARATOR && path[i] != *ALT_DIRECTORY_SEPARATOR) || --n > 0))
                {
                    i++;
                }
            }
        }
        else if (length >= 2 && path[1] == *VOLUME_SEPARATOR_CHAR)
        {
            i = 2;
            if (length >= 3 && (is_directory_separator(path[2])))
            {
                i++;
            }
        }

        return i;
#else
        if(length > = 1 && is_directory_separator(path[0]))
        {
            i = 1;
        }
        return i;
#endif
    }

    bool path::is_directory_separator(char c)
    {
        return (c == *DIRECTORY_SEPARATOR || c == *ALT_DIRECTORY_SEPARATOR);
    }

}}