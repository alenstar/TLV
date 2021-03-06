
#include <memory>
#include <string>

#include "buffer.h"
#include "stream.h"

namespace Streams {

Stream::Stream( int access_mode /*= AccessMode::READ*/ )
    : m_name( AEON_STREAMS_DEFAULT_STREAM_NAME )
    , m_size( 0 )
    , m_access_mode( access_mode ) {}

Stream::Stream( const std::string &name, int access_mode /*= AccessMode::READ*/ )
    : m_name( name )
    , m_size( 0 )
    , m_access_mode( access_mode ) {}

Stream::~Stream() {
    // 由子类调用
    // close();
}

BufferPtr Stream::get_as_buffer() {
    size_t s = size();

    auto buffer = std::make_shared<Buffer>( s );
    read( buffer->get(), s );

    return buffer;
}

size_t Stream::write( const std::string &str ) { return write( str.data(), str.size() ); }

size_t Stream::write( BufferPtr buffer ) {
    if ( !buffer ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "Stream: Tried writing an empty buffer to a stream." );
#endif

        return 0;
    }

    return write( buffer->get(), buffer->size() );
}

} /* namespace Streams */
