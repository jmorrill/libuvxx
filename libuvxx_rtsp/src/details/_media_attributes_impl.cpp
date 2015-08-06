#include "details/_media_attributes_impl.h"

using namespace uvxx::io;
using namespace uvxx::rtsp::details;

_media_attributes_impl::_media_attributes_impl()
{
}

void _media_attributes_impl::attribute_set(const std::string& attribute_name, const memory_buffer& buffer)
{
    _attribute_map[attribute_name] = buffer;
}

memory_buffer _media_attributes_impl::attribute_get(const std::string& attribute_name)
{
    auto iterator = _attribute_map.find(attribute_name);

    if (iterator == _attribute_map.end())
    {
        return nullptr;
    }

    return iterator->second;
}

void _media_attributes_impl::copy_attributes_from(const _media_attributes_impl_ptr& attributes)
{
    clear_attributes();
    _attribute_map = attributes->_attribute_map;
}

void _media_attributes_impl::clear_attributes()
{
    _attribute_map.clear();
}