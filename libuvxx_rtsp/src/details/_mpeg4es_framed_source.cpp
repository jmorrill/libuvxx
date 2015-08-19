#include "details/_mpeg4es_framed_source.hpp"
#include "media_sample.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

_mpeg4es_framed_source::_mpeg4es_framed_source(int stream_id, unsigned session_id) :
    _live_framed_source(stream_id, session_id)
  
{
}

_mpeg4es_framed_source::~_mpeg4es_framed_source()
{

}
