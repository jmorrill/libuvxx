#include "details/_media_attributes_impl.h"
#include "media_attributes.hpp"

using namespace uvxx::io;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

media_attributes::media_attributes(const details::_media_attributes_impl_ptr& attributes)
    : ___media_attributes_impl(attributes)
{
}

media_attributes::media_attributes() :
    ___media_attributes_impl(std::make_shared<details::_media_attributes_impl>())
{
}
void media_attributes::attribute_blob_set(const std::string& attribute_name, const memory_buffer& buffer)
{
    ___media_attributes_impl->attribute_set(attribute_name, buffer);
}

memory_buffer media_attributes::attribute_blob_get(const std::string& attribute_name) const
{
    return ___media_attributes_impl->attribute_get(attribute_name);
}

void media_attributes::copy_attributes_from(const media_attributes& attributes)
{
    ___media_attributes_impl->copy_attributes_from(attributes.___media_attributes_impl);
}

void media_attributes::clear_attributes()
{
    ___media_attributes_impl->clear_attributes();
}
