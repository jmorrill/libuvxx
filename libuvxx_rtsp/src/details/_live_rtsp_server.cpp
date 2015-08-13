#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <functional>
#include "Base64.hh"
#include "GroupsockHelper.hh"
#include "RTSPCommon.hh"

#include "pplx/pplxtasks.h"
#include "details/_live_common.hpp"
#include "details/_live_rtsp_server.hpp"

#define RTSP_PARAM_STRING_MAX 200

using namespace uvxx::pplx;
using namespace uvxx::rtsp::details;

enum class StreamingMode 
{
    RTP_UDP,
    RTP_TCP,
    RAW_UDP
};


template <int code>
struct constant_expresion
{
    static int value()
    {
        return code;
    }
};

static void parseTransportHeader(char const*    buf,
                                 size_t       /*buff_len*/,
                                 StreamingMode& streamingMode,
                                 std::string&   streamingModeString,
                                 std::string&   destinationAddressStr,
                                 u_int8_t&      destinationTTL,
                                 portNumBits&   clientRTPPortNum,  /* if UDP */
                                 portNumBits&   clientRTCPPortNum, /* if UDP */
                                 unsigned char& rtpChannelId,      /* if TCP */
                                 unsigned char& rtcpChannelId      /* if TCP */) 
{
    // Initialize the result parameters to default values:
    streamingMode = StreamingMode::RTP_UDP;

    streamingModeString.clear();

    destinationAddressStr.clear();

    destinationTTL = 255;

    clientRTPPortNum = 0;

    clientRTCPPortNum = 1;

    rtpChannelId = rtcpChannelId = 0xFF;

    portNumBits p1, p2;

    unsigned ttl, rtpCid, rtcpCid;

    // First, find "Transport:"
    for (;;)
    {
        if (*buf == '\0') return; // not found

        if (*buf == '\r' && *(buf + 1) == '\n' && *(buf + 2) == '\r') return; // end of the headers => not found

        if (_strncasecmp(buf, "Transport:", 10) == 0) break;

        ++buf;
    }

    // Then, run through each of the fields, looking for ones we handle:
    char const* fields = buf + 10;

    while (*fields == ' ') ++fields;

    std::unique_ptr<char[]> field(strDupSize(fields));

    while (sscanf(fields, "%[^;\r\n]", field.get()) == 1)
    {
        if (strcmp(field.get(), "RTP/AVP/TCP") == 0) 
        {
            streamingMode = StreamingMode::RTP_TCP;
        }
        else if (strcmp(field.get(), "RAW/RAW/UDP") == 0 ||
                 strcmp(field.get(), "MP2T/H2221/UDP") == 0) 
        {
            streamingMode = StreamingMode::RAW_UDP;

            streamingModeString = field.get();
        }
        else if (_strncasecmp(field.get(), "destination=", 12) == 0) 
        {
            destinationAddressStr = (field.get() + 12);
        }
        else if (sscanf(field.get(), "ttl%u", &ttl) == 1)
        {
            destinationTTL = static_cast<u_int8_t>(ttl);
        }
        else if (sscanf(field.get(), "client_port=%hu-%hu", &p1, &p2) == 2)
        {
            clientRTPPortNum = p1;

            clientRTCPPortNum = streamingMode == StreamingMode::RAW_UDP ? 0 : p2; // ignore the second port number if the client asked for raw UDP
        }
        else if (sscanf(field.get(), "client_port=%hu", &p1) == 1)
        {
            clientRTPPortNum = p1;

            clientRTCPPortNum = streamingMode == StreamingMode::RAW_UDP ? 0 : p1 + 1;
        }
        else if (sscanf(field.get(), "interleaved=%u-%u", &rtpCid, &rtcpCid) == 2) 
        {
            rtpChannelId = static_cast<unsigned char>(rtpCid);

            rtcpChannelId = static_cast<unsigned char>(rtcpCid);
        }

        fields += strlen(field.get());

        while (*fields == ';' || *fields == ' ' || *fields == '\t') ++fields; // skip over separating ';' chars or whitespace

        if (*fields == '\0' || *fields == '\r' || *fields == '\n') break;
    }
}

static bool parsePlayNowHeader(char const* buf) {
    // Find "x-playNow:" header, if present
    for (; ;)
    {
        if (*buf == '\0') return false; // not found

        if (_strncasecmp(buf, "x-playNow:", 10) == 0) break;

        ++buf;
    }

    return true;
}

// A special version of "parseTransportHeader()", used just for parsing the "Transport:" header in an incoming "REGISTER" command:
static void parseTransportHeaderForREGISTER(char const* buf,
                                            bool &reuseConnection,
                                            bool& deliverViaTCP,
                                            std::string& proxyURLSuffix) 
{
    // Initialize the result parameters to default values:
    reuseConnection = false;

    deliverViaTCP = false;

    proxyURLSuffix.clear();

    // First, find "Transport:"
    for (;;)
    {
        if (*buf == '\0') return; // not found

        if (*buf == '\r' && *(buf + 1) == '\n' && *(buf + 2) == '\r') return; // end of the headers => not found

        if (_strncasecmp(buf, "Transport:", 10) == 0) break;

        ++buf;
    }

    // Then, run through each of the fields, looking for ones we handle:
    char const* fields = buf + 10;

    while (*fields == ' ') ++fields;

    std::unique_ptr<char[]> field(strDupSize(fields));

    while (sscanf(fields, "%[^;\r\n]", field.get()) == 1) 
    {
        if (strcmp(field.get(), "reuse_connection") == 0) 
        {
            reuseConnection = true;
        }
        else if (_strncasecmp(field.get(), "preferred_delivery_protocol=udp", 31) == 0) 
        {
            deliverViaTCP = false;
        }
        else if (_strncasecmp(field.get(), "preferred_delivery_protocol=interleaved", 39) == 0) 
        {
            deliverViaTCP = true;
        }
        else if (_strncasecmp(field.get(), "proxy_url_suffix=", 17) == 0)
        {
            proxyURLSuffix = (field.get() + 17);
        }

        fields += strlen(field.get());

        while (*fields == ';' || *fields == ' ' || *fields == '\t') ++fields; // skip over separating ';' chars or whitespace
        
        if (*fields == '\0' || *fields == '\r' || *fields == '\n') break;
    }
}

