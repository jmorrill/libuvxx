#pragma once
#include "DigestAuthentication.hh"

namespace uvxx { namespace rtsp { namespace details 
{
    class _live_authenticator : public Authenticator
    {
    public:
        _live_authenticator() = default;

        _live_authenticator(const _live_authenticator&) = delete;

        _live_authenticator& operator=(const _live_authenticator&) = delete;
    };
}}}