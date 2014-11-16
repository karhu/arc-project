#include "FileStream.hpp"

#include "arc/core.hpp"

#include <SDL2/SDL_rwops.h>
#include <cstring>

namespace arc { namespace io {

uint64_t FileReadStream::read(void *target, uint64_t n_bytes)
{
    ARC_ASSERT(is_open(), "FILE NOT OPEN");
    auto sdl = (SDL_RWops*)_impl;
    auto ret = SDL_RWread(sdl, target, 1, n_bytes);
    return ret;
}

uint64_t FileReadStream::seek_current(int64 byte_offset)
{
    ARC_ASSERT(is_open(), "FILE NOT OPEN");
    auto sdl = (SDL_RWops*)_impl;
    auto ret =  SDL_RWseek(sdl, byte_offset, RW_SEEK_CUR);
    ARC_ASSERT(ret != -1, "SDL_FILE_SEEK ERROR");
    return ret;
}

uint64_t FileReadStream::seek_start(int64 byte_offset)
{
    ARC_ASSERT(is_open(), "FILE NOT OPEN");
    auto sdl = (SDL_RWops*)_impl;
    auto ret =  SDL_RWseek(sdl, byte_offset, RW_SEEK_SET);
    ARC_ASSERT(ret != -1, "SDL_FILE_SEEK ERROR");
    return ret;
}

uint64_t FileReadStream::seek_end(int64 byte_offset)
{
    ARC_ASSERT(is_open(), "FILE NOT OPEN");
    auto sdl = (SDL_RWops*)_impl;
    auto ret =  SDL_RWseek(sdl, byte_offset, RW_SEEK_END);
    ARC_ASSERT(ret != -1, "SDL_FILE_SEEK ERROR");
    return ret;
}

uint64_t FileReadStream::tell()
{
    ARC_ASSERT(is_open(), "FILE NOT OPEN");
    auto sdl = (SDL_RWops*)_impl;
    auto ret = SDL_RWtell(sdl);
    ARC_ASSERT(ret != -1, "SDL_FILE_TELL ERROR");
    return ret;
}

bool FileReadStream::supports_seek()
{
    return true;
}

bool FileReadStream::supports_tell()
{
    return true;
}

String FileReadStream::source_name()
{
    return _source_name;
}

FileReadStream::~FileReadStream()
{
    close();
}

bool FileReadStream::open(StringView path)
{
    if (_impl != nullptr) close();

    _source_name = path;

	_impl = SDL_RWFromFile(_source_name.c_str(), "r");
    return _impl != nullptr;
}

bool FileReadStream::close()
{
    if (_impl != nullptr)
    {
        auto sdl = (SDL_RWops*)_impl;
        auto ret = SDL_RWclose(sdl);
        _impl = nullptr;
        return ret == 0;
    }
    return true;
}

bool FileReadStream::is_open()
{
    return _impl != nullptr;
}

FileWriteStream::~FileWriteStream()
{
    close();
}

bool FileWriteStream::open(StringView path)
{
    if (_impl != nullptr) close();

	_destination_name = path;

	_impl = SDL_RWFromFile(_destination_name.c_str(), "w");
    return _impl != nullptr;
}

bool FileWriteStream::close()
{
    if (_impl != nullptr)
    {
        auto sdl = (SDL_RWops*)_impl;
        auto ret = SDL_RWclose(sdl);
        _impl = nullptr;
        return ret == 0;
    }
    return true;
}

bool FileWriteStream::is_open()
{
    return _impl != nullptr;
}

uint64_t FileWriteStream::write(void *source, uint64_t n_bytes)
{
    ARC_ASSERT(is_open(), "FILE NOT OPEN");
    auto sdl = (SDL_RWops*)_impl;
    auto ret = SDL_RWwrite(sdl, source, 1, n_bytes);
    return ret;
}

uint64_t FileWriteStream::seek_current(int64 byte_offset)
{
    ARC_ASSERT(is_open(), "FILE NOT OPEN");
    auto sdl = (SDL_RWops*)_impl;
    auto ret =  SDL_RWseek(sdl, byte_offset, RW_SEEK_CUR);
    ARC_ASSERT(ret != -1, "SDL_FILE_SEEK ERROR");
    return ret;
}

uint64_t FileWriteStream::seek_start(int64 byte_offset)
{
    ARC_ASSERT(is_open(), "FILE NOT OPEN");
    auto sdl = (SDL_RWops*)_impl;
    auto ret =  SDL_RWseek(sdl, byte_offset, RW_SEEK_SET);
    ARC_ASSERT(ret != -1, "SDL_FILE_SEEK ERROR");
    return ret;
}

uint64_t FileWriteStream::seek_end(int64 byte_offset)
{
    ARC_ASSERT(is_open(), "FILE NOT OPEN");
    auto sdl = (SDL_RWops*)_impl;
    auto ret =  SDL_RWseek(sdl, byte_offset, RW_SEEK_END);
    ARC_ASSERT(ret != -1, "SDL_FILE_SEEK ERROR");
    return ret;
}

uint64_t FileWriteStream::tell()
{
    ARC_ASSERT(is_open(), "FILE NOT OPEN");
    auto sdl = (SDL_RWops*)_impl;
    auto ret = SDL_RWtell(sdl);
    ARC_ASSERT(ret != -1, "SDL_FILE_TELL ERROR");
    return ret;
}

bool FileWriteStream::supports_seek()
{
    return true;
}

bool FileWriteStream::supports_tell()
{
    return true;
}

String FileWriteStream::destination_name()
{
    return _destination_name;
}


}} // namespace arc::io