_live_rtsp_server::_live_rtsp_server(uint16_t port) : 
    RTSPServerSupportingHTTPStreaming(*_get_live_environment().get(), setup_socket(port), port, nullptr, 30)
{
}

_live_rtsp_server::~_live_rtsp_server()
{
}

void _live_rtsp_server::set_on_lookup_media_session(_lookup_media_session_delegate callback)
{
    __lookup_media_session_delegate = callback;
}

bool _live_rtsp_server::allow_streaming_rtp_over_tcp()
{
    return fAllowStreamingRTPOverTCP;
}

ServerMediaSession* _live_rtsp_server::lookupServerMediaSession(char const* /*stream_name*/, Boolean /*is_first_lookup_in_session*/)
{
    return nullptr;
}

task<_live_server_media_session*> _live_rtsp_server::begin_lookup_server_media_session(const std::string& stream_name, bool /*is_first_lookup_in_session*/)
{
    if(!__lookup_media_session_delegate)
    {
        return task_from_result<_live_server_media_session*>(nullptr);
    }
    else
    {
        return __lookup_media_session_delegate(stream_name);
    }
}

GenericMediaServer::ClientSession* _live_rtsp_server::createNewClientSession(u_int32_t session_id)
{
    return new _live_rtsp_client_session(*this, session_id);
}

GenericMediaServer::ClientConnection* _live_rtsp_server::createNewClientConnection(int client_socket, sockaddr_in client_addr)
{
    return new _live_rtsp_client_connection(*this, client_socket, client_addr);
}

int _live_rtsp_server::setup_socket(uint16_t port)
{
    Port port_(port);

    auto socket = setUpOurSocket(*_get_live_environment().get(), port_);

    printf("opened server socket on %d\n", socket);

    return socket;
}

_live_rtsp_server::_live_rtsp_client_session::_live_rtsp_client_session(_live_rtsp_server& our_server, u_int32_t session_id) :
    RTSPClientSession(our_server, session_id),
    _our_server(our_server)
{

}

bool _live_rtsp_server::_live_rtsp_client_session::stream_after_setup_get()
{
    return fStreamAfterSETUP;
}

void _live_rtsp_server::_live_rtsp_client_session::note_liveness()
{
    noteLiveness();
}

task<void> _live_rtsp_server::_live_rtsp_client_session::begin_handle_setup(_live_rtsp_client_connection* our_client_connection, const std::string& url_pre_suffix, const std::string& url_suffix, const std::string& full_request_str)
{
    std::string cseq = our_client_connection->current_cseq();

    return  _our_server.begin_lookup_server_media_session(url_pre_suffix.c_str(), true).then([=](task<_live_server_media_session*> t)
    {
        auto sms = t.get();

        return handle_cmd_setup(our_client_connection, sms, cseq.c_str(), url_pre_suffix.c_str(), url_suffix.c_str(), full_request_str.c_str());
    });
}

