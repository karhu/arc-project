#pragma once

#include "arc/core/numeric_types.hpp"

#include <stdio.h>

namespace arc
{
	class String;
	class StringView;

    bool operator==(const String& first, const StringView& second);
    bool operator==(const StringView& first, const String& second);
    bool operator==(const StringView& first, const StringView& second);

#if 0
	void fmt_test()
	{
		//const char* output = format("hello {0}, {1}, {2} and {3}!", "world", 42, 2.5f);
	}

	namespace string
	{
		const int32 R_NOT_FINISHED = 0;
		const int32 R_NULL_REACHED = 1;
		const int32 R_OUT_BUFFER_FILLED = 2;
		const int32 R_ERROR_PARSE = -1;
		const int32 R_ERROR_INTERNAL = -1;

		int32 write(char* output_buffer, uint32& io_length, int32 value)
		{
			uint32 cnt = _snprintf_s(output_buffer, io_length, io_length, "%i", value);
			
			if (cnt < 0)
			{
				return R_ERROR_INTERNAL;
			}
			else if (cnt < io_length)
			{
				io_length = cnt;
				return R_NOT_FINISHED;
			}
			else
			{
				return R_OUT_BUFFER_FILLED;
			}
		}

		template<typename ...Args> inline
		bool format(char* output_buffer, uint32& io_length, const char* format_string, Args&& ...arguments)
		{
			// check
			if (format_string == nullptr) return false;

			// bind args to std::tuple
			auto tmp = std::tie(arguments...);
			auto args = std::tuple_cat()

			const char* in_iter = format_string;
			char* out_iter = output_buffer;
			char* out_iter_end = out_iter + io_length;

			using State = std::function < int32() >;
			State state_normal, state_bracket_opened, state_read_number;

			int32 number = 0;

			state_read_number = [&]() {
				char c;
				do
				{
					c = in_iter[0];
					++in_iter;

					if (c == '}')
					{
						uint32 length = out_iter_end - out_iter;
						int32 ret = R_ERROR_INTERNAL;
						switch (number)
						{
						case 0: ret = write(out_iter, length, std::get<0>(args)); break;
						case 1: ret = write(out_iter, length, std::get<1>(args)); break;
						case 2: ret = write(out_iter, length, std::get<2>(args)); break;
						case 3: ret = write(out_iter, length, std::get<3>(args)); break;
						case 4: ret = write(out_iter, length, std::get<4>(args)); break;
						case 5: ret = write(out_iter, length, std::get<5>(args)); break;
						case 6: ret = write(out_iter, length, std::get<6>(args)); break;
						case 7: ret = write(out_iter, length, std::get<7>(args)); break;
						case 8: ret = write(out_iter, length, std::get<8>(args)); break;
						case 9: ret = write(out_iter, length, std::get<9>(args)); break;
						default:
							return R_ERROR_INTERNAL;
						}

						if (ret != R_ERROR_INTERNAL) out_iter += length;
						return ret;
						
					}
					else if (c <= '9' && c >= '0')
					{
						number = (10 * number) + (c - '0');
					}
					else
					{
						return R_ERROR_PARSE;
					}
				
				} while (c != '\0');
				

				return R_NULL_REACHED;
			};

			state_bracket_opened = [&]() {
				char c = in_iter[0];
		
				switch (c)
				{
				case '\0':
					out_iter[0] = c;
					++out_iter;
					return R_NULL_REACHED;
				case '{':
					out_iter[0] = c;
					++out_iter;
					++in_iter;
					return R_NOT_FINISHED;
				case '0':
					
				case '1': 
				case '2':
				case '3': 
				case '4': 
				case '5': 
				case '6': 
				case '7': 
				case '8':
				case '9':

				default:
					return R_ERROR_PARSE;
				}
			};

			state_normal = [&]() {
				while (out_iter < out_iter_end)
				{
					char c = in_iter[0];
					++in_iter;

					switch (c)
					{
					case '\0':
						out_iter[0] = c;
						++out_iter;
						return R_NULL_REACHED;
					case '{': {
						int32 result = state_bracket_opened();
						if (result != R_NOT_FINISHED) return result;
					}
					default:
						out_iter[0] = c;
						++out_iter;
					}
				}
				return R_OUT_BUFFER_FILLED;
			};

			int32 result = state_normal();

			return true;


			/*
			using State = std::function < bool() > ;

			State state_current, state_normal, state_bracket_open;

			state_normal = [&]() {
				char c = in_iter[0];
				bool ok = true;

				switch (c)
				{
				case '{':
					state_current = state_bracket_open;
					break;
				case '}':
					state_current = state_bracket_close;
					break;
				case '\0':
					ok = false;
					break;
				default:
					out_iter[0] = c;
					++out_iter;
					break;
				}
				++in_iter;
				return ok;
			};
			
			state_bracket_open = [&]() {
				char c = in_iter[0];
				bool ok = true;

				switch (c)
				{
				case '{':
					out_iter[0] = c;
					++out_iter;
					state_current = state_normal;
					break;
				case '\0':
					ok = false;
					break;
				default:
					out_iter[0] = c;
					++out_iter;
					return true;
				}
			}

			state_current = state_normal;

			while (out_iter < out_iter_end && cont)
			{
				cont = state_current();
			}
			*/
		}


	}
#endif

} // namespace arc
