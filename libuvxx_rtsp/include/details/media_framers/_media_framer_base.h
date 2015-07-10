#pragma once
#include "media_session.hpp"
#include "details/_media_session_impl.hpp"

namespace uvxx { namespace rtsp { namespace details { namespace media_framers
{
    class _media_framer_base
    {
    public:
        _media_framer_base(const media_subsession& subsession) : _subsession(std::move(subsession))
        {
           
        }


        
    private:
        media_subsession _subsession;
    };
}}}}