#pragma once
#include "DigestAuthentication.hh"
#include <memory>

namespace uvxx { namespace rtsp { namespace details 
{
    class _live_authenticator : public Authenticator
    {
    public:
        _live_authenticator() : Authenticator()
        {

        }
    };
}}}