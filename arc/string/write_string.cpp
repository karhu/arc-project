#include "write_string.hpp"

#include "arc/collections/Slice.hpp"

#include <cstring>
#include <cstdio>

#ifdef _WIN32
#define snprintf(a,b,c,d) _snprintf_s(a,b,_TRUNCATE,c,d);
template<typename T>
int32_t fmt_write(char* buffer, size_t buffer_size, const char* fmt, T value)
{
	auto ret = _snprintf_s(buffer, buffer_size, _TRUNCATE, fmt, value);
	return ret;
}
#else
template<typename T>
int32_t fmt_write(char* buffer, int64_t buffer_size, const char* fmt, T value)
{
	auto ret = snprintf(buffer, buffer_size, fmt, value);
	if (ret < 0 || ret > buffer_size) return -1;
	return ret;
}
#endif

namespace arc
{
	#define RESOLVE()												\
		if (written == -1) return false;							\
		else { buffer.trim_front(written); return true; }			\

	bool write_string(Slice<char>& buffer, uint32 v)
	{
		auto written = fmt_write(buffer.ptr(), buffer.size(), "%u", v);
		RESOLVE();
	}

	bool write_string(Slice<char>& buffer, int32 v)
	{
		auto written = fmt_write(buffer.ptr(), buffer.size(), "%d", v);
		RESOLVE();
	}

	bool write_string(Slice<char>& buffer, void* v)
	{
		auto written = fmt_write(buffer.ptr(), buffer.size(), "%p", v);
		RESOLVE();
	}
}