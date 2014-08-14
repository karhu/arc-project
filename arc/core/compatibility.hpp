#pragma once

// thread local storage ////////////////////////////////////////////////

#ifdef _WIN32 // win32 & win64 should rather be visual c++ compiler check
	#define ARC_THREAD_LOCAL __declspec(thread)

	#include <type_traits>
	#define alignof(T) std::alignment_of<T>::value

	#define ARC_CONSTEXPR
#else // assume full c++11 support
	#define ARC_THREAD_LOCAL thread_local
	#define ARC_CONSTEXPR constexpr
#endif
