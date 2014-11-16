#pragma once

#include "arc/core/numeric_types.hpp"
#include "arc/string/String.hpp"
#include "arc/string/StringView.hpp"

namespace arc { namespace io {

    class BinaryReadStream
    {
    public:
        virtual ~BinaryReadStream() {}
    public:
        virtual uint64_t read(void* target, uint64_t n_bytes) = 0;
    public:
        virtual uint64_t seek_current(int64 byte_offset) = 0;
        virtual uint64_t seek_start(int64 byte_offset = 0) = 0;
        virtual uint64_t seek_end(int64 byte_offset = 0) = 0;
    public:
        virtual uint64_t tell() = 0;
    public:
        virtual bool supports_seek() = 0;
        virtual bool supports_tell() = 0;
    public:
        virtual String source_name() = 0;
    };

    class BinaryWriteStream
    {
    public:
        virtual ~BinaryWriteStream() {}
    public:
        virtual uint64_t write(void* source, uint64_t n_bytes) = 0;
    public:
        virtual uint64_t seek_current(int64 byte_offset) = 0;
        virtual uint64_t seek_start(int64 byte_offset = 0) = 0;
        virtual uint64_t seek_end(int64 byte_offset = 0) = 0;
    public:
        virtual uint64_t tell() = 0;
    public:
        virtual bool supports_seek() = 0;
        virtual bool supports_tell() = 0;
    public:
        virtual String destination_name() = 0;
    };

    class FileReadStream final : public BinaryReadStream
    {
    public:
        ~FileReadStream();
    public:
        bool open(StringView path);
        bool close();
        bool is_open();
    public:
        uint64_t read(void* target, uint64_t n_bytes) override;
    public:
        uint64_t seek_current(int64 byte_offset) override;
        uint64_t seek_start(int64 byte_offset = 0) override;
        uint64_t seek_end(int64 byte_offset = 0) override;
    public:
        uint64_t tell() override;
    public:
        bool supports_seek() override;
        bool supports_tell() override;
    public:
        virtual String source_name() override;
    private:
        void*  _impl = nullptr;
        String _source_name;
    };

    class FileWriteStream final : public BinaryWriteStream
    {
    public:
        ~FileWriteStream();
    public:
		bool open(StringView path);
        bool close();
        bool is_open();
    public:
        virtual uint64_t write(void* source, uint64_t n_bytes);
    public:
        virtual uint64_t seek_current(int64 byte_offset);
        virtual uint64_t seek_start(int64 byte_offset = 0);
        virtual uint64_t seek_end(int64 byte_offset = 0);
    public:
        virtual uint64_t tell();
    public:
        virtual bool supports_seek();
        virtual bool supports_tell();
    public:
        virtual String destination_name() override;
    private:
        void*  _impl = nullptr;
        String _destination_name;
    };


}} // namespace arc::io
