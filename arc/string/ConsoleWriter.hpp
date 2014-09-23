#pragma once

#include "arc/collections/Slice.hpp"
#include <iostream>
#include "arc/string/write_string.hpp"

namespace arc
{
	template <uint32 BUFFER_SIZE>
	struct ConsoleWriter
	{
		template< typename ...Args>
		void write(Args&&... args)
		{
			auto buffer = make_slice<char>(m_buffer, BUFFER_SIZE);
			unroll_write(buffer, std::forward<Args>(args)...);
		}
	private:
		template<typename T, typename ...Tail>
		void unroll_write(Slice<char> buffer, const T& head, Tail&&... tail)
		{
			bool ok;
			ok = write_string(buffer, head);
			if (!ok)
			{
				// flush
				std::cout.write(m_buffer, BUFFER_SIZE - buffer.size());
				// reset buffer
				buffer = make_slice<char>(m_buffer, (uint64)BUFFER_SIZE);
				// try again
				ok = write_string(buffer, head);
				if (!ok)
				{
					StringView error = " <ERROR> ";
					std::cout.write(error.c_str(), error.length());
				}
			}
			unroll_write(buffer, std::forward<Tail>(tail)...);
		}

		void unroll_write(Slice<char> buffer)
		{
			// flush
			std::cout.write(m_buffer, BUFFER_SIZE - buffer.size());
			std::cout.flush();
		}
		char m_buffer[BUFFER_SIZE];
	};

}