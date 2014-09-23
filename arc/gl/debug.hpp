#pragma once

#include "functions.hpp"

#include "arc/string/ConsoleWriter.hpp"

namespace arc { namespace gl {


	class CallObserver
	{
	public:
		virtual void map_buffer_range(void* result, BufferType target, ptrdiff_t offset, ptrdiff_t length, BufferAccess access) {}
	};

	class WriteObserver : public CallObserver
	{
	public:
		virtual void map_buffer_range(void* result, BufferType target, ptrdiff_t offset, ptrdiff_t length, BufferAccess access)
		{
			m_writer.write(result, " = map_buffer_range(", (uint32)target, ", ", offset, ", ", length, ", ", (uint32)access, ")\n");
		}

	private:
		ConsoleWriter<1024> m_writer;
	};


	static WriteObserver _g_gl_write_observer = WriteObserver();
	static CallObserver* gl_observer = &_g_gl_write_observer;

}}