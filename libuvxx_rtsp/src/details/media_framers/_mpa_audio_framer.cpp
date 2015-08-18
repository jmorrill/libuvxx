#include "details/media_framers/_mpa_audio_framer.hpp"
#include "sample_attributes.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::sample_attributes;
using namespace uvxx::rtsp::details::media_framers;


enum class mpa_version
{
    MPEG25 = 0,
    MPEGReserved,
    MPEG2,
    MPEG1
};

enum class mpa_layer
{
    Layer1,
    Layer2,
    Layer3,
    LayerReserved
};

enum class channel_mode
{
    Stereo,
    JointStereo,
    DualChannel,
    SingleChannel
};

static const uint32_t sampling_rates[4][3] =
{
    { 11025, 12000, 8000, },  /* MPEG 2.5 */
    { 0,     0,     0, },     /* reserved */
    { 22050, 24000, 16000, }, /* MPEG 2   */
    { 44100, 48000, 32000 }   /* MPEG 1   */
};

static const uint32_t bitrate_table[2][3][15]
{
    {    /* MPEG 1 */
        { 0,32,64,96,128,160,192,224,256,288,320,352,384,416,448, },  /* Layer1 */
        { 0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384, },  /* Layer2 */
        { 0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320, }   /* Layer3 */
    },                                                                
    {    /* MPEG 2, 2.5 */                                              
        { 0,32,48,56,64,80,96,112,128,144,160,176,192,224,256, },    /* Layer1 */
        { 0,8,16,24,32,40,48,56,64,80,96,112,128,144,160, },         /* Layer2 */
        { 0,8,16,24,32,40,48,56,64,80,96,112,128,144,160, }          /* Layer3 */
    }
};

class mpa_header_parser
{
public:
    void parse(const media_sample& sample)
    {
        auto data = sample.data();

        if ((data[0] == 0xFF) && ((data[1] & 0xE0) == 0xE0) && ((data[2] & 0xF0) != 0xF0))
        {
            /* get mpeg version (bits 11 and 12) */
            auto version = static_cast<mpa_version>((data[1] >> 3) & 0x03);    /* mask only the right 2 bits */

            bool low_sampling_frequencies = false;

            if (version == mpa_version::MPEG1)
            {
                low_sampling_frequencies = false;
            }
            else
            {
                low_sampling_frequencies = true;
            }

            /* get the layer (bit 13 and 14) */
            auto layer = static_cast<mpa_layer>(3 - ((data[1] >> 1) & 0x03));

            /* inverted protection bit 15 */
            bool has_crc = !((data[1]) & 0x01);

            uint8_t bitrate_index = static_cast<uint8_t>((data[2] >> 4) & 0x0F);

            if (bitrate_index == 0x0F)
            {
                return;
            }

            _bitrate = bitrate_table[low_sampling_frequencies][static_cast<size_t>(layer)][bitrate_index];

            _bitrate *= 1000; /* kbps */

            /* fuck if I know */
            if (_bitrate == 0)    
            {
                return;
            }

            /* sampling rate (bit 20 & 21) */
            uint8_t sampling_rate_index = static_cast<uint8_t>((data[2] >> 2) & 0x03);

            if (sampling_rate_index == 0x03)        // all bits set is reserved
            {
                return;
            }
            
            _samples_per_second = sampling_rates[static_cast<size_t>(version)][sampling_rate_index];

             auto channel = static_cast<channel_mode>((data[3] >> 6) & 0x03);

             switch(channel)
             {
             case channel_mode::Stereo: 
                 _channel_count = 2;
                 break;
             case channel_mode::JointStereo: 
                 _channel_count = 4;
                 break;
             case channel_mode::DualChannel: 
                 _channel_count = 2;
                 break;
             case channel_mode::SingleChannel: 
                 _channel_count = 1;
                 break;
             default: break;
             }
        }
    }

    uint32_t bitrate()
    {
        return _bitrate;
    }

    uint32_t samples_per_second()
    {
        return _samples_per_second;
    }

    uint8_t channel_count()
    {
        return _channel_count;
    }

private:
    uint32_t _bitrate = 0;

    uint32_t _samples_per_second = 0;

    uint8_t _channel_count = 0;

};

static const uint32_t BITS_PER_SAMPLE = 16;

_mpa_audio_framer::_mpa_audio_framer(const media_subsession& subsession) : _media_framer_base(subsession)
{
    auto sample = _media_framer_base::working_sample();

    sample.attribute_set(ATTRIBUTE_AUDIO_BITS_PER_SAMPLE, BITS_PER_SAMPLE);
}

_mpa_audio_framer::~_mpa_audio_framer()
{
}

void _mpa_audio_framer::sample_receieved(bool packet_marker_bit)
{
    mpa_header_parser parser;

    auto sample = working_sample();

    parser.parse(sample);

    sample.attribute_set(ATTRIBUTE_AUDIO_BITS_PER_SAMPLE, BITS_PER_SAMPLE);

    sample.attribute_set(ATTRIBUTE_AUDIO_CHANNEL_COUNT, parser.channel_count());

    sample.attribute_set(ATTRIBUTE_AUDIO_SAMPLES_PER_SECOND, parser.samples_per_second());

    sample.attribute_set(ATTRIBUTE_AUDIO_BITRATE, parser.bitrate());

    _media_framer_base::sample_receieved(packet_marker_bit);
}