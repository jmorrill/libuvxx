#include <functional>
#include "Base64.hh"
#include "GroupsockHelper.hh"
#include "RTSPCommon.hh"

#include "details/_live_common.hpp"
#include "details/_live_rtsp_server.hpp"
#include <pplx/pplxtasks.h>

using namespace uvxx::rtsp::details;

typedef enum StreamingMode {
    RTP_UDP,
    RTP_TCP,
    RAW_UDP
} StreamingMode;


static void parseTransportHeader(char const* buf,
    StreamingMode& streamingMode,
    char*& streamingModeString,
    char*& destinationAddressStr,
    u_int8_t& destinationTTL,
    portNumBits& clientRTPPortNum, // if UDP
    portNumBits& clientRTCPPortNum, // if UDP
    unsigned char& rtpChannelId, // if TCP
    unsigned char& rtcpChannelId // if TCP
    ) {
    // Initialize the result parameters to default values:
    streamingMode = RTP_UDP;
    streamingModeString = NULL;
    destinationAddressStr = NULL;
    destinationTTL = 255;
    clientRTPPortNum = 0;
    clientRTCPPortNum = 1;
    rtpChannelId = rtcpChannelId = 0xFF;

    portNumBits p1, p2;
    unsigned ttl, rtpCid, rtcpCid;

    // First, find "Transport:"
    while (1) {
        if (*buf == '\0') return; // not found
        if (*buf == '\r' && *(buf + 1) == '\n' && *(buf + 2) == '\r') return; // end of the headers => not found
        if (_strncasecmp(buf, "Transport:", 10) == 0) break;
        ++buf;
    }

    // Then, run through each of the fields, looking for ones we handle:
    char const* fields = buf + 10;
    while (*fields == ' ') ++fields;
    char* field = strDupSize(fields);
    while (sscanf(fields, "%[^;\r\n]", field) == 1) {
        if (strcmp(field, "RTP/AVP/TCP") == 0) {
            streamingMode = RTP_TCP;
        }
        else if (strcmp(field, "RAW/RAW/UDP") == 0 ||
            strcmp(field, "MP2T/H2221/UDP") == 0) {
            streamingMode = RAW_UDP;
            streamingModeString = strDup(field);
        }
        else if (_strncasecmp(field, "destination=", 12) == 0) {
            delete[] destinationAddressStr;
            destinationAddressStr = strDup(field + 12);
        }
        else if (sscanf(field, "ttl%u", &ttl) == 1) {
            destinationTTL = (u_int8_t)ttl;
        }
        else if (sscanf(field, "client_port=%hu-%hu", &p1, &p2) == 2) {
            clientRTPPortNum = p1;
            clientRTCPPortNum = streamingMode == RAW_UDP ? 0 : p2; // ignore the second port number if the client asked for raw UDP
        }
        else if (sscanf(field, "client_port=%hu", &p1) == 1) {
            clientRTPPortNum = p1;
            clientRTCPPortNum = streamingMode == RAW_UDP ? 0 : p1 + 1;
        }
        else if (sscanf(field, "interleaved=%u-%u", &rtpCid, &rtcpCid) == 2) {
            rtpChannelId = (unsigned char)rtpCid;
            rtcpChannelId = (unsigned char)rtcpCid;
        }

        fields += strlen(field);
        while (*fields == ';' || *fields == ' ' || *fields == '\t') ++fields; // skip over separating ';' chars or whitespace
        if (*fields == '\0' || *fields == '\r' || *fields == '\n') break;
    }
    delete[] field;
}

static Boolean parsePlayNowHeader(char const* buf) {
    // Find "x-playNow:" header, if present
    while (1) {
        if (*buf == '\0') return False; // not found
        if (_strncasecmp(buf, "x-playNow:", 10) == 0) break;
        ++buf;
    }

    return True;
}


_live_rtsp_server::_live_rtsp_server(uint16_t port) : 
    RTSPServerSupportingHTTPStreaming(*_get_live_environment().get(), setup_socket(port),
        port, 
        nullptr, 
        65)
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

ServerMediaSession* _live_rtsp_server::lookupServerMediaSession(char const* stream_name, Boolean /*is_first_lookup_in_session*/)
{
    return nullptr;// __lookup_media_session_delegate ? __lookup_media_session_delegate(stream_name) : nullptr;
}

void _live_rtsp_server::begin_lookup_server_media_session(char const* stream_name, Boolean is_first_lookup_in_session, _server_media_session_callback_delegate callback)
{
    __lookup_media_session_delegate(stream_name).then([=](pplx::task<ServerMediaSession*> t)
    {
        callback(t.get());
    });
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

    return socket;
}


#define RTSP_PARAM_STRING_MAX 200

using namespace uvxx::rtsp::details;