task<void> _live_rtsp_server::_live_rtsp_client_session::handle_cmd_setup(_live_rtsp_client_connection* client_connection, _live_server_media_session* live_server_media_session, const std::string& cseq, const std::string& url_pre_suffix_string, const std::string& url_suffix_string, const std::string& full_request_string)
{
    auto trackId = std::make_shared<std::string>();
    
    return create_task([=]
    {
        // Normally, "url_pre_suffix" should be the session (stream) name, and "url_suffix" should be the subsession (track) name.
        // However (being "liberal in what we accept"), we also handle 'aggregate' SETUP requests (i.e., without a track name),
        // in the special case where we have only a single track.  I.e., in this case, we also handle:
        // "url_pre_suffix" is empty and "url_suffix" is the session (stream) name, or
        // "url_pre_suffix" concatenated with "url_suffix" (with "/" inbetween) is the session (stream) name.
        std::string streamName;

        *trackId = url_suffix_string; // in the normal case

        std::unique_ptr<char[]> concatenatedStreamName;

        if (live_server_media_session == nullptr)
        {
            // Check for the special case (noted above), before we give up:
            if (url_pre_suffix_string[0] == '\0')
            {
                streamName = url_suffix_string;
            }
            else
            {
                concatenatedStreamName = std::make_unique<char[]>(url_pre_suffix_string.length() + url_suffix_string.length() + 2); // allow for the "/" and the trailing '\0'

                sprintf(concatenatedStreamName.get(), "%s/%s", url_pre_suffix_string.c_str(), url_suffix_string.c_str());

                streamName = concatenatedStreamName.get();
            }

            trackId->clear();

            // Check again:
            return _our_server.begin_lookup_server_media_session(streamName, fOurServerMediaSession == nullptr);
        }

        return task_from_result(live_server_media_session);

    }).then([=](_live_server_media_session* session)
    {
        if (session == nullptr)
        {
            if (fOurServerMediaSession == nullptr)
            {
                // The client asked for a stream that doesn't exist (and this session descriptor has not been used before):
                client_connection->handleCmd_notFound();
            }
            else
            {
                // The client asked for a stream that doesn't exist, but using a stream id for a stream that does exist. Bad request:
                client_connection->handleCmd_bad();
            }

            return;
        }
        else
        {
            if (fOurServerMediaSession == nullptr)
            {
                // We're accessing the "ServerMediaSession" for the first time.
                fOurServerMediaSession = session;

                fOurServerMediaSession->incrementReferenceCount();
            }
            else if (session != fOurServerMediaSession)
            {
                // The client asked for a stream that's different from the one originally requested for this stream id.  Bad request:
                client_connection->handleCmd_bad();

                return;
            }
        }

        if (fStreamStates == nullptr)
        {
            // This is the first "SETUP" for this session.  Set up our array of states for all of this session's subsessions (tracks):
            ServerMediaSubsessionIterator iter(*fOurServerMediaSession);

            for (fNumStreamStates = 0; iter.next() != nullptr; ++fNumStreamStates) {} // begin by counting the number of subsessions (tracks)

            fStreamStates = new struct streamState[fNumStreamStates];

            iter.reset();

            ServerMediaSubsession* subsession;

            for (unsigned i = 0; i < fNumStreamStates; ++i)
            {
                subsession = iter.next();

                fStreamStates[i].subsession = subsession;

                fStreamStates[i].tcpSocketNum = -1; // for now; may get set for RTP-over-TCP streaming

                fStreamStates[i].streamToken = nullptr; // for now; it may be changed by the "getStreamParameters()" call that comes later
            }
        }

        // Look up information for the specified subsession (track):
        ServerMediaSubsession* subsession = nullptr;

        unsigned trackNum;

        if (!trackId->empty() && trackId->at(0) != '\0')
        {   // normal case
            for (trackNum = 0; trackNum < fNumStreamStates; ++trackNum)
            {
                subsession = fStreamStates[trackNum].subsession;

                if (subsession != nullptr && strcmp(trackId->c_str(), subsession->trackId()) == 0) break;
            }
            if (trackNum >= fNumStreamStates)
            {
                // The specified track id doesn't exist, so this request fails:
                client_connection->handleCmd_notFound();

                return;
            }
        }
        else
        {
            // Weird case: there was no track id in the URL.
            // This works only if we have only one subsession:
            if (fNumStreamStates != 1 || fStreamStates[0].subsession == nullptr)
            {
                client_connection->handleCmd_bad();

                return;
            }

            trackNum = 0;

            subsession = fStreamStates[trackNum].subsession;
        }

        void*& token = fStreamStates[trackNum].streamToken; // alias

        if (token != nullptr)
        {
            // We already handled a "SETUP" for this track (to the same client),
            // so stop any existing streaming of it, before we set it up again:
            subsession->pauseStream(fOurSessionId, token);

            _our_server.unnoteTCPStreamingOnSocket(fStreamStates[trackNum].tcpSocketNum, this, trackNum);

            subsession->deleteStream(fOurSessionId, token);
        }

        // Look for a "Transport:" header in the request string, to extract client parameters:
        StreamingMode streamingMode;

        std::string streamingModeString; // set when RAW_UDP streaming is specified

        std::string clientsDestinationAddressStr;

        u_int8_t clientsDestinationTTL;

        portNumBits clientRTPPortNum, clientRTCPPortNum;

        unsigned char rtpChannelId, rtcpChannelId;

        parseTransportHeader(full_request_string.c_str(),
                             full_request_string.size(),
                             streamingMode,
                             streamingModeString,
                             clientsDestinationAddressStr,
                             clientsDestinationTTL,
                             clientRTPPortNum,
                             clientRTCPPortNum,
                             rtpChannelId,
                             rtcpChannelId);

        if ((streamingMode == StreamingMode::RTP_TCP && rtpChannelId == 0xFF) ||
            (streamingMode != StreamingMode::RTP_TCP && client_connection->client_output_socket() != client_connection->client_input_socket()))
        {
            // An anomolous situation, caused by a buggy client.  Either:
            //     1/ TCP streaming was requested, but with no "interleaving=" fields.  (QuickTime Player sometimes does this.), or
            //     2/ TCP streaming was not requested, but we're doing RTSP-over-HTTP tunneling (which implies TCP streaming).
            // In either case, we assume TCP streaming, and set the RTP and RTCP channel ids to proper values:
            streamingMode = StreamingMode::RTP_TCP;

            rtpChannelId = fTCPStreamIdCount;

            rtcpChannelId = fTCPStreamIdCount + 1;
        }

        if (streamingMode == StreamingMode::RTP_TCP)
        {
            fTCPStreamIdCount += 2;
        }

        Port clientRTPPort(clientRTPPortNum);

        Port clientRTCPPort(clientRTCPPortNum);

        // Next, check whether a "Range:" or "x-playNow:" header is present in the request.
        // This isn't legal, but some clients do this to combine "SETUP" and "PLAY":
        double rangeStart = 0.0, rangeEnd = 0.0;

        char* absStart_ = nullptr; char* absEnd_ = nullptr;

        Boolean startTimeIsNow;

        std::unique_ptr<char[]> t;

        if (parseRangeHeader(full_request_string.c_str(), rangeStart, rangeEnd, absStart_, absEnd_, startTimeIsNow))
        {
            delete[] absStart_;

            delete[] absEnd_;

            fStreamAfterSETUP = true;
        }
        else if (parsePlayNowHeader(full_request_string.c_str()))
        {
            fStreamAfterSETUP = true;
        }
        else
        {
            fStreamAfterSETUP = false;
        }

        // Then, get server parameters from the 'subsession':
        if (streamingMode == StreamingMode::RTP_TCP)
        {
            // Note that we'll be streaming over the RTSP TCP connection:
            fStreamStates[trackNum].tcpSocketNum = client_connection->client_output_socket();

            _our_server.noteTCPStreamingOnSocket(fStreamStates[trackNum].tcpSocketNum, this, trackNum);
        }

        netAddressBits destinationAddress = 0;

        u_int8_t destinationTTL = 255;

#ifdef RTSP_ALLOW_CLIENT_DESTINATION_SETTING
        if (!clientsDestinationAddressStr.empty())
        {
            // Use the client-provided "destination" address.
            // Note: This potentially allows the server to be used in denial-of-service
            // attacks, so don't enable this code unless you're sure that clients are
            // trusted.
            destinationAddress = our_inet_addr(clientsDestinationAddressStr.c_str());
        }
        // Also use the client-provided TTL.
        destinationTTL = clientsDestinationTTL;
#endif

        Port serverRTPPort(0);

        Port serverRTCPPort(0);

        // Make sure that we transmit on the same interface that's used by the client (in case we're a multi-homed server):
        struct sockaddr_in sourceAddr; SOCKLEN_T namelen = sizeof sourceAddr;

        getsockname(client_connection->client_input_socket(), reinterpret_cast<struct sockaddr*>(&sourceAddr), reinterpret_cast<socklen_t*>(&namelen));

        netAddressBits origSendingInterfaceAddr = SendingInterfaceAddr;

        netAddressBits origReceivingInterfaceAddr = ReceivingInterfaceAddr;

        // NOTE: The following might not work properly, so we ifdef it out for now:
#ifdef HACK_FOR_MULTIHOMED_SERVERS
        ReceivingInterfaceAddr = SendingInterfaceAddr = sourceAddr.sin_addr.s_addr;
#endif

        subsession->getStreamParameters(fOurSessionId,
                                        client_connection->client_addr().sin_addr.s_addr,
                                        clientRTPPort,
                                        clientRTCPPort,
                                        fStreamStates[trackNum].tcpSocketNum,
                                        rtpChannelId,
                                        rtcpChannelId,
                                        destinationAddress,
                                        destinationTTL,
                                        fIsMulticast,
                                        serverRTPPort,
                                        serverRTCPPort,
                                        fStreamStates[trackNum].streamToken);

        SendingInterfaceAddr = origSendingInterfaceAddr;

        ReceivingInterfaceAddr = origReceivingInterfaceAddr;

        AddressString destAddrStr(destinationAddress);

        AddressString sourceAddrStr(sourceAddr);

        char timeoutParameterString[100];

        if (_our_server.fReclamationSeconds > 0)
        {
            sprintf(timeoutParameterString, ";timeout=%u", _our_server.fReclamationSeconds);
        }
        else
        {
            timeoutParameterString[0] = '\0';
        }

        if (fIsMulticast)
        {
            switch (streamingMode)
            {
                case StreamingMode::RTP_UDP:
                {
                    snprintf(reinterpret_cast<char*>(client_connection->response_buffer()),
                             client_connection->response_buffer_size(),
                             "RTSP/1.0 200 OK\r\n"
                             "CSeq: %s\r\n"
                             "%s"
                             "Transport: RTP/AVP;multicast;destination=%s;source=%s;port=%d-%d;ttl=%d\r\n"
                             "Session: %08X%s\r\n\r\n",
                             cseq.c_str(),
                             dateHeader(),
                             destAddrStr.val(),
                             sourceAddrStr.val(),
                             ntohs(serverRTPPort.num()),
                             ntohs(serverRTCPPort.num()),
                             destinationTTL,
                             fOurSessionId,
                             timeoutParameterString);

                    break;
                }

                case StreamingMode::RTP_TCP:
                {
                    // multicast streams can't be sent via TCP
                    client_connection->handleCmd_unsupportedTransport();

                    break;
                }

                case StreamingMode::RAW_UDP:
                {
                    snprintf(reinterpret_cast<char*>(client_connection->response_buffer()),
                             client_connection->response_buffer_size(),
                             "RTSP/1.0 200 OK\r\n"
                             "CSeq: %s\r\n"
                             "%s"
                             "Transport: %s;multicast;destination=%s;source=%s;port=%d;ttl=%d\r\n"
                             "Session: %08X%s\r\n\r\n",
                             cseq.c_str(),
                             dateHeader(),
                             streamingModeString.c_str(),
                             destAddrStr.val(),
                             sourceAddrStr.val(),
                             ntohs(serverRTPPort.num()),
                             destinationTTL,
                             fOurSessionId,
                             timeoutParameterString);

                    break;
                }
            }
        }
        else
        {
            switch (streamingMode)
            {
                case StreamingMode::RTP_UDP:
                {
                    snprintf(reinterpret_cast<char*>(client_connection->response_buffer()),
                            client_connection->response_buffer_size(),
                            "RTSP/1.0 200 OK\r\n"
                            "CSeq: %s\r\n"
                            "%s"
                            "Transport: RTP/AVP;unicast;destination=%s;source=%s;client_port=%d-%d;server_port=%d-%d\r\n"
                            "Session: %08X%s\r\n\r\n",
                            cseq.c_str(),
                            dateHeader(),
                            destAddrStr.val(),
                            sourceAddrStr.val(),
                            ntohs(clientRTPPort.num()),
                            ntohs(clientRTCPPort.num()),
                            ntohs(serverRTPPort.num()),
                            ntohs(serverRTCPPort.num()),
                            fOurSessionId,
                            timeoutParameterString);

                    break;
                }
                case StreamingMode::RTP_TCP:
                {
                    if (!_our_server.allow_streaming_rtp_over_tcp())
                    {
                        client_connection->handleCmd_unsupportedTransport();
                    }
                    else
                    {
                        snprintf(reinterpret_cast<char*>(client_connection->response_buffer()),
                                 client_connection->response_buffer_size(),
                                 "RTSP/1.0 200 OK\r\n"
                                 "CSeq: %s\r\n"
                                 "%s"
                                 "Transport: RTP/AVP/TCP;unicast;destination=%s;source=%s;interleaved=%d-%d\r\n"
                                 "Session: %08X%s\r\n\r\n",
                                 cseq.c_str(),
                                 dateHeader(),
                                 destAddrStr.val(),
                                 sourceAddrStr.val(),
                                 rtpChannelId,
                                 rtcpChannelId,
                                 fOurSessionId,
                                 timeoutParameterString);
                    }

                    break;
                }
                case StreamingMode::RAW_UDP:
                {
                    snprintf(reinterpret_cast<char*>(client_connection->response_buffer()),
                             client_connection->response_buffer_size(),
                             "RTSP/1.0 200 OK\r\n"
                             "CSeq: %s\r\n"
                             "%s"
                             "Transport: %s;unicast;destination=%s;source=%s;client_port=%d;server_port=%d\r\n"
                             "Session: %08X%s\r\n\r\n",
                             cseq.c_str(),
                             dateHeader(),
                             streamingModeString.c_str(),
                             destAddrStr.val(),
                             sourceAddrStr.val(),
                             ntohs(clientRTPPort.num()),
                             ntohs(serverRTPPort.num()),
                             fOurSessionId,
                             timeoutParameterString);

                    break;
                }
            }
        }
    });
}