// A special version of "parseTransportHeader()", used just for parsing the "Transport:" header in an incoming "REGISTER" command:
static void parseTransportHeaderForREGISTER(char const* buf,
    Boolean &reuseConnection,
    Boolean& deliverViaTCP,
    char*& proxyURLSuffix) {
    // Initialize the result parameters to default values:
    reuseConnection = False;
    deliverViaTCP = False;
    proxyURLSuffix = NULL;

    // First, find "Transport:"
    while (1) {
        if (*buf == '\0') return; // not found
        if (*buf == '\r' && *(buf + 1) == '\n' && *(buf + 2) == '\r') return; // end of the headers => not found
        if (_strncasecmp(buf, "Transport:", 10) == 0) break;
        ++buf;
    }

    // Then, run through each of the fields, looking for ones we handle:
    char const* fields = buf + 10;
    while (*fields == ' ') ++fields;
    char* field = strDupSize(fields);
    while (sscanf(fields, "%[^;\r\n]", field) == 1) {
        if (strcmp(field, "reuse_connection") == 0) {
            reuseConnection = True;
        }
        else if (_strncasecmp(field, "preferred_delivery_protocol=udp", 31) == 0) {
            deliverViaTCP = False;
        }
        else if (_strncasecmp(field, "preferred_delivery_protocol=interleaved", 39) == 0) {
            deliverViaTCP = True;
        }
        else if (_strncasecmp(field, "proxy_url_suffix=", 17) == 0) {
            delete[] proxyURLSuffix;
            proxyURLSuffix = strDup(field + 17);
        }

        fields += strlen(field);
        while (*fields == ';' || *fields == ' ' || *fields == '\t') ++fields; // skip over separating ';' chars or whitespace
        if (*fields == '\0' || *fields == '\r' || *fields == '\n') break;
    }
    delete[] field;
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

void _live_rtsp_server::_live_rtsp_client_session::begin_handle_setup(_live_rtsp_client_connection* our_client_connection, const std::string& url_pre_suffix, const std::string& url_suffix, const std::string& full_request_str, std::function<void()> end_setup_callback)
{
    std::string cseq = our_client_connection->current_cseq();

    _our_server.begin_lookup_server_media_session(url_pre_suffix.c_str(), true,[=](ServerMediaSession* sms)
    {
        handle_cmd_setup(our_client_connection, sms, cseq.c_str(), url_pre_suffix.c_str(), url_suffix.c_str(), full_request_str.c_str());
        end_setup_callback();
    });
}

void _live_rtsp_server::_live_rtsp_client_session::handle_cmd_setup(_live_rtsp_client_connection* ourClientConnection, ServerMediaSession* sms, char const* cseq, char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr)
{
    // Normally, "urlPreSuffix" should be the session (stream) name, and "urlSuffix" should be the subsession (track) name.
    // However (being "liberal in what we accept"), we also handle 'aggregate' SETUP requests (i.e., without a track name),
    // in the special case where we have only a single track.  I.e., in this case, we also handle:
    //    "urlPreSuffix" is empty and "urlSuffix" is the session (stream) name, or
    //    "urlPreSuffix" concatenated with "urlSuffix" (with "/" inbetween) is the session (stream) name.
    char const* streamName = urlPreSuffix; // in the normal case
    char const* trackId = urlSuffix; // in the normal case
    char* concatenatedStreamName = NULL; // in the normal case

    do {
        // First, make sure the specified stream name exists:
       
        if (sms == NULL) {
            // Check for the special case (noted above), before we give up:
            if (urlPreSuffix[0] == '\0') {
                streamName = urlSuffix;
            }
            else {
                concatenatedStreamName = new char[strlen(urlPreSuffix) + strlen(urlSuffix) + 2]; // allow for the "/" and the trailing '\0'
                sprintf(concatenatedStreamName, "%s/%s", urlPreSuffix, urlSuffix);
                streamName = concatenatedStreamName;
            }
            trackId = NULL;

            // Check again:
            sms = fOurServer.lookupServerMediaSession(streamName, fOurServerMediaSession == NULL);
        }
        if (sms == NULL) {
            if (fOurServerMediaSession == NULL) {
                // The client asked for a stream that doesn't exist (and this session descriptor has not been used before):
                ourClientConnection->handleCmd_notFound();
            }
            else {
                // The client asked for a stream that doesn't exist, but using a stream id for a stream that does exist. Bad request:
                ourClientConnection->handleCmd_bad();
            }
            break;
        }
        else {
            if (fOurServerMediaSession == NULL) {
                // We're accessing the "ServerMediaSession" for the first time.
                fOurServerMediaSession = sms;
                fOurServerMediaSession->incrementReferenceCount();
            }
            else if (sms != fOurServerMediaSession) {
                // The client asked for a stream that's different from the one originally requested for this stream id.  Bad request:
                ourClientConnection->handleCmd_bad();
                break;
            }
        }

        if (fStreamStates == NULL) {
            // This is the first "SETUP" for this session.  Set up our array of states for all of this session's subsessions (tracks):
            ServerMediaSubsessionIterator iter(*fOurServerMediaSession);
            for (fNumStreamStates = 0; iter.next() != NULL; ++fNumStreamStates) {} // begin by counting the number of subsessions (tracks)

            fStreamStates = new struct streamState[fNumStreamStates];

            iter.reset();
            ServerMediaSubsession* subsession;
            for (unsigned i = 0; i < fNumStreamStates; ++i) {
                subsession = iter.next();
                fStreamStates[i].subsession = subsession;
                fStreamStates[i].tcpSocketNum = -1; // for now; may get set for RTP-over-TCP streaming
                fStreamStates[i].streamToken = NULL; // for now; it may be changed by the "getStreamParameters()" call that comes later
            }
        }

        // Look up information for the specified subsession (track):
        ServerMediaSubsession* subsession = NULL;
        unsigned trackNum;
        if (trackId != NULL && trackId[0] != '\0') { // normal case
            for (trackNum = 0; trackNum < fNumStreamStates; ++trackNum) {
                subsession = fStreamStates[trackNum].subsession;
                if (subsession != NULL && strcmp(trackId, subsession->trackId()) == 0) break;
            }
            if (trackNum >= fNumStreamStates) {
                // The specified track id doesn't exist, so this request fails:
                ourClientConnection->handleCmd_notFound();
                break;
            }
        }
        else {
            // Weird case: there was no track id in the URL.
            // This works only if we have only one subsession:
            if (fNumStreamStates != 1 || fStreamStates[0].subsession == NULL) {
                ourClientConnection->handleCmd_bad();
                break;
            }
            trackNum = 0;
            subsession = fStreamStates[trackNum].subsession;
        }
        // ASSERT: subsession != NULL

        void*& token = fStreamStates[trackNum].streamToken; // alias
        if (token != NULL) {
            // We already handled a "SETUP" for this track (to the same client),
            // so stop any existing streaming of it, before we set it up again:
            subsession->pauseStream(fOurSessionId, token);
            _our_server.unnoteTCPStreamingOnSocket(fStreamStates[trackNum].tcpSocketNum, this, trackNum);
            subsession->deleteStream(fOurSessionId, token);
        }

        // Look for a "Transport:" header in the request string, to extract client parameters:
        StreamingMode streamingMode;
        char* streamingModeString = NULL; // set when RAW_UDP streaming is specified
        char* clientsDestinationAddressStr;
        u_int8_t clientsDestinationTTL;
        portNumBits clientRTPPortNum, clientRTCPPortNum;
        unsigned char rtpChannelId, rtcpChannelId;
        parseTransportHeader(fullRequestStr, streamingMode, streamingModeString,
            clientsDestinationAddressStr, clientsDestinationTTL,
            clientRTPPortNum, clientRTCPPortNum,
            rtpChannelId, rtcpChannelId);
        if ((streamingMode == RTP_TCP && rtpChannelId == 0xFF) ||
            (streamingMode != RTP_TCP && ourClientConnection->client_output_socket() != ourClientConnection->client_input_socket())) {
            // An anomolous situation, caused by a buggy client.  Either:
            //     1/ TCP streaming was requested, but with no "interleaving=" fields.  (QuickTime Player sometimes does this.), or
            //     2/ TCP streaming was not requested, but we're doing RTSP-over-HTTP tunneling (which implies TCP streaming).
            // In either case, we assume TCP streaming, and set the RTP and RTCP channel ids to proper values:
            streamingMode = RTP_TCP;
            rtpChannelId = fTCPStreamIdCount; rtcpChannelId = fTCPStreamIdCount + 1;
        }
        if (streamingMode == RTP_TCP) fTCPStreamIdCount += 2;

        Port clientRTPPort(clientRTPPortNum);
        Port clientRTCPPort(clientRTCPPortNum);

        // Next, check whether a "Range:" or "x-playNow:" header is present in the request.
        // This isn't legal, but some clients do this to combine "SETUP" and "PLAY":
        double rangeStart = 0.0, rangeEnd = 0.0;
        char* absStart = NULL; char* absEnd = NULL;
        Boolean startTimeIsNow;
        if (parseRangeHeader(fullRequestStr, rangeStart, rangeEnd, absStart, absEnd, startTimeIsNow)) {
            delete[] absStart; delete[] absEnd;
            fStreamAfterSETUP = True;
        }
        else if (parsePlayNowHeader(fullRequestStr)) {
            fStreamAfterSETUP = True;
        }
        else {
            fStreamAfterSETUP = False;
        }

        // Then, get server parameters from the 'subsession':
        if (streamingMode == RTP_TCP) {
            // Note that we'll be streaming over the RTSP TCP connection:
            fStreamStates[trackNum].tcpSocketNum = ourClientConnection->client_output_socket();
            _our_server.noteTCPStreamingOnSocket(fStreamStates[trackNum].tcpSocketNum, this, trackNum);
        }
        netAddressBits destinationAddress = 0;
        u_int8_t destinationTTL = 255;
#ifdef RTSP_ALLOW_CLIENT_DESTINATION_SETTING
        if (clientsDestinationAddressStr != NULL) {
            // Use the client-provided "destination" address.
            // Note: This potentially allows the server to be used in denial-of-service
            // attacks, so don't enable this code unless you're sure that clients are
            // trusted.
            destinationAddress = our_inet_addr(clientsDestinationAddressStr);
        }
        // Also use the client-provided TTL.
        destinationTTL = clientsDestinationTTL;
#endif
        delete[] clientsDestinationAddressStr;
        Port serverRTPPort(0);
        Port serverRTCPPort(0);

        // Make sure that we transmit on the same interface that's used by the client (in case we're a multi-homed server):
        struct sockaddr_in sourceAddr; SOCKLEN_T namelen = sizeof sourceAddr;
        getsockname(ourClientConnection->client_input_socket(), (struct sockaddr*)&sourceAddr, &namelen);
        netAddressBits origSendingInterfaceAddr = SendingInterfaceAddr;
        netAddressBits origReceivingInterfaceAddr = ReceivingInterfaceAddr;
        // NOTE: The following might not work properly, so we ifdef it out for now:
#ifdef HACK_FOR_MULTIHOMED_SERVERS
        ReceivingInterfaceAddr = SendingInterfaceAddr = sourceAddr.sin_addr.s_addr;
#endif

        subsession->getStreamParameters(fOurSessionId, ourClientConnection->client_addr().sin_addr.s_addr,
            clientRTPPort, clientRTCPPort,
            fStreamStates[trackNum].tcpSocketNum, rtpChannelId, rtcpChannelId,
            destinationAddress, destinationTTL, fIsMulticast,
            serverRTPPort, serverRTCPPort,
            fStreamStates[trackNum].streamToken);
        SendingInterfaceAddr = origSendingInterfaceAddr;
        ReceivingInterfaceAddr = origReceivingInterfaceAddr;

        AddressString destAddrStr(destinationAddress);
        AddressString sourceAddrStr(sourceAddr);
        char timeoutParameterString[100];
        if (_our_server.fReclamationSeconds > 0) {
            sprintf(timeoutParameterString, ";timeout=%u", _our_server.fReclamationSeconds);
        }
        else {
            timeoutParameterString[0] = '\0';
        }
        if (fIsMulticast) {
            switch (streamingMode) {
            case RTP_UDP: {
                snprintf((char*)ourClientConnection->response_buffer(), ourClientConnection->response_buffer_size(),
                    "RTSP/1.0 200 OK\r\n"
                    "CSeq: %s\r\n"
                    "%s"
                    "Transport: RTP/AVP;multicast;destination=%s;source=%s;port=%d-%d;ttl=%d\r\n"
                    "Session: %08X%s\r\n\r\n",
                    cseq,
                    dateHeader(),
                    destAddrStr.val(), sourceAddrStr.val(), ntohs(serverRTPPort.num()), ntohs(serverRTCPPort.num()), destinationTTL,
                    fOurSessionId, timeoutParameterString);
                break;
            }
            case RTP_TCP: {
                // multicast streams can't be sent via TCP
                ourClientConnection->handleCmd_unsupportedTransport();
                break;
            }
            case RAW_UDP: {
                snprintf((char*)ourClientConnection->response_buffer(), ourClientConnection->response_buffer_size(),
                    "RTSP/1.0 200 OK\r\n"
                    "CSeq: %s\r\n"
                    "%s"
                    "Transport: %s;multicast;destination=%s;source=%s;port=%d;ttl=%d\r\n"
                    "Session: %08X%s\r\n\r\n",
                    cseq,
                    dateHeader(),
                    streamingModeString, destAddrStr.val(), sourceAddrStr.val(), ntohs(serverRTPPort.num()), destinationTTL,
                    fOurSessionId, timeoutParameterString);
                break;
            }
            }
        }
        else {
            switch (streamingMode) {
            case RTP_UDP: {
                snprintf((char*)ourClientConnection->response_buffer(), ourClientConnection->response_buffer_size(),
                    "RTSP/1.0 200 OK\r\n"
                    "CSeq: %s\r\n"
                    "%s"
                    "Transport: RTP/AVP;unicast;destination=%s;source=%s;client_port=%d-%d;server_port=%d-%d\r\n"
                    "Session: %08X%s\r\n\r\n",
                    cseq,
                    dateHeader(),
                    destAddrStr.val(), sourceAddrStr.val(), ntohs(clientRTPPort.num()), ntohs(clientRTCPPort.num()), ntohs(serverRTPPort.num()), ntohs(serverRTCPPort.num()),
                    fOurSessionId, timeoutParameterString);
                break;
            }
            case RTP_TCP: {
                if (!_our_server.allow_streaming_rtp_over_tcp()) {
                    ourClientConnection->handleCmd_unsupportedTransport();
                }
                else {
                    snprintf((char*)ourClientConnection->response_buffer(), ourClientConnection->response_buffer_size(),
                        "RTSP/1.0 200 OK\r\n"
                        "CSeq: %s\r\n"
                        "%s"
                        "Transport: RTP/AVP/TCP;unicast;destination=%s;source=%s;interleaved=%d-%d\r\n"
                        "Session: %08X%s\r\n\r\n",
                        cseq,
                        dateHeader(),
                        destAddrStr.val(), sourceAddrStr.val(), rtpChannelId, rtcpChannelId,
                        fOurSessionId, timeoutParameterString);
                }
                break;
            }
            case RAW_UDP: {
                snprintf((char*)ourClientConnection->response_buffer(), ourClientConnection->response_buffer_size(),
                    "RTSP/1.0 200 OK\r\n"
                    "CSeq: %s\r\n"
                    "%s"
                    "Transport: %s;unicast;destination=%s;source=%s;client_port=%d;server_port=%d\r\n"
                    "Session: %08X%s\r\n\r\n",
                    cseq,
                    dateHeader(),
                    streamingModeString, destAddrStr.val(), sourceAddrStr.val(), ntohs(clientRTPPort.num()), ntohs(serverRTPPort.num()),
                    fOurSessionId, timeoutParameterString);
                break;
            }
            }
        }
        delete[] streamingModeString;
    } while (0);

    delete[] concatenatedStreamName;
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

void _live_rtsp_server::_live_rtsp_client_connection::handleCmd_DESCRIBE(char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr) {
    ServerMediaSession* session = NULL;
    char* sdpDescription = NULL;
    char* rtspURL = NULL;
    do {
        char urlTotalSuffix[RTSP_PARAM_STRING_MAX];
        urlTotalSuffix[0] = '\0';
        if (urlPreSuffix[0] != '\0') {
            strcat(urlTotalSuffix, urlPreSuffix);
            strcat(urlTotalSuffix, "/");
        }
        strcat(urlTotalSuffix, urlSuffix);

        if (!authenticationOK("DESCRIBE", urlTotalSuffix, fullRequestStr)) break;

        // We should really check that the request contains an "Accept:" #####
        // for "application/sdp", because that's what we're sending back #####

        // Begin by looking up the "ServerMediaSession" object for the specified "urlTotalSuffix":
        session = fOurServer.lookupServerMediaSession(urlTotalSuffix);
        if (session == NULL) {
            handleCmd_notFound();
            break;
        }

        // Increment the "ServerMediaSession" object's reference count, in case someone removes it
        // while we're using it:
        session->incrementReferenceCount();

        // Then, assemble a SDP description for this session:
        sdpDescription = session->generateSDPDescription();
        if (sdpDescription == NULL) {
            // This usually means that a file name that was specified for a
            // "ServerMediaSubsession" does not exist.
            setRTSPResponse("404 File Not Found, Or In Incorrect Format");
            break;
        }
        unsigned sdpDescriptionSize = strlen(sdpDescription);

        // Also, generate our RTSP URL, for the "Content-Base:" header
        // (which is necessary to ensure that the correct URL gets used in subsequent "SETUP" requests).
        rtspURL = fOurRTSPServer.rtspURL(session, fClientInputSocket);

        snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
            "RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
            "%s"
            "Content-Base: %s/\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: %d\r\n\r\n"
            "%s",
            fCurrentCSeq,
            dateHeader(),
            rtspURL,
            sdpDescriptionSize,
            sdpDescription);
    } while (0);

    if (session != NULL) {
        // Decrement its reference count, now that we're done using it:
        session->decrementReferenceCount();
        if (session->referenceCount() == 0 && session->deleteWhenUnreferenced()) {
            fOurServer.removeServerMediaSession(session);
        }
    }

    delete[] sdpDescription;
    delete[] rtspURL;
}

void _live_rtsp_server::_live_rtsp_client_connection::handleCmd_unsupportedTransport()
{
    RTSPClientConnection::handleCmd_unsupportedTransport();
}

void _live_rtsp_server::_live_rtsp_client_connection::begin_handle_describe(char const* url_pre_suffix, char const* url_suffix, char const* full_request_str, std::function<void()> end_describe_callback)
{
    ServerMediaSession* session = NULL;

    char* sdpDescription = NULL;

    char* rtspURL = NULL;

    char urlTotalSuffix[RTSP_PARAM_STRING_MAX];

    urlTotalSuffix[0] = '\0';

    if (url_pre_suffix[0] != '\0') 
    {
        strcat(urlTotalSuffix, url_pre_suffix);

        strcat(urlTotalSuffix, "/");
    }

    strcat(urlTotalSuffix, url_suffix);

    std::shared_ptr<char> description_ptr(sdpDescription, [](char* p) {delete[] p; });

    std::shared_ptr<char> rtsp_url_ptr(rtspURL, [](char* p) {delete[] p; });

    if (!authenticationOK("DESCRIBE", urlTotalSuffix, full_request_str))
    {
        end_describe_callback();
        return;
    }
  
    std::string cseq = fCurrentCSeq;

    __live_rtsp_server.begin_lookup_server_media_session(urlTotalSuffix, true, [=](ServerMediaSession* session) mutable
    {
        auto p1 = description_ptr;
        auto p2 = rtsp_url_ptr;

        do {

        if (session == NULL) 
        {
            handleCmd_notFound();
            break;
        }

        // Increment the "ServerMediaSession" object's reference count, in case someone removes it
        // while we're using it:
        session->incrementReferenceCount();

        // Then, assemble a SDP description for this session:
        sdpDescription = session->generateSDPDescription();
        if (sdpDescription == NULL) {
            // This usually means that a file name that was specified for a
            // "ServerMediaSubsession" does not exist.
            setRTSPResponse("404 File Not Found, Or In Incorrect Format");
            break;
        }
        unsigned sdpDescriptionSize = strlen(sdpDescription);

        // Also, generate our RTSP URL, for the "Content-Base:" header
        // (which is necessary to ensure that the correct URL gets used in subsequent "SETUP" requests).
        rtspURL = fOurRTSPServer.rtspURL(session, fClientInputSocket);

        snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
            "RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
            "%s"
            "Content-Base: %s/\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: %d\r\n\r\n"
            "%s",
            cseq.c_str(),
            dateHeader(),
            rtspURL,
            sdpDescriptionSize,
            sdpDescription);
        } while (0);

        if (session != NULL) {
            // Decrement its reference count, now that we're done using it:
            session->decrementReferenceCount();
            if (session->referenceCount() == 0 && session->deleteWhenUnreferenced()) {
                fOurServer.removeServerMediaSession(session);
            }
        }

        end_describe_callback();
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

    do {
        _live_rtsp_client_session* clientSession = NULL;

        if (newBytesRead < 0 || (unsigned)newBytesRead >= fRequestBufferBytesLeft) {
            // Either the client socket has died, or the request was too big for us.
            // Terminate this connection:
#ifdef DEBUG
            fprintf(stderr, "RTSPClientConnection[%p]::handleRequestBytes() read %d new bytes (of %d); terminating connection!\n", this, newBytesRead, fRequestBufferBytesLeft);
#endif
            fIsActive = False;
            break;
        }

        Boolean endOfMsg = False;
        unsigned char* ptr = &fRequestBuffer[fRequestBytesAlreadySeen];
#ifdef DEBUG
        ptr[newBytesRead] = '\0';
        fprintf(stderr, "RTSPClientConnection[%p]::handleRequestBytes() %s %d new bytes:%s\n",
            this, numBytesRemaining > 0 ? "processing" : "read", newBytesRead, ptr);
#endif

        if (fClientOutputSocket != fClientInputSocket && numBytesRemaining == 0) {
            // We're doing RTSP-over-HTTP tunneling, and input commands are assumed to have been Base64-encoded.
            // We therefore Base64-decode as much of this new data as we can (i.e., up to a multiple of 4 bytes).

            // But first, we remove any whitespace that may be in the input data:
            unsigned toIndex = 0;
            for (int fromIndex = 0; fromIndex < newBytesRead; ++fromIndex) {
                char c = ptr[fromIndex];
                if (!(c == ' ' || c == '\t' || c == '\r' || c == '\n')) { // not 'whitespace': space,tab,CR,NL
                    ptr[toIndex++] = c;
                }
            }
            newBytesRead = toIndex;

            unsigned numBytesToDecode = fBase64RemainderCount + newBytesRead;
            unsigned newBase64RemainderCount = numBytesToDecode % 4;
            numBytesToDecode -= newBase64RemainderCount;
            if (numBytesToDecode > 0) {
                ptr[newBytesRead] = '\0';
                unsigned decodedSize;
                unsigned char* decodedBytes = base64Decode((char const*)(ptr - fBase64RemainderCount), numBytesToDecode, decodedSize);
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
                delete[] decodedBytes;
            }
            fBase64RemainderCount = newBase64RemainderCount;
        }

        unsigned char* tmpPtr = fLastCRLF + 2;
        if (fBase64RemainderCount == 0) { // no more Base-64 bytes remain to be read/decoded
                                          // Look for the end of the message: <CR><LF><CR><LF>
            if (tmpPtr < fRequestBuffer) tmpPtr = fRequestBuffer;
            while (tmpPtr < &ptr[newBytesRead - 1]) {
                if (*tmpPtr == '\r' && *(tmpPtr + 1) == '\n') {
                    if (tmpPtr - fLastCRLF == 2) { // This is it:
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
        Boolean parseSucceeded = parseRTSPRequestString((char*)fRequestBuffer, fLastCRLF + 2 - fRequestBuffer,
            cmdName, sizeof cmdName,
            urlPreSuffix, sizeof urlPreSuffix,
            urlSuffix, sizeof urlSuffix,
            cseq, sizeof cseq,
            sessionIdStr, sizeof sessionIdStr,
            contentLength);
        fLastCRLF[2] = '\r'; // restore its value
        Boolean playAfterSetup = False;
        if (parseSucceeded) {
#ifdef DEBUG
            fprintf(stderr, "parseRTSPRequestString() succeeded, returning cmdName \"%s\", urlPreSuffix \"%s\", urlSuffix \"%s\", CSeq \"%s\", Content-Length %u, with %d bytes following the message.\n", cmdName, urlPreSuffix, urlSuffix, cseq, contentLength, ptr + newBytesRead - (tmpPtr + 2));
#endif
            // If there was a "Content-Length:" header, then make sure we've received all of the data that it specified:
            if (ptr + newBytesRead < tmpPtr + 2 + contentLength) break; // we still need more data; subsequent reads will give it to us 

                                                                        // If the request included a "Session:" id, and it refers to a client session that's
                                                                        // current ongoing, then use this command to indicate 'liveness' on that client session:
            Boolean const requestIncludedSessionId = sessionIdStr[0] != '\0';
            if (requestIncludedSessionId) {
                clientSession = (_live_rtsp_client_session*)(__live_rtsp_server.fClientSessions->Lookup(sessionIdStr));
                if (clientSession != NULL) clientSession->note_liveness();
            }

            // We now have a complete RTSP request.
            // Handle the specified command (beginning with commands that are session-independent):
            fCurrentCSeq = cseq;
            if (strcmp(cmdName, "OPTIONS") == 0) {
                // If the "OPTIONS" command included a "Session:" id for a session that doesn't exist,
                // then treat this as an error:
                if (requestIncludedSessionId && clientSession == NULL) {
                    handleCmd_sessionNotFound();
                }
                else {
                    // Normal case:
                    handleCmd_OPTIONS();
                }
            }
            else if (urlPreSuffix[0] == '\0' && urlSuffix[0] == '*' && urlSuffix[1] == '\0') {
                // The special "*" URL means: an operation on the entire server.  This works only for GET_PARAMETER and SET_PARAMETER:
                if (strcmp(cmdName, "GET_PARAMETER") == 0) {
                    handleCmd_GET_PARAMETER((char const*)fRequestBuffer);
                }
                else if (strcmp(cmdName, "SET_PARAMETER") == 0) {
                    handleCmd_SET_PARAMETER((char const*)fRequestBuffer);
                }
                else {
                    handleCmd_notSupported();
                }
            }
            else if (strcmp(cmdName, "DESCRIBE") == 0) {
                begin_handle_describe(urlPreSuffix, urlSuffix, (char const*)fRequestBuffer, [=]() mutable
                {
                    send(fClientOutputSocket, (char const*)fResponseBuffer, strlen((char*)fResponseBuffer), 0);

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

                    if(numBytesRemaining)
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
                });

                return;
            }
            else if (strcmp(cmdName, "SETUP") == 0) {
                Boolean areAuthenticated = True;

                if (!requestIncludedSessionId) {
                    // No session id was present in the request.  So create a new "RTSPClientSession" object
                    // for this request.  Choose a random (unused) 32-bit integer for the session id
                    // (it will be encoded as a 8-digit hex number).  (We avoid choosing session id 0,
                    // because that has a special use (by "OnDemandServerMediaSubsession").)

                    // But first, make sure that we're authenticated to perform this command:
                    char urlTotalSuffix[RTSP_PARAM_STRING_MAX];
                    urlTotalSuffix[0] = '\0';
                    if (urlPreSuffix[0] != '\0') {
                        strcat(urlTotalSuffix, urlPreSuffix);
                        strcat(urlTotalSuffix, "/");
                    }
                    strcat(urlTotalSuffix, urlSuffix);
                    if (authenticationOK("SETUP", urlTotalSuffix, (char const*)fRequestBuffer)) {
                        u_int32_t sessionId;
                        do {
                            sessionId = (u_int32_t)our_random32();
                            sprintf(sessionIdStr, "%08X", sessionId);
                        } while (sessionId == 0 || __live_rtsp_server.fClientSessions->Lookup(sessionIdStr) != NULL);
                        clientSession = (_live_rtsp_client_session*)__live_rtsp_server.createNewClientSession(sessionId);
                        __live_rtsp_server.fClientSessions->Add(sessionIdStr, clientSession);
                    }
                    else {
                        areAuthenticated = False;
                    }
                }
                if (clientSession != NULL) 
                {
                    unsigned char* request_buffer = fRequestBuffer;
                    unsigned char* response_buffer = fResponseBuffer;

                    std::string urlPreSuffix_ = urlPreSuffix;
                    std::string urlSuffix_ = urlSuffix;

                    clientSession->begin_handle_setup(this, urlPreSuffix, urlSuffix, (char const*)fRequestBuffer,[=]() mutable
                    {
                        playAfterSetup = clientSession->stream_after_setup_get();
                        
                        send(fClientOutputSocket, (char const*)response_buffer, strlen((char*)response_buffer), 0);

                        if (playAfterSetup) {
                            // The client has asked for streaming to commence now, rather than after a
                            // subsequent "PLAY" command.  So, simulate the effect of a "PLAY" command:
                            clientSession->handle_cmd_within_session(this, "PLAY", urlPreSuffix_.c_str(), urlSuffix_.c_str(), (char const*)request_buffer);
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
                    });

                    return;
                }
                else if (areAuthenticated) {
                    handleCmd_sessionNotFound();
                }
            }
            else if (strcmp(cmdName, "TEARDOWN") == 0
                || strcmp(cmdName, "PLAY") == 0
                || strcmp(cmdName, "PAUSE") == 0
                || strcmp(cmdName, "GET_PARAMETER") == 0
                || strcmp(cmdName, "SET_PARAMETER") == 0) {
                if (clientSession != NULL) {
                    clientSession->handle_cmd_within_session(this, cmdName, urlPreSuffix, urlSuffix, (char const*)fRequestBuffer);
                }
                else {
                    handleCmd_sessionNotFound();
                }
            }
            else if (strcmp(cmdName, "REGISTER") == 0) {
                // Because - unlike other commands - an implementation of this command needs
                // the entire URL, we re-parse the command to get it:
                char* url = strDupSize((char*)fRequestBuffer);
                if (sscanf((char*)fRequestBuffer, "%*s %s", url) == 1) {
                    // Check for special command-specific parameters in a "Transport:" header:
                    Boolean reuseConnection, deliverViaTCP;
                    char* proxyURLSuffix;
                    parseTransportHeaderForREGISTER((const char*)fRequestBuffer, reuseConnection, deliverViaTCP, proxyURLSuffix);

                    handleCmd_REGISTER(url, urlSuffix, (char const*)fRequestBuffer, reuseConnection, deliverViaTCP, proxyURLSuffix);
                    delete[] proxyURLSuffix;
                }
                else {
                    handleCmd_bad();
                }
                delete[] url;
            }
            else {
                // The command is one that we don't handle:
                handleCmd_notSupported();
            }
        }
        else {
#ifdef DEBUG
            fprintf(stderr, "parseRTSPRequestString() failed; checking now for HTTP commands (for RTSP-over-HTTP tunneling)...\n");
#endif
            // The request was not (valid) RTSP, but check for a special case: HTTP commands (for setting up RTSP-over-HTTP tunneling):
            char sessionCookie[RTSP_PARAM_STRING_MAX];
            char acceptStr[RTSP_PARAM_STRING_MAX];
            *fLastCRLF = '\0'; // temporarily, for parsing
            parseSucceeded = parseHTTPRequestString(cmdName, sizeof cmdName,
                urlSuffix, sizeof urlPreSuffix,
                sessionCookie, sizeof sessionCookie,
                acceptStr, sizeof acceptStr);
            *fLastCRLF = '\r';
            if (parseSucceeded) {
#ifdef DEBUG
                fprintf(stderr, "parseHTTPRequestString() succeeded, returning cmdName \"%s\", urlSuffix \"%s\", sessionCookie \"%s\", acceptStr \"%s\"\n", cmdName, urlSuffix, sessionCookie, acceptStr);
#endif
                // Check that the HTTP command is valid for RTSP-over-HTTP tunneling: There must be a 'session cookie'.
                Boolean isValidHTTPCmd = True;
                if (strcmp(cmdName, "OPTIONS") == 0) {
                    handleHTTPCmd_OPTIONS();
                }
                else if (sessionCookie[0] == '\0') {
                    // There was no "x-sessioncookie:" header.  If there was an "Accept: application/x-rtsp-tunnelled" header,
                    // then this is a bad tunneling request.  Otherwise, assume that it's an attempt to access the stream via HTTP.
                    if (strcmp(acceptStr, "application/x-rtsp-tunnelled") == 0) {
                        isValidHTTPCmd = False;
                    }
                    else {
                        handleHTTPCmd_StreamingGET(urlSuffix, (char const*)fRequestBuffer);
                    }
                }
                else if (strcmp(cmdName, "GET") == 0) {
                    handleHTTPCmd_TunnelingGET(sessionCookie);
                }
                else if (strcmp(cmdName, "POST") == 0) {
                    // We might have received additional data following the HTTP "POST" command - i.e., the first Base64-encoded RTSP command.
                    // Check for this, and handle it if it exists:
                    unsigned char const* extraData = fLastCRLF + 4;
                    unsigned extraDataSize = &fRequestBuffer[fRequestBytesAlreadySeen] - extraData;
                    if (handleHTTPCmd_TunnelingPOST(sessionCookie, extraData, extraDataSize)) {
                        // We don't respond to the "POST" command, and we go away:
                        fIsActive = False;
                        break;
                    }
                }
                else {
                    isValidHTTPCmd = False;
                }
                if (!isValidHTTPCmd) {
                    handleHTTPCmd_notSupported();
                }
            }
            else {
#ifdef DEBUG
                fprintf(stderr, "parseHTTPRequestString() failed!\n");
#endif
                handleCmd_bad();
            }
        }

#ifdef DEBUG
        fprintf(stderr, "sending response: %s", fResponseBuffer);
#endif
        send(fClientOutputSocket, (char const*)fResponseBuffer, strlen((char*)fResponseBuffer), 0);

        if (playAfterSetup) {
            // The client has asked for streaming to commence now, rather than after a
            // subsequent "PLAY" command.  So, simulate the effect of a "PLAY" command:
            clientSession->handle_cmd_within_session(this, "PLAY", urlPreSuffix, urlSuffix, (char const*)fRequestBuffer);
        }

        // Check whether there are extra bytes remaining in the buffer, after the end of the request (a rare case).
        // If so, move them to the front of our buffer, and keep processing it, because it might be a following, pipelined request.
        unsigned requestSize = (fLastCRLF + 4 - fRequestBuffer) + contentLength;
        numBytesRemaining = fRequestBytesAlreadySeen - requestSize;
        resetRequestBuffer(); // to prepare for any subsequent request

        if (numBytesRemaining > 0) {
            memmove(fRequestBuffer, &fRequestBuffer[requestSize], numBytesRemaining);
            newBytesRead = numBytesRemaining;
        }
    } while (numBytesRemaining > 0);

    --fRecursionCount;
    if (!fIsActive) {
        if (fRecursionCount > 0) closeSockets(); else delete this;
        // Note: The "fRecursionCount" test is for a pathological situation where we reenter the event loop and get called recursively
        // while handling a command (e.g., while handling a "DESCRIBE", to get a SDP description).
        // In such a case we don't want to actually delete ourself until we leave the outermost call.
    }
}