void _live_rtsp_server::_live_rtsp_client_session::handle_cmd_within_session(RTSPClientConnection* ourClientConnection, char const* cmdName, char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr)
{
    RTSPClientSession::handleCmd_withinSession(ourClientConnection, cmdName, urlPreSuffix, urlSuffix, fullRequestStr);
}

_live_rtsp_server::_live_rtsp_client_connection::_live_rtsp_client_connection(_live_rtsp_server& our_server, int client_socket, sockaddr_in client_addr):
    RTSPClientConnectionSupportingHTTPStreaming(our_server, client_socket, client_addr),
    __live_rtsp_server(our_server)
{
                
}

_live_rtsp_server::_live_rtsp_client_connection::~_live_rtsp_client_connection()
{
}

void _live_rtsp_server::_live_rtsp_client_connection::handleCmd_unsupportedTransport()
{
    RTSPClientConnection::handleCmd_unsupportedTransport();
}

task<void> _live_rtsp_server::_live_rtsp_client_connection::begin_handle_describe(const std::string& url_pre_suffix, const std::string& url_suffix, const std::string& full_request_str)
{
    std::string url_total_suffix;

    url_total_suffix.reserve(RTSP_PARAM_STRING_MAX);

    if (url_pre_suffix[0] != '\0') 
    {
        url_total_suffix += url_pre_suffix;

        url_total_suffix += "/";
    }

    url_total_suffix += url_suffix;

    if (!authenticationOK("DESCRIBE", url_total_suffix.c_str(), full_request_str.c_str()))
    {
        return task_from_result();
    }
  
    std::string cseq = fCurrentCSeq;
    
    struct pointer_holder
    {
        pointer_holder() : session(nullptr)
        {
        }

        _live_server_media_session* session = nullptr;
    };

    auto session_holder = std::make_shared<pointer_holder>();

    return __live_rtsp_server.begin_lookup_server_media_session(url_total_suffix, true).then([=](_live_server_media_session* session) mutable
    {
        if (session == nullptr)
        {
            handleCmd_notFound();

            return task_from_result();
        }

        session_holder->session = session;

        return session->preload_sdp_data();

    }).then([=](task<void> loaded_task)
    {
        try
        {
            loaded_task.get();
        }catch(std::exception& e)
        {
            printf(e.what());
        }
        
        auto session = session_holder->session;

        if(session == nullptr)
        {
            return;
        }

        // Increment the "ServerMediaSession" object's reference count, in case someone removes it
        // while we're using it:
        session->incrementReferenceCount();

        // Then, assemble a SDP description for this session:
        std::unique_ptr<char[]> sdpDescription(session->generateSDPDescription());

        if (sdpDescription == nullptr)
        {
            // This usually means that a file name that was specified for a
            // "ServerMediaSubsession" does not exist.
            setRTSPResponse("404 File Not Found, Or In Incorrect Format");

            return;
        }

        unsigned sdpDescriptionSize = strlen(sdpDescription.get());

        // Also, generate our RTSP URL, for the "Content-Base:" header
        // (which is necessary to ensure that the correct URL gets used in subsequent "SETUP" requests).
        std::unique_ptr<char[]> rtspURL(fOurRTSPServer.rtspURL(session, fClientInputSocket));

        snprintf(reinterpret_cast<char*>(fResponseBuffer), 
                 sizeof fResponseBuffer,
                 "RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
                 "%s"
                 "Content-Base: %s/\r\n"
                 "Content-Type: application/sdp\r\n"
                 "Content-Length: %d\r\n\r\n"
                 "%s",
                 cseq.c_str(),
                 dateHeader(),
                 rtspURL.get(),
                 sdpDescriptionSize,
                 sdpDescription.get());

        if(session == nullptr)
        {
            return;
        }

        // Decrement its reference count, now that we're done using it:
        session->decrementReferenceCount();

        if (session->referenceCount() == 0 &&
            session->deleteWhenUnreferenced())
        {
            fOurServer.removeServerMediaSession(session);
        }
    });
}

int _live_rtsp_server::_live_rtsp_client_connection::client_output_socket()
{
    return fClientOutputSocket;
}

int _live_rtsp_server::_live_rtsp_client_connection::client_input_socket()
{
    return fClientInputSocket;
}

sockaddr_in _live_rtsp_server::_live_rtsp_client_connection::client_addr()
{
    return fClientAddr;
}

const char* _live_rtsp_server::_live_rtsp_client_connection::current_cseq()
{
    return fCurrentCSeq;
}

unsigned char* _live_rtsp_server::_live_rtsp_client_connection::response_buffer()
{
    return fResponseBuffer;
}

size_t _live_rtsp_server::_live_rtsp_client_connection::response_buffer_size()
{
    return sizeof fRequestBuffer;
}

void _live_rtsp_server::_live_rtsp_client_connection::handleCmd_notFound()
{
    RTSPClientConnection::handleCmd_notFound();
}

void _live_rtsp_server::_live_rtsp_client_connection::handleCmd_bad()
{
    RTSPClientConnection::handleCmd_bad();
}

void _live_rtsp_server::_live_rtsp_client_connection::handleRequestBytes(int newBytesRead)
{
    int numBytesRemaining = 0;

    ++fRecursionCount;

    do 
    {
        _live_rtsp_client_session* clientSession = nullptr;

        if (newBytesRead < 0 || static_cast<unsigned>(newBytesRead) >= fRequestBufferBytesLeft)
        {
            // Either the client socket has died, or the request was too big for us.
            // Terminate this connection:
#ifdef DEBUG
            fprintf(stderr, "RTSPClientConnection[%p]::handleRequestBytes(int) read %d new bytes (of %d); terminating connection!\n", this, newBytesRead, fRequestBufferBytesLeft);
#endif
            fIsActive = false;
            break;
        }

        bool endOfMsg = false;

        unsigned char* ptr = &fRequestBuffer[fRequestBytesAlreadySeen];

#ifdef DEBUG
        ptr[newBytesRead] = '\0';

        fprintf(stderr, 
                "RTSPClientConnection[%p]::handleRequestBytes(  ) %s %d new bytes:%s\n",
                this, 
                numBytesRemaining > 0 ? "processing" : "read", 
                newBytesRead, 
                ptr);
#endif

        if (fClientOutputSocket != fClientInputSocket && numBytesRemaining == 0)
        {
            // We're doing RTSP-over-HTTP tunneling, and input commands are assumed to have been Base64-encoded.
            // We therefore Base64-decode as much of this new data as we can (i.e., up to a multiple of 4 bytes).

            // But first, we remove any whitespace that may be in the input data:
            unsigned toIndex = 0;

            for (int fromIndex = 0; fromIndex < newBytesRead; ++fromIndex) 
            {
                char c = ptr[fromIndex];

                if (!(c == ' ' || c == '\t' || c == '\r' || c == '\n')) 
                {   // not 'whitespace': space,tab,CR,NL
                    ptr[toIndex++] = c;
                }
            }

            newBytesRead = toIndex;

            unsigned numBytesToDecode = fBase64RemainderCount + newBytesRead;

            unsigned newBase64RemainderCount = numBytesToDecode % 4;

            numBytesToDecode -= newBase64RemainderCount;

            if (numBytesToDecode > 0) 
            {
                ptr[newBytesRead] = '\0';

                unsigned decodedSize;

                std::unique_ptr<uint8_t[]> decodedBytes(base64Decode(reinterpret_cast<char const*>(ptr - fBase64RemainderCount), numBytesToDecode, decodedSize));
#ifdef DEBUG
                fprintf(stderr, "Base64-decoded %d input bytes into %d new bytes:", numBytesToDecode, decodedSize);

                for (unsigned k = 0; k < decodedSize; ++k) fprintf(stderr, "%c", decodedBytes[k]);

                fprintf(stderr, "\n");
#endif

                // Copy the new decoded bytes in place of the old ones (we can do this because there are fewer decoded bytes than original):
                unsigned char* to = ptr - fBase64RemainderCount;

                for (unsigned i = 0; i < decodedSize; ++i) *to++ = decodedBytes[i];

                // Then copy any remaining (undecoded) bytes to the end:
                for (unsigned j = 0; j < newBase64RemainderCount; ++j) *to++ = (ptr - fBase64RemainderCount + numBytesToDecode)[j];

                newBytesRead = decodedSize - fBase64RemainderCount + newBase64RemainderCount;
                // adjust to allow for the size of the new decoded data (+ remainder)
            }

            fBase64RemainderCount = newBase64RemainderCount;
        }

        unsigned char* tmpPtr = fLastCRLF + 2;

        if (fBase64RemainderCount == 0) 
        { 
            // no more Base-64 bytes remain to be read/decoded
            // Look for the end of the message: <CR><LF><CR><LF>
            if (tmpPtr < fRequestBuffer) tmpPtr = fRequestBuffer;

            while (tmpPtr < &ptr[newBytesRead - 1]) 
            {
                if (*tmpPtr == '\r' && *(tmpPtr + 1) == '\n')
                {
                    if (tmpPtr - fLastCRLF == 2)
                    {   // This is it:
                        endOfMsg = True;
                        break;
                    }

                    fLastCRLF = tmpPtr;
                }

                ++tmpPtr;
            }
        }

        fRequestBufferBytesLeft -= newBytesRead;

        fRequestBytesAlreadySeen += newBytesRead;

        if (!endOfMsg) break; // subsequent reads will be needed to complete the request

                              // Parse the request string into command name and 'CSeq', then handle the command:

        fRequestBuffer[fRequestBytesAlreadySeen] = '\0';

        char cmdName[RTSP_PARAM_STRING_MAX];

        char urlPreSuffix[RTSP_PARAM_STRING_MAX];

        char urlSuffix[RTSP_PARAM_STRING_MAX];

        char cseq[RTSP_PARAM_STRING_MAX];

        char sessionIdStr[RTSP_PARAM_STRING_MAX];

        unsigned contentLength = 0;

        fLastCRLF[2] = '\0'; // temporarily, for parsing

        auto request_buff = reinterpret_cast<char*>(fRequestBuffer);

        bool parseSucceeded = parseRTSPRequestString(request_buff, fLastCRLF + 2 - fRequestBuffer,
                                                     cmdName,      sizeof cmdName,
                                                     urlPreSuffix, sizeof urlPreSuffix,
                                                     urlSuffix,    sizeof urlSuffix,
                                                     cseq,         sizeof cseq,
                                                     sessionIdStr, sizeof sessionIdStr,
                                                     contentLength);
        fLastCRLF[2] = '\r'; // restore its value

        bool playAfterSetup = false;

        if (parseSucceeded)
        {
#ifdef DEBUG
            fprintf(stderr, "parseRTSPRequestString() succeeded, returning cmdName \"%s\", urlPreSuffix \"%s\", urlSuffix \"%s\", CSeq \"%s\", Content-Length %u, with %d bytes following the message.\n", cmdName, urlPreSuffix, urlSuffix, cseq, contentLength, ptr + newBytesRead - (tmpPtr + 2));
#endif
            // If there was a "Content-Length:" header, then make sure we've received all of the data that it specified:
            if (ptr + newBytesRead < tmpPtr + 2 + contentLength) break; // we still need more data; subsequent reads will give it to us 

            // If the request included a "Session:" id, and it refers to a client session that's
            // current ongoing, then use this command to indicate 'liveness' on that client session:
            bool const requestIncludedSessionId = sessionIdStr[0] != '\0';

            if (requestIncludedSessionId) 
            {
                clientSession = static_cast<_live_rtsp_client_session*>(__live_rtsp_server.fClientSessions->Lookup(sessionIdStr));
               
                if (clientSession != nullptr)
                {
                    clientSession->note_liveness();
                }
            }

            // We now have a complete RTSP request.
            // Handle the specified command (beginning with commands that are session-independent):
            fCurrentCSeq = cseq;

            if (strcmp(cmdName, "OPTIONS") == 0) 
            {
                // If the "OPTIONS" command included a "Session:" id for a session that doesn't exist,
                // then treat this as an error:
                if (requestIncludedSessionId && clientSession == nullptr) 
                {
                    handleCmd_sessionNotFound();
                }
                else 
                {
                    // Normal case:
                    handleCmd_OPTIONS();
                }
            }
            else if (urlPreSuffix[0] == '\0' && urlSuffix[0] == '*' && urlSuffix[1] == '\0') 
            {
                // The special "*" URL means: an operation on the entire server.  This works only for GET_PARAMETER and SET_PARAMETER:
                if (strcmp(cmdName, "GET_PARAMETER") == 0) 
                {
                    handleCmd_GET_PARAMETER(reinterpret_cast<char const*>(fRequestBuffer));
                }
                else if (strcmp(cmdName, "SET_PARAMETER") == 0)
                {
                    handleCmd_SET_PARAMETER(reinterpret_cast<char const*>(fRequestBuffer));
                }
                else 
                {
                    handleCmd_notSupported();
                }
            }
            else if (strcmp(cmdName, "DESCRIBE") == 0) 
            {
                envir().taskScheduler().disableBackgroundHandling(fOurSocket);

                begin_handle_describe(urlPreSuffix, urlSuffix, reinterpret_cast<char const*>(fRequestBuffer)).
                then([=]() mutable
                {
                    send(fClientOutputSocket, 
                         reinterpret_cast<char const*>(fResponseBuffer), 
                         strlen(reinterpret_cast<char*>(fResponseBuffer)), 
                         0);

                    // Check whether there are extra bytes remaining in the buffer, after the end of the request (a rare case).
                    // If so, move them to the front of our buffer, and keep processing it, because it might be a following, pipelined request.
                    unsigned requestSize = (fLastCRLF + 4 - fRequestBuffer) + contentLength;

                    numBytesRemaining = fRequestBytesAlreadySeen - requestSize;

                    resetRequestBuffer(); // to prepare for any subsequent request

                    if (numBytesRemaining > 0) 
                    {
                        memmove(fRequestBuffer, &fRequestBuffer[requestSize], numBytesRemaining);

                        newBytesRead = numBytesRemaining;
                    }

                    if(numBytesRemaining > 0)
                    {
                        handleRequestBytes(numBytesRemaining);
                    }

                    --fRecursionCount;

                    if (!fIsActive) 
                    {
                        if (fRecursionCount > 0)
                        {
                            closeSockets();
                        }
                        else
                        {
                            delete this;
                        }
                    }
                    else
                    {
                        envir().taskScheduler().setBackgroundHandling(fOurSocket, 
                                                                      SOCKET_READABLE | SOCKET_EXCEPTION, 
                                                                      incomingRequestHandler, this);
                    }
                });

                return;
            }
            else if (strcmp(cmdName, "SETUP") == 0) 
            {
                bool areAuthenticated = true;

                if (!requestIncludedSessionId)
                {
                    // No session id was present in the request.  So create a new "RTSPClientSession" object
                    // for this request.  Choose a random (unused) 32-bit integer for the session id
                    // (it will be encoded as a 8-digit hex number).  (We avoid choosing session id 0,
                    // because that has a special use (by "OnDemandServerMediaSubsession").)

                    // But first, make sure that we're authenticated to perform this command:
                    char urlTotalSuffix[RTSP_PARAM_STRING_MAX];

                    urlTotalSuffix[0] = '\0';

                    if (urlPreSuffix[0] != '\0') 
                    {
                        strcat(urlTotalSuffix, urlPreSuffix);

                        strcat(urlTotalSuffix, "/");
                    }

                    strcat(urlTotalSuffix, urlSuffix);

                    if (authenticationOK("SETUP", urlTotalSuffix, reinterpret_cast<char const*>(fRequestBuffer)))
                    {
                        u_int32_t sessionId;

                        do 
                        {
                            sessionId = static_cast<u_int32_t>(our_random32());

                            sprintf(sessionIdStr, "%08X", sessionId);
                        }
                        while (sessionId == 0 || __live_rtsp_server.fClientSessions->Lookup(sessionIdStr) != nullptr);

                        clientSession = static_cast<_live_rtsp_client_session*>(__live_rtsp_server.createNewClientSession(sessionId));

                        __live_rtsp_server.fClientSessions->Add(sessionIdStr, clientSession);
                    }
                    else 
                    {
                        areAuthenticated = false;
                    }
                }

                if (clientSession != nullptr) 
                {
                    auto request_buffer = fRequestBuffer;
                    auto response_buffer = fResponseBuffer;

                    std::string urlPreSuffix_ = urlPreSuffix;
                    std::string urlSuffix_ = urlSuffix;

                    envir().taskScheduler().disableBackgroundHandling(fOurSocket);

                    clientSession->begin_handle_setup(this, urlPreSuffix, urlSuffix, reinterpret_cast<char const*>(fRequestBuffer)).
                    then([=]() mutable
                    {
                        playAfterSetup = clientSession->stream_after_setup_get();
                        
                        send(fClientOutputSocket, 
                             reinterpret_cast<char const*>(response_buffer), 
                             strlen(reinterpret_cast<char*>(response_buffer)), 
                             0);

                        if (playAfterSetup)
                        {
                            // The client has asked for streaming to commence now, rather than after a
                            // subsequent "PLAY" command.  So, simulate the effect of a "PLAY" command:
                            clientSession->handle_cmd_within_session(this, 
                                                                     "PLAY", 
                                                                     urlPreSuffix_.c_str(), 
                                                                     urlSuffix_.c_str(), 
                                                                     reinterpret_cast<char const*>(request_buffer));
                        }

                        unsigned requestSize = (fLastCRLF + 4 - fRequestBuffer) + contentLength;

                        numBytesRemaining = fRequestBytesAlreadySeen - requestSize;

                        resetRequestBuffer(); // to prepare for any subsequent request

                        if (numBytesRemaining > 0)
                        {
                            memmove(fRequestBuffer, &fRequestBuffer[requestSize], numBytesRemaining);

                            newBytesRead = numBytesRemaining;
                        }

                        if (numBytesRemaining > 0)
                        {
                            handleRequestBytes(numBytesRemaining);
                        }

                        --fRecursionCount;

                        if (!fIsActive)
                        {
                            if (fRecursionCount > 0)
                            {
                                closeSockets();
                            }
                            else
                            {
                                delete this;
                            }
                        }
                        else
                        {
                            envir().taskScheduler().setBackgroundHandling(fOurSocket, 
                                                                          SOCKET_READABLE | SOCKET_EXCEPTION, 
                                                                          incomingRequestHandler, 
                                                                          this);
                        }
                    });

                    return;
                }
                else if (areAuthenticated) 
                {
                    handleCmd_sessionNotFound();
                }
            }
            else if (strcmp(cmdName, "TEARDOWN")      == 0
                  || strcmp(cmdName, "PLAY")          == 0
                  || strcmp(cmdName, "PAUSE")         == 0
                  || strcmp(cmdName, "GET_PARAMETER") == 0
                  || strcmp(cmdName, "SET_PARAMETER") == 0) 
            {
                if (clientSession != nullptr) 
                {
                    clientSession->handle_cmd_within_session(this, 
                                                             cmdName, 
                                                             urlPreSuffix, 
                                                             urlSuffix, 
                                                             reinterpret_cast<char const*>(fRequestBuffer));
                }
                else 
                {
                    handleCmd_sessionNotFound();
                }
            }
            else if (strcmp(cmdName, "REGISTER") == 0) 
            {
                // Because - unlike other commands - an implementation of this command needs
                // the entire URL, we re-parse the command to get it:
                std::unique_ptr<char[]> url(strDupSize(reinterpret_cast<char*>(fRequestBuffer)));

                if (sscanf(reinterpret_cast<char*>(fRequestBuffer), "%*s %s", url.get()) == 1) 
                {
                    // Check for special command-specific parameters in a "Transport:" header:
                    bool reuseConnection, deliverViaTCP;

                    std::string proxyURLSuffix;

                    parseTransportHeaderForREGISTER(reinterpret_cast<const char*>(fRequestBuffer), 
                                                    reuseConnection, 
                                                    deliverViaTCP, 
                                                    proxyURLSuffix);

                    handleCmd_REGISTER(url.get(), 
                                       urlSuffix, 
                                       reinterpret_cast<char const*>(fRequestBuffer), 
                                       reuseConnection, 
                                       deliverViaTCP, 
                                       proxyURLSuffix.c_str());
                }
                else 
                {
                    handleCmd_bad();
                }
            }
            else 
            {
                // The command is one that we don't handle:
                handleCmd_notSupported();
            }
        }
        else 
        {
#ifdef DEBUG
            fprintf(stderr, "parseRTSPRequestString() failed; checking now for HTTP commands (for RTSP-over-HTTP tunneling)...\n");
#endif
            // The request was not (valid) RTSP, but check for a special case: HTTP commands (for setting up RTSP-over-HTTP tunneling):
            char sessionCookie[RTSP_PARAM_STRING_MAX];

            char acceptStr[RTSP_PARAM_STRING_MAX];

            *fLastCRLF = '\0'; // temporarily, for parsing

            parseSucceeded = parseHTTPRequestString(cmdName,       sizeof cmdName,
                                                    urlSuffix,     sizeof urlPreSuffix,
                                                    sessionCookie, sizeof sessionCookie,
                                                    acceptStr,     sizeof acceptStr);
            *fLastCRLF = '\r';
            if (parseSucceeded) 
            {
#ifdef DEBUG
                fprintf(stderr, "parseHTTPRequestString() succeeded, returning cmdName \"%s\", urlSuffix \"%s\", sessionCookie \"%s\", acceptStr \"%s\"\n", cmdName, urlSuffix, sessionCookie, acceptStr);
#endif
                // Check that the HTTP command is valid for RTSP-over-HTTP tunneling: There must be a 'session cookie'.
                bool isValidHTTPCmd = true;

                if (strcmp(cmdName, "OPTIONS") == 0) 
                {
                    handleHTTPCmd_OPTIONS();
                }
                else if (sessionCookie[0] == '\0') {
                    // There was no "x-sessioncookie:" header.  If there was an "Accept: application/x-rtsp-tunnelled" header,
                    // then this is a bad tunneling request.  Otherwise, assume that it's an attempt to access the stream via HTTP.
                    if (strcmp(acceptStr, "application/x-rtsp-tunnelled") == 0) 
                    {
                        isValidHTTPCmd = false;
                    }
                    else 
                    {
                        handleHTTPCmd_StreamingGET(urlSuffix, reinterpret_cast<char const*>(fRequestBuffer));
                    }
                }
                else if (strcmp(cmdName, "GET") == 0) 
                {
                    handleHTTPCmd_TunnelingGET(sessionCookie);
                }
                else if (strcmp(cmdName, "POST") == 0)
                {
                    // We might have received additional data following the HTTP "POST" command - i.e., the first Base64-encoded RTSP command.
                    // Check for this, and handle it if it exists:
                    unsigned char const* extraData = fLastCRLF + 4;

                    unsigned extraDataSize = &fRequestBuffer[fRequestBytesAlreadySeen] - extraData;

                    if (handleHTTPCmd_TunnelingPOST(sessionCookie, extraData, extraDataSize))
                    {
                        // We don't respond to the "POST" command, and we go away:
                        fIsActive = false;

                        break;
                    }
                }
                else 
                {
                    isValidHTTPCmd = false;
                }
                if (!isValidHTTPCmd) 
                {
                    handleHTTPCmd_notSupported();
                }
            }
            else 
            {
#ifdef DEBUG
                fprintf(stderr, "parseHTTPRequestString() failed!\n");
#endif
                handleCmd_bad();
            }
        }

#ifdef DEBUG
        fprintf(stderr, "sending response: %s", fResponseBuffer);
#endif
        send(fClientOutputSocket, 
             reinterpret_cast<char const*>(fResponseBuffer), 
             strlen(reinterpret_cast<char*>(fResponseBuffer)), 
             0);
     
        if (playAfterSetup)
        {
            // The client has asked for streaming to commence now, rather than after a
            // subsequent "PLAY" command.  So, simulate the effect of a "PLAY" command:
            clientSession->handle_cmd_within_session(this, 
                                                     "PLAY", 
                                                     urlPreSuffix, 
                                                     urlSuffix, 
                                                     reinterpret_cast<char const*>(fRequestBuffer));
        }

        // Check whether there are extra bytes remaining in the buffer, after the end of the request (a rare case).
        // If so, move them to the front of our buffer, and keep processing it, because it might be a following, pipelined request.
        unsigned requestSize = (fLastCRLF + 4 - fRequestBuffer) + contentLength;

        numBytesRemaining = fRequestBytesAlreadySeen - requestSize;

        resetRequestBuffer(); // to prepare for any subsequent request

        if (numBytesRemaining > 0) 
        {
            memmove(fRequestBuffer, &fRequestBuffer[requestSize], numBytesRemaining);

            newBytesRead = numBytesRemaining;
        }
    } 
    while (numBytesRemaining > 0);

    --fRecursionCount;

    if (!fIsActive)
    {
        if (fRecursionCount > 0)
        {
            closeSockets();
        }
        else
        {
            delete this;
        }

        // Note: The "fRecursionCount" test is for a pathological situation where we reenter the event loop and get called recursively
        // while handling a command (e.g., while handling a "DESCRIBE", to get a SDP description).
        // In such a case we don't want to actually delete ourself until we leave the outermost call.
    }
}
